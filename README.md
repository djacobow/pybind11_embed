# pybind11_embed

Quick how-to on how to embed python.

## Why?

Most people use pybind11 to call c++ from their Python, but it can be
cool to call Python from their c++.

## What

A simple wrapper around pybind11 embed that shows how it is done.

### Threads

Threads are a complication, because of the need to lock and unlock the
gil, and as far as I can tell, the "scoped gil" code to do this does
not work quite right in pybind11, so this wrapper also creates a queue
where calls into python are (unfortunately) serialized and processed
one-at-a-time by a special python-running thread. In each case the caller
that pushed into the queue blocks waiting to be notified of its result.

#### UPDATE !!!

With the help of people from the pybind discussion, I was able to make
the gil locking work without the use of a work queue. I have added
a minimal example here called `no_queue.cc`. You can use this entirely
instead of `pycall.hpp`

### Protobufs

A common way of sharing data between python and c++ is via protobufs,
which both support. Sadly, their underlying memory image of a protobuf is
pretty different, so code that moves protobufs across this boundary does
so by using the protobufs marshal/unmarshal capabilities to stringify
when crossing the bounday. This is convenient and simple, but not super
efficient.  It's probably better to use pybind11's wrapping abilities
to do what you want.

## Building

Rather than provide a complex CMake or Bazel file, this just includes
a script that shows how to get the necessary linker and cflags to build
an app with pybind11 and protobufs.

