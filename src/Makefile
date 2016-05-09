CFLAGS = -g -W -Wall -Werror

client_target = client
server_target = server

common_obj = comm.o socket.o
client_obj = client.o ${common_obj} 
server_obj = server.o ${common_obj}

all : ${client_target} ${server_target}
.PHONY : all

${client_target} : ${client_obj} 
	cc -o ${client_target} ${client_obj}  

${server_target} : ${server_obj}
	cc -o ${server_target} ${server_obj}

comm.o : comm.c comm.h
	cc -c ${CFLAGS} comm.c

server.o : server.c comm.h
	cc -c ${CFLAGS} server.c

client.o : client.c comm.h
	cc -c ${CFLAGS} client.c
	
socket.o : socket.c socket.h
	cc -c ${CFLAGS} socket.c

clean : 
	-rm -f ${client_target} ${server_target} *.o
