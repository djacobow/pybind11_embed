#include <cassert>
#include <numbers>
#include <random>
#include <thread>

#include "tests.h"
#include "proto_cc/test_examp.pb.h"

uint32_t rand_test(std::shared_ptr<PythonCaller> pc, uint32_t count, uint16_t max_size) {
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

uint32_t many_subthread(std::shared_ptr<PythonCaller> &pc, uint32_t thrcount) {
    std::vector<std::unique_ptr<std::thread>> tv;
    uint32_t ecount = 0;
    for (uint32_t i=0; i<thrcount; i++) {
        tv.push_back(std::make_unique<std::thread>(
            [pc, &ecount]() {
                ecount += rand_test(pc, 5, 65535);
            }
        ));
    }
    for (auto &t : tv) {
        t->join();
    }
    return ecount;
}

