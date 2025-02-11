#pragma once

#include <optional>
#include <string>
#include <vector>

#include "tsafeq.hpp"
#include "pybind11/embed.h"

namespace py = pybind11;

class PythonCaller;

using BytesIn = const std::string &;
using BytesOut = std::optional<std::string>;

// This starts the interpreter the first time it is needed,
// and since the object is file-level, it will keep the
// interpreter running until destruction at program end
class __attribute__((visibility("default"))) PythonCaller {
    class QSubmission;

    public:
        explicit PythonCaller(
            const std::string module_name,
            bool threadify = true,
            const std::vector<std::string> &sys_paths = std::vector<std::string>()
        );

        PythonCaller() = delete;
        PythonCaller(const PythonCaller &) = delete;

        void Stop();
        ~PythonCaller();

        BytesOut Run(const std::string fname, BytesIn &arg);

    private:
        // NOLINTBEGIN
        void _Init(); 
        void _Start(bool use_thread); 
        BytesOut _CallFunction(const std::string &fname, BytesIn &ib);
        // NOLINTEND

        bool inited_;
        bool run_;
        const std::string module_name_;
        const std::vector<std::string> sys_paths_;
        TSafeQueue<std::shared_ptr<QSubmission>> tsq_;
        py::module_ imported_;
        std::thread tq_;


        class QSubmission {
            friend class PythonCaller;
        
            public:
                QSubmission(const std::string fname, BytesIn &arg) :
                    done_(false),
                    fname_(fname), 
                    arg_(arg) { }
        
                QSubmission(const QSubmission &) = delete;
        
                void SignalComplete(const BytesOut &result);
        
                BytesOut Wait();
    
            private:
                std::mutex m_;
                std::condition_variable c_;
                std::optional<std::string> result_;
                bool done_;
                const std::string fname_;
                BytesIn arg_;
    
        };
};


template<class PBIN, class PBOUT>
std::optional<PBOUT> PyFunctionPbPb(std::shared_ptr<PythonCaller> pc, const std::string &fname, const PBIN &ipb) {
    std::string is;
    ipb.SerializeToString(&is);
    const std::optional<std::string> os = pc->Run(fname, is);
    if (os.has_value()) {
        PBOUT opb;
        opb.ParseFromString(os.value());
        return opb;
    }
    return {};
}

