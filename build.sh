#!/bin/bash -x

#sudo apt install pybind11 pybind11-dev protobuf google-protobuf

PACKAGES="python3-embed pybind11 protobuf"
CFLAGS="-std=c++20 -g -O0 `pkg-config --cflags ${PACKAGES}`"
LDFLAGS=`pkg-config --libs ${PACKAGES}`

rm -rf *.o proto_cc proto_py test no_queue
mkdir proto_cc
mkdir proto_py
protoc --cpp_out=proto_cc    test_examp.proto
protoc --python_out=proto_py test_examp.proto

g++ ${CFLAGS} -c proto_cc/test_examp.pb.cc
g++ ${CFLAGS} -c main.cc
g++ ${CFLAGS} -c pycall.cc
g++ ${CFLAGS} -c no_queue.cc
g++ ${CFLAGS} main.o pycall.o test_examp.pb.o ${LDFLAGS} -o test
g++ ${CFLAGS} no_queue.o ${LDFLAGS} -o no_queue 
./test
./no_queue

