#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <numbers>
#include <random>
#include <thread>
#include <utility>

#include "pycall.hpp"
#include "proto_cc/test_examp.pb.h"

static uint32_t rand_test(std::shared_ptr<PythonCaller> pc, uint32_t count, uint16_t max_size) {
    uint32_t errors = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> genLengths(1, max_size);
    std::uniform_int_distribution<uint8_t> genBytes(0, 255);

    for (auto iters=0; iters<count; iters++) {
        auto test_str = std::string(genLengths(gen),0);
        for (char &v: test_str) {
            v = static_cast<char>(genBytes(gen));
        }
        auto rv = pc->Run("reverse_bytes", test_str);
        if (!rv.has_value()) {
            std::cerr << "no return value\n";
            errors++;
        }
        std::ranges::reverse(test_str);
        if (rv.has_value()) { // for clang-tidy
            if (test_str != rv.value()) {
                std::cerr << "reverse error\n";
                errors++;
            }
        }
    }
    return errors;
}

uint32_t trivial(std::shared_ptr<PythonCaller> &pc) {
    return rand_test(pc, 10, 65535);
};

uint32_t subthread(std::shared_ptr<PythonCaller> &pc) {
    uint32_t ecount = 0;
    std::thread t1( [pc, &ecount]() {
        ecount += rand_test(pc, 5, 65535);
    });
    std::thread t2( [pc, &ecount]() {
        ecount += rand_test(pc, 5, 65535);
    });
    t1.join();
    t2.join();
    return ecount;
};

uint32_t pbpb(std::shared_ptr<PythonCaller> &pc) {
    uint32_t ecount = 0;
    In_pb ipb;
    ipb.set_a_string("This is the input pb argument string");        
    ipb.set_a_double(std::numbers::pi);
    ipb.set_a_uint64(0xdeadbeef);
   
    auto opb = PyFunctionPbPb<In_pb, Out_pb>(pc, "pb_in_pb_out", ipb);
    assert(opb.has_value());
    if (opb.has_value()) {
        assert(opb.value().a_string() == std::string("this is the result"));
        assert(fabs(opb.value().a_float() - 2.71828) <= 1e-7);
        assert(opb.value().a_int32() == 0x5555aaaa);
    } 
    return ecount;
}

int main(int argc, char *argv[]) {
    std::shared_ptr<PythonCaller> pc = nullptr;
    pc = std::make_shared<PythonCaller>(
        "test_examp"
    );
    uint32_t errors = 0;
    errors += trivial(pc);
    errors += subthread(pc);
    errors += pbpb(pc);

    if (errors) {
        std::cerr << "Whoops, there were errors\n";
    } else {
        std::cout << "Yay! No errors!\n";
    }
    return errors;
};

