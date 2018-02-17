#!/bin/bash
#Copy over the source code
cp -r desktop_src/. ../../openpose/examples/user_code/

#Build openpose
cd ../../openpose/build/
make -j`nproc`
