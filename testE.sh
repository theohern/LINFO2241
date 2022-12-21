#! /bin/bash

if [ -e E.txt ]; then
    rm E.txt
fi

r=3

./server-float-avx -j 1 -s 256 -p 2241 & \

sleep 1

./client-queue -k 128 -r $r -t 30 127.0.0.1:2241 >> E.txt

pkill -f "./server-float-avx -j 1 -s 256 -p 2241"
