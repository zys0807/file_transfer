#include "comm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>

int get_file_size(const char *file_path, int *pfile_size)
{
	assert(file_path != NULL);
	assert(pfile_size != NULL);

	struct stat buf;
	if (-1 == stat(file_path, &buf))
	{
		lerror("call stat() failed, err: %s", file_path, strerror(errno));
		return -1;
	}
	*pfile_size = buf.st_size;

	return 0;
}

int read_file(struct trans_data_t *ptrans, char *file_path)
{
	assert(ptrans != NULL);
	assert(ptrans->data_len > 0);
	assert(file_path != NULL);


	FILE *pfile = NULL;
	size_t rlen = 0;

	pfile = fopen(file_path, "r");
	if (NULL == pfile)
	{
		lerror("open file failed, file_path: %s, err:%s", file_path, strerror(errno));
		goto ERR;
	}

	rlen = fread(ptrans->data, ptrans->data_len, 1, pfile);
	if (rlen != ptrans->data_len)
	{
		lerror("not enough to read the data, rlen: %d, data_len: %d", rlen, ptrans->data_len);
		goto ERR;
	}
	fclose(pfile);
ERR:
	if (pfile != NULL)
	{
		fclose(pfile);
	}
	return -1;
}

int write_file(struct trans_data_t *ptran)
{
	assert(ptran != NULL);

	char *filename = ptran->filename;
	FILE *pfile = NULL;
	char file_path[MAX_FILE_PATH_LENGTH] = { '\0' };
	int wlen = 0;

	snprintf(file_path, MAX_FILE_PATH_LENGTH, "%s%s", "server", filename);
	file_path[MAX_FILE_PATH_LENGTH - 1] = '0';

	pfile = fopen(file_path, "w");
	if (NULL == pfile)
	{
		lerror("open file failed, file_path: %s, err:%s", file_path, strerror(errno));
		goto ERR;
	}
	wlen = fwrite(ptran->data, ptran->data_len, 1, pfile);
	if (wlen != ptran->data_len)
	{
		lerror("not enough to write the data, wlen: %d, data_len: %d", wlen, ptran->data_len);
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

int send_data(int sock, struct trans_data_t *ptrans)
{
	assert(sock >= 0);
	assert(ptrans != NULL);

	int total_len = sizeof(struct trans_data_t) + ptrans->data_len;
	int send_len = 0;
	int len = 0;

	while (send_len >= total_len)
	{
		len = send(sock, ptrans + send_len, send_len, 0);
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

int recv_data(int sock, struct trans_data_t *ptrans)
{
	assert(sock > 0);
	assert(ptrans != NULL);

	int recv_len = 0;

	while (1)
	{
		recv_len = recv(sock, ptrans->data, ptrans->data_len, 0);
		if (recv_len < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				continue;
			}
			lerror("recv data failed, data_len: %d, err: %s", ptrans->data_len, strerror(errno));
			return -1;
		}
		else if (recv_len == 0)
		{
			lwarn("client has been shut down");
			return -1;
		}
		break;
	}
	ptrans->data_len = recv_len;
	return recv_len;
}