#! /bin/bash

if [ -e time.txt ]; then
    rm time.txt
fi

for i in 128 256 512 1024
do
    echo "server launched with file size $i"
    ./server-optim -j 1 -s $i -p 2243 & \
    sleep 1
    ./client -k 128 -r 4 -t 5 127.0.0.1:2243 >> time.txt
    pid=$(ps -A | grep server-optim)
    kill ${pid:2:6}
    sleep 1
done

