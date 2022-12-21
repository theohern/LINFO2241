#! /bin/bash

if [ -e time.txt ]; then
    rm time.txt
fi

for i in 16 32 64 128 256 512 1024
do
    echo "server launched with file size $i"
    ./server-float -j 1 -s $i -p 2241 & \
    sleep 1
    ./client -k 8 -r 20 -t 5 127.0.0.1:2241 >> time.txt
    pkill -f "./server-float -j 1 -s $i -p 2241"
    sleep 1
done


for i in 16 32 64 128 256 512 1024
do
    echo "server launched with file size $i"
    ./server-float-avx -j 1 -s $i -p 2241 & \
    sleep 1
    ./client -k 8 -r 20 -t 5 127.0.0.1:2241 >> time.txt
    pkill -f "./server-float-avx -j 1 -s $i -p 2241"
    sleep 1
done


python3 plot.py

