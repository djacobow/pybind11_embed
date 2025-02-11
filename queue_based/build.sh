#!/bin/bash -x

#sudo apt install pybind11 pybind11-dev protobuf google-protobuf

PACKAGES="python3-embed pybind11 protobuf"
CFLAGS="-std=c++20 -g -O0 -I. `pkg-config --cflags ${PACKAGES}`"
LDFLAGS=`pkg-config --libs ${PACKAGES}`

rm -rf *.o ../common/proto_cc ../common/proto_py test
mkdir ../common/proto_cc
mkdir ../commonproto_py
protoc -I../common --cpp_out=../common/proto_cc    ../common/test_examp.proto
protoc -I../common --python_out=../common/proto_py ../common/test_examp.proto

g++ ${CFLAGS} -c proto_cc/test_examp.pb.cc
g++ ${CFLAGS} -c main.cc
g++ ${CFLAGS} -c pycall.cc
g++ ${CFLAGS} -c ../common/tests.cc
g++ ${CFLAGS} main.o pycall.o tests.o test_examp.pb.o ${LDFLAGS} -o test
./test
