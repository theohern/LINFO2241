CC=gcc

client:
	$(CC) client.c -o client 
	./client -k 16 -r 1000 -t 10 127.0.0.1:2000 
	@rm -f client


server:
	$(CC) server.c -o server 
	./server -j 4 -s 32 -p 2000
	@rm -f server
	

time :
	$(CC) client.c -o client
	/usr/bin/time -f %e ./client -k 16 -r 1000 -t 10 127.0.0.1:2000 
	@rm -f client

clean:
	@rm -f client
	@rm -f a.out
	@rm -f server

