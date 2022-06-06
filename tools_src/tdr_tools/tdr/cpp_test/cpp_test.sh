#!/bin/sh

scons -u && \
cd ../tools_src/tdr_tools/tdr/cpp_test/ &&\
make &&\
./tdr_cpp_test && \
echo "+-----------------------------+" &&\
echo "+      ..ooOOOOOOOOoo..       +" &&\
echo "+-----------------------------+"
