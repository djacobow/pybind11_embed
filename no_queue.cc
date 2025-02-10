#include <format>
#include <thread>
#include <vector>

#include "pybind11/embed.h"

namespace py = pybind11;

void acquire_and_release() {
    py::gil_scoped_acquire g{};
};

void call_in_threads(uint32_t count) {
    std::vector<std::unique_ptr<std::thread>> tv;
    for (uint32_t i=0; i<count; i++) {
        tv.push_back(std::make_unique<std::thread>(
            [i]() {
                py::gil_scoped_acquire g{};
                py::exec(std::format("print('Hello from thread:{}')", i));
            }
        ));
    }
    for (auto &t : tv) {
        t->join();
    }

};

int main(int ac, char *av[]) {
    py::initialize_interpreter();
    PyThreadState *tstate = PyEval_SaveThread();

    acquire_and_release();
    call_in_threads(50);

    PyEval_RestoreThread(tstate);
    py::finalize_interpreter();
    return 0;
};
