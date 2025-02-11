#pragma once

#include <optional>
#include <string>
#include <vector>

#include "pybind11/embed.h"

namespace py = pybind11;

class PythonCaller;

using BytesIn = const std::string &;
using BytesOut = std::optional<std::string>;

class __attribute__((visibility("default"))) PythonCaller {
    public:
        explicit PythonCaller(
            const std::string module_name,
            const std::vector<std::string> &sys_paths = std::vector<std::string>()
        );

        PythonCaller() = delete;
        PythonCaller(const PythonCaller &) = delete;

        ~PythonCaller();

        BytesOut Run(const std::string fname, BytesIn &ib);

        template<class PBIN, class PBOUT>
        std::optional<PBOUT> Run(const std::string &fname, const PBIN &ipb) {
            std::string is;
            ipb.SerializeToString(&is);
            const std::optional<std::string> os = Run(fname, is);
            if (os.has_value()) {
                PBOUT opb;
                opb.ParseFromString(os.value());
                return opb;
            }
            return {};
        }

    private:
        bool inited_;
        PyThreadState *tstate_;
        py::module_ imported_;
};

