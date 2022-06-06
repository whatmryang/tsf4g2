# See README.txt.

.PHONY: all clean

all: bp_performance

clean:
	rm -f *.pb.*
	rm -f bp_performance

protoc_middleman: mixture.proto
	protoc --cpp_out . mixture.proto

bp_performance: bp_performance.cc protoc_middleman
	pkg-config --cflags protobuf  # fails if protobuf is not installed
	g++ bp_performance.cc -g -O2 *.pb.cc -o bp_performance -lprotobuf -lpthread
