#include "pycall.hpp"
#include <iostream>

void PythonCaller::Stop() {
    run_ = false;
    if (tq_.joinable()) {
        tq_.join();
    };
}

BytesOut PythonCaller::Run(const std::string fname, BytesIn &arg) {
    auto s = std::make_shared<PythonCaller::QSubmission>(fname, arg);
    tsq_.Push(s);
    return s->Wait();
}

void PythonCaller::_Init() {
    if (!inited_) {
        py::exec("import sys");
        for (const auto &p : sys_paths_) {
            py::exec("sys.path.insert(0,\"" + p + "\")");
        }
        char *runfiles_dir_str = std::getenv("RUNFILES_DIR");
        if (runfiles_dir_str) {
            std::cout << __func__ << " RUNFILES_DIR=" << std::getenv("RUNFILES_DIR") << "\n";
            py::exec("sys.path.insert(0,os.environ.get('RUNFILES_DIR'))");
        }
        if (false) {
            py::exec("print('sys.paths:\\n' + '\\n '.join([ p if p else \"None\" for p in sys.path]))");
        };
        imported_ = py::module_::import(module_name_.c_str());
        inited_ = true;
    }
}

void PythonCaller::_Start(bool use_thread) {
    if (!use_thread) {
        _Init();
        return;
    }
    tq_ = std::thread([this]() {
        py::scoped_interpreter i_ {};  // NOLINT
        _Init();
        run_ = true;
        while (this->run_) {
            std::shared_ptr<QSubmission> sub;
            if (tsq_.PopNoBlock(sub)) {
                try {
                    sub->SignalComplete(this->_CallFunction(sub->fname_, sub->arg_));
                } catch (...) {
                    std::cout  << "tq_ caught something\n";
                }
            } else {
                usleep(10000);
            }
        }
        imported_ = {};
    });
}

PythonCaller::~PythonCaller() {
    if (run_) {
        Stop();
    }
}

PythonCaller::PythonCaller(
    const std::string module_name,
    bool threadify,
    const std::vector<std::string> &sys_paths
) :

    inited_(false),
    run_(false),
    module_name_(module_name),
    sys_paths_(sys_paths) {
    _Start(threadify);
};

BytesOut PythonCaller::_CallFunction(const std::string &fname, BytesIn &ib) {
    const py::object result = imported_.attr(fname.c_str())(py::bytes(ib));
    if (result.is_none()) {
        return {};
    }
    const auto res_type = py::type::of(result);
    const auto type_str = res_type.attr("__name__").cast<std::string>();
    if ((type_str == "bytes") || (type_str == "bytearray")) {
        return result.cast<std::string>();
    }
    return {};
}

void PythonCaller::QSubmission::SignalComplete(const BytesOut &result) {
    std::unique_lock lock(m_);
    result_ = result;
    done_ = true;
    lock.unlock();
    c_.notify_one();
}

BytesOut PythonCaller::QSubmission::Wait() {
    std::unique_lock lk(m_);
    c_.wait(lk, [this] { return done_; });
    return result_;
}
