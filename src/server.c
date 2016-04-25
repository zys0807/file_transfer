#include "comm.h"

/// 自定义epoll最多支持的socket，取值1来自于需求设计
#define MAX_EPOLL_SIZE 1

int process_upload(const struct trans_data_t *ptran)
{
	assert(ptran != NULL);

	ldebug("be called, filename: %s, data_len: %d", ptran.filename, ptran.data_len);
	return write_file(ptran);
}

int process_func(int trans_sock)
{
	assert(trans_sock > 0);

	struct trans_data_t *ptrans = NULL;

	ptrans = (struct trans_data_t *)calloc(1, sizeof(struct trans_data_t) + MAX_TRANSMIT_DATA_SIZE);
	if (NULL == ptrans)
	{
		lerror("calloc memory failed, err: %s", strerror(errno));
		goto ERR;
	}
	ptrans->data_len = MAX_TRANSMIT_DATA_SIZE;

	if (-1 == recv_data(trans_sock, ptrans))
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
			(void)process_download(ptrans);
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

int main(int argc, char **argv)
{
	int listen_sock = -1, trans_sock = -1, event_fd = -1, cur_fds = 0, nfds = 0, index = 0;
	struct sockaddr_in client_addr;
	size_t sock_len = sizeof(struct sockaddr_in);
	struct epoll_event ev = { 0 };
	struct epoll_event wait_process_fds[MAX_EPOLL_SIZE] = { -1 };

	if (-1 == create_server_socket(&listen_sock))
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
	ev.events = EPOLLIN; //在LT模式下工作
	ev.data.fd = listen_sock;

	if (epoll_ctl(event_fd, EPOLL_CTL_ADD, listen_sock, &ev) < 0)
	{
		lerror("call epoll_ctl() failed, err: %s", strerror(errno));
		goto ERR;
	}

	ldebug("listen_sock: %d, event_fd: %d", listen_sock, event_fd);
	ldebug("enter event loop...");
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
				trans_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &sock_len);
				if (-1 == trans_sock)
				{
					lerror("call accept() failed, listen_sock: %d, err: %s", listen_sock, strerror(errno));
					goto ERR;
				}
				(void)process_func(trans_sock);
			}
		}
	}

	if (evnt_fd > 0)
	{
		close(event_fd);
	}
	if (listen_sock > 0)
	{
		close(listen_sock);
	}
	return 0;
ERR:
	if (evnt_fd > 0)
	{
		close(event_fd);
	}
	if (listen_sock > 0)
	{
		close(listen_sock);
	}
	return -1;
}