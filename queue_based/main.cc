#include <cassert>

#include "pycall.hpp"
#include "../common/tests.h"
#include "proto_cc/test_examp.pb.h"

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
        "test_examp", true, std::vector<std::string>{"../common"}
    );

    uint32_t errors = 0;
    errors += trivial(pc);
    errors += subthread(pc);
    errors += many_subthread(pc, 50);
    errors += pbpb(pc);

    if (errors) {
        std::cerr << "Whoops, there were errors\n";
    } else {
        std::cout << "Yay! No errors!\n";
    }
    return errors;
};

