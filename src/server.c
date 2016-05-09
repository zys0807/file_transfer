#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "comm.h"
#include "socket.h"

/// 自定义epoll最多支持的socket，取值1来自于需求设计
#define MAX_EPOLL_SIZE 1

static int process_upload(const struct trans_data_t *ptrans)
{
	assert(ptrans != NULL);

	ldebug("be called, filename: %s, size: %d", ptrans->filename, ptrans->data_len);

	char file_path[MAX_FILE_PATH_LENGTH] = { '\0' };
	snprintf(file_path, MAX_FILE_PATH_LENGTH-1, "%s%s", STORE_SERVER_DATA_PATH, ptrans->filename);
	file_path[MAX_FILE_PATH_LENGTH - 1] = '0';

	return write_file(ptrans, file_path);
}

static int process_download(int sock, struct trans_data_t *ptrans)
{
	assert(ptrans != NULL);
	
	char file_path[MAX_FILE_PATH_LENGTH] = { '\0' };
	snprintf(file_path, MAX_FILE_PATH_LENGTH-1, "%s%s", STORE_SERVER_DATA_PATH, ptrans->filename);
	file_path[MAX_FILE_PATH_LENGTH-1] = '\0';

	if (-1 == get_file_size(file_path, &(ptrans->data_len)))
	{
		lwarn("call get_fail_size failed failed");
		goto FAIL;
	}

	ldebug("be called, filename: %s, size: %d", ptrans->filename, ptrans->data_len);

	if (-1 == read_file(ptrans, file_path))
	{
		lerror("read file failed, file_path: %s", file_path);
		goto FAIL;
	}

	if (-1 == send_data(sock, ptrans, get_trans_data_t_size(ptrans)))
	{
		lerror("send data failed, sock: %d, data_len: %d", sock, ptrans->data_len);
		goto ERR;
	}
	return 0;
FAIL:
	assert(0); //TODO：未完成
	return -1;
ERR:
	return -1;
}

int process_func(int trans_sock)
{
	assert(trans_sock > 0);

	struct trans_data_t *ptrans = NULL;
	int max_len = sizeof(struct trans_data_t) + MAX_TRANSMIT_DATA_SIZE;

	ptrans = (struct trans_data_t *)calloc(1, max_len);
	if (NULL == ptrans)
	{
		lerror("calloc memory failed, err: %s", strerror(errno));
		goto ERR;
	}
	ptrans->data_len = MAX_TRANSMIT_DATA_SIZE;

	if (-1 == recv_data(trans_sock, ptrans, max_len))
	{
		lerror("call recv_data() failed");
		goto ERR;
	}

	switch (ptrans->cmd)
	{
		case UD_UPLOAD:
			(void)process_upload(ptrans);
			break;
		case UD_DOWNLOAD:
			(void)process_download(trans_sock, ptrans);
			break;
		default:
			lerror("unknown cmd type, cmd: %d", ptrans->cmd);
			break;
	}

	free(ptrans);

	return 0;
ERR:
	if (NULL != ptrans)
	{
		free(ptrans);
	}
	return -1;
}

int main()
{
	int listen_sock = -1, trans_sock = -1, event_fd = -1, cur_fds = 0, nfds = 0, index = 0;
	struct sockaddr_in client_addr;
	size_t sock_len = sizeof(struct sockaddr_in);
	struct epoll_event ev = { 0 };
	struct epoll_event wait_process_fds[MAX_EPOLL_SIZE];

	if (access(STORE_SERVER_DATA_PATH, F_OK) < 0)
	{
		if (mkdir(STORE_SERVER_DATA_PATH, 0) < 0)
		{
			lerror("mkdir failed, path: %s, err: %s", STORE_SERVER_DATA_PATH, strerror(errno));
			return -1;
		}
	}

	listen_sock = create_server_socket();
	if (-1 == listen_sock)
	{
		lerror("call create_server_socket() failed");
		goto ERR;
	}

	event_fd = epoll_create(MAX_EPOLL_SIZE);
	if (-1 == event_fd)
	{
		lerror("call epoll_create() failed, MAX_EPOLL_SIZE: %d, err: %s", MAX_EPOLL_SIZE, strerror(errno));
		goto ERR;
	}
	ev.events = EPOLLIN | EPOLLET; //在LT模式下工作
	ev.data.fd = listen_sock;

	if (epoll_ctl(event_fd, EPOLL_CTL_ADD, listen_sock, &ev) < 0)
	{
		lerror("call epoll_ctl() failed, err: %s", strerror(errno));
		goto ERR;
	}

	ldebug("listen_sock: %d, event_fd: %d", listen_sock, event_fd);
	ldebug("enter event loop...");

	cur_fds = 1;
	while (1)
	{
		nfds = epoll_wait(event_fd, wait_process_fds, cur_fds, -1);
		if (-1 == nfds)
		{
			lerror("call epoll_wait() failed, err: %s", strerror(errno));
			goto ERR;
		}
		for (index = 0; index < nfds; index++)
		{
			if (wait_process_fds[index].data.fd == listen_sock)
			{
				trans_sock = accept(listen_sock, (struct sockaddr *)(&client_addr), (socklen_t *)&sock_len);
				if (-1 == trans_sock)
				{
					lerror("call accept() failed, listen_sock: %d, err: %s", listen_sock, strerror(errno));
					goto ERR;
				}
				(void)process_func(trans_sock);
			}
		}
	}

	if (event_fd > 0)
	{
		close(event_fd);
	}
	if (listen_sock > 0)
	{
		close(listen_sock);
	}
	return 0;
ERR:
	if (event_fd > 0)
	{
		close(event_fd);
	}
	if (listen_sock > 0)
	{
		close(listen_sock);
	}
	return -1;
}