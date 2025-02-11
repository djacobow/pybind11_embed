#pragma once

#include "pycall.hpp"

uint32_t rand_test(std::shared_ptr<PythonCaller> pc, uint32_t count, uint16_t max_size);
uint32_t trivial(std::shared_ptr<PythonCaller> &pc);
uint32_t subthread(std::shared_ptr<PythonCaller> &pc);
uint32_t many_subthread(std::shared_ptr<PythonCaller> &pc, uint32_t thrcount);
