CC=gcc
LP=-lpthread
FLAG=-mno-sse2 -mno-avx -mno-avx2 -mno-avx512f -fno-unroll-loops -fno-tree-vectorize -O2
FILE_SIZE=1024
PORT=2241
THREAD=1
KEY_SIZE=128
RATE=20
TIME=5
IP=127.0.0.1

all : client server server-float server-float-avx

client:
	$(CC) client.c -o client -lpthread $(FLAG)


server:
	$(CC) $(LP) -o server server.c $(FLAG)

server-float:
	$(CC) $(LP) -o server-float server-float.c $(FLAG)

server-float-avx:
	$(CC) $(LP) -o server-float-avx server-float-avx.c $(FLAG) -mavx -g

run-client:
	./client -k $(KEY_SIZE) -r $(RATE) -t $(TIME) 127.0.0.1:2241

run-time-client:
	/usr/bin/time -f %e ./client -k $(KEY_SIZE) -r $(RATE) -t $(TIME) $(IP):$(PORT) >> times.txt

run-server:
	./server -j $(THREAD) -s $(FILE_SIZE) -p $(PORT)

run-server-float:
	./server-float -j $(THREAD) -s $(FILE_SIZE) -p $(PORT)


run-server-float-avx:
	./server-float-avx -j $(THREAD) -s $(FILE_SIZE) -p $(PORT)

clean:
	rm server
	rm server-float
	rm client
	rm server-float-avx

graph : all 
	./script.sh

tar:
	tar -zcvf Part3.tar.gz client.c server.c server-float.c server-float-avx.c Makefile plot.py script.sh

