# pybind11_embed

Quick how-to on how to embed python.

## Why?

Most people use pybind11 to call c++ from their Python, but it can be
cool to call Python from their c++.

## What

A simple wrapper around pybind11 embed that shows how it is done.

### Threads

Threads are a complication, because of the need to lock and unlock the
gil. I struggled  with this so much that I implemented a version of
this that pushes all python work through a serializing queue. You
can see that in the `queue_based/` directory.

However, someone showed me how it is done, and we can now call from
different threads by grabbing the gil lock, and not requiring a single
python worker thread. See `gil_only/`.

For compute-bound workloads these are not much different, but if the
python code blocks on io or other things, then using the gil as the
only serializing method is an advantage.


### Protobufs

A common way of sharing data between python and c++ is via protobufs,
which both support. Sadly, their underlying memory image of a protobuf is
pretty different, so code that moves protobufs across this boundary does
so by using the protobufs marshal/unmarshal capabilities to stringify
when crossing the bounday. This is convenient and simple, but not super
efficient.  It may be better to use pybind11's wrapping abilities
to do what you want.

## Building

Rather than provide a complex CMake or Bazel file, this just includes a
script `build.sh` that shows how to get the necessary linker and cflags
to build an app with pybind11 and protobufs.

