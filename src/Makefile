all : client server


client : client.o comm.o
	cc -o client client.o comm.o

server : server.o comm.o
	cc -o server server.o comm.o

comm.o : comm.c comm.h
	cc -c comm.c

server.o : server.c comm.h
	cc -c server.c

client.o : client.c comm.h
	cc -c client.c

clean : 
	-rm -f client server *.o
