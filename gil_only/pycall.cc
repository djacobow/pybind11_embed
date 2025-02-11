#include "pycall.hpp"

#include <iostream>

PythonCaller::PythonCaller(
    const std::string module_name,
    const std::vector<std::string> &sys_paths) : 
    
    inited_(false), tstate_(nullptr) {

    py::initialize_interpreter();
    tstate_ = PyEval_SaveThread();
    py::gil_scoped_acquire g{};
    if (!inited_) {
        py::exec("import sys");
        for (const auto &p : sys_paths) {
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
        imported_ = py::module_::import(module_name.c_str());
        inited_ = true;
    }
}

PythonCaller::~PythonCaller() {
    PyEval_RestoreThread(tstate_);
    py::gil_scoped_acquire g{};
}

BytesOut PythonCaller::Run(const std::string fname, BytesIn &ib) {
    py::gil_scoped_acquire g{};
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

