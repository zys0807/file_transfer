#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "comm.h"
#include "socket.h"

#define SOCKET_PORT (6162)
#define CLINT_CONNECT_MAX_NUM (10)

static int set_nonblocking(int sock)
{
	assert(sock >= 0);

	if ( fcntl(sock, F_SETFL, fcntl(sock, F_GETFD, 0) | O_NONBLOCK) < 0 )
	{
		lerror("set socket O_NONBLOCK failed, sock: %d, err: %s", sock, strerror(errno));
		return -1;
	}
	return 0;
}


int create_client_socket()
{
	int sock = -1;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		lerror("create socket failed, err: %s", strerror(errno));
		goto ERR;

	}
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SOCKET_PORT);
    addr.sin_addr.s_addr = inet_addr("192.168.116.128");

	if (connect(sock, (struct sockaddr *)(&addr), sizeof(struct sockaddr)) < 0)
	{
		lerror("connect server failed, err: %s", strerror(errno));
		goto ERR;
	}

	return sock;
ERR:
	if (sock >= 0)
	{
		close(sock);
	}
	return -1;
}

int create_server_socket()
{
	int sock = -1;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		lerror("create socket failed, err: %s", strerror(errno));
		goto ERR;
	}

	if (set_nonblocking(sock) < 0)
	{
		goto ERR;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SOCKET_PORT);
	addr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)(&addr), sizeof(struct sockaddr)) < 0)
	{
		lerror("bind socket failed, err: %s", strerror(errno));
		goto ERR;
	}

	if (listen(sock, CLINT_CONNECT_MAX_NUM) < 0)
	{
		lerror("listen socket failed, err: %s", strerror(errno));
		goto ERR;
	}
	
	return sock;
ERR:
	if (sock >= 0)
	{
		close(sock);
	}
	return 0;
}
