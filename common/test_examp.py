import math
import sys

import proto_py.test_examp_pb2 as test_examp_pb2

# a decorator to help with converting bytes to/from pb when
# using pybind11 ffi.
def Unwrap_PB(in_type, out_type):
    def decorator(function):
        def wrapper(ins):
            ipb = in_type()
            ipb.ParseFromString(ins)
            opb = function(ipb)
            if opb is None:
                return None
            return opb.SerializeToString()
        return wrapper
    return decorator


def dprint(*args, **kwargs):
    print(args, kwargs)
    sys.stdout.flush()


def reverse_bytes(b):
    # dprint("python b: ", b)
    return bytes(reversed(b))


@Unwrap_PB(test_examp_pb2.In_pb, test_examp_pb2.Out_pb)
def pb_in_pb_out(ipb):
    dprint("python ipb: ", ipb)

    in_ok = all(
        [
            ipb.a_string == "This is the input pb argument string",
            ipb.a_double == math.pi,
            ipb.a_uint64 == 0xDEADBEEF,
        ]
    )
    if not in_ok:
        return None

    opb = test_examp_pb2.Out_pb()
    opb.a_string = "this is the result"
    opb.a_float = 2.71828
    opb.a_int32 = 0x5555AAAA
    return opb
