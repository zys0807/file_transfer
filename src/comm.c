#include "comm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

const char *string_cmd(int cmd_type)
{
	ASSERT_CMD_TYPE(cmd_type);

	static const char *string_cmd[] = { "UD_MIN", "UD_UPLOAD", "UD_DOWNLOAD", "UD_MAX" };
	return string_cmd[cmd_type];
}

inline int get_trans_data_t_size(const struct trans_data_t *ptrans)
{
	return sizeof(struct trans_data_t) + ptrans->data_len;
}

int get_file_size(const char *file_path, int *pfile_size)
{
	assert(file_path != NULL);
	assert(pfile_size != NULL);

	struct stat buf;
	if (-1 == stat(file_path, &buf))
	{
		lerror("call stat() failed, file_path: %s, err: %s", file_path, strerror(errno));
		return -1;
	}
	*pfile_size = buf.st_size;

	return 0;
}

int read_file(struct trans_data_t *ptrans, const char *file_path)
{
	assert(ptrans != NULL);
	assert(ptrans->data_len > 0);
	assert(file_path != NULL);


	FILE *pfile = NULL;
	int nmemb = 0;

	pfile = fopen(file_path, "r");
	if (NULL == pfile)
	{
		lerror("open file failed, file_path: %s, err:%s", file_path, strerror(errno));
		goto ERR;
	}

	nmemb = 1;
	nmemb = fread(ptrans->data, ptrans->data_len, nmemb, pfile);
	if (nmemb != 1)
	{
		lerror("not enough to read the data, rlen: %d, data_len: %d", nmemb, ptrans->data_len);
		goto ERR;
	}
	fclose(pfile);
	return 0;
ERR:
	if (pfile != NULL)
	{
		fclose(pfile);
	}
	return -1;
}

int write_file(const struct trans_data_t *ptran, const char *file_path)
{
	assert(ptran != NULL);
	assert(file_path != NULL);
	
	FILE *pfile = NULL;
	int nmemb = 0;

	pfile = fopen(file_path, "w");
	if (NULL == pfile)
	{
		lerror("open file failed, file_path: %s, err:%s", file_path, strerror(errno));
		goto ERR;
	}
	nmemb = 1;
	nmemb = fwrite(ptran->data, ptran->data_len, nmemb, pfile);
	if (nmemb != 1)
	{
		lerror("not enough to write the data, wlen: %d, data_len: %d", nmemb, ptran->data_len);
		unlink(file_path);
		goto ERR;
	}
	fclose(pfile);
	return 0;
ERR:
	if (pfile != NULL)
	{
		fclose(pfile);
	}
	return -1;
}

int send_data(int sock, const void* buff, int buf_len)
{
	assert(sock >= 0);
	assert(buff != NULL);

	int send_len = 0;
	int len = 0;

	while (send_len < buf_len)
	{
		len = send(sock, buff + send_len, buf_len - send_len, 0);
		if (len < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			return -1;
		}
		else if (len == 0)
		{
			lwarn("server has been shut down");
			return -1;
		}
		send_len += len;
	}
	return 0;
} 

int recv_data(int sock, void *buff, int max_len)
{
	assert(sock > 0);
	assert(buff != NULL);

	int recv_len = 0;

	while (1)
	{
		recv_len = recv(sock, buff, max_len, 0);
		if (recv_len < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			lerror("recv data failed, recv_len: %d, err: %s", recv_len, strerror(errno));
			return -1;
		}
		else if (recv_len == 0)
		{
			lwarn("client has been shut down");
			return -1;
		}
		break;
	}
	return 0;
}

inline int send_cmd(int sock, const struct trans_data_t *ptrans)
{
	return send_data(sock, ptrans, get_trans_data_t_size(ptrans));
}