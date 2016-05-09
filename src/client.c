#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include "comm.h"
#include "socket.h"


static int client_upload(int sock, char *file_path)
{
	assert(file_path != NULL);
	assert(sock >= 0);

	ldebug("upload file(%s), please wait a moment...", file_path);

	int file_size = 0;
	int total_len = 0;
	struct trans_data_t *ptrans = NULL;

	if (-1 == get_file_size(file_path, &file_size))
	{
		lerror("call get_file_size() failed");
		goto ERR;
	}
	if (!file_size)
	{
		lwarn("file is null, file_path: %s", file_path);
		goto ERR;
	}

	total_len = sizeof(struct trans_data_t) + file_size;
	ptrans = (struct trans_data_t *)calloc(1, total_len);
	if (NULL == ptrans)
	{
		lerror("malloc memory failed, err:%s", strerror(errno));
		abort();
	}
	ptrans->cmd = UD_UPLOAD;
	ptrans->data_len = file_size;
	snprintf(ptrans->filename, MAX_FILE_NAME_LENGTH-1, "%s", basename(file_path));
	ptrans->filename[MAX_FILE_NAME_LENGTH - 1] = '0';

	if (-1 == read_file(ptrans, file_path))
	{
		lerror("call read_data() failed, file_path: %s", file_path);
	}
	
	if (-1 == send_data(sock, ptrans, get_trans_data_t_size(ptrans)))
	{
		lerror("call send_data() failed, sock: %d, data_len: %d", sock, ptrans->data_len);
		return -1;
	}

	ldebug("upload file success");
	return 0;
ERR:
	lwarn("upload file failed");
	if (ptrans != NULL)
	{
		free(ptrans);
	}
	return -1;
}

static int client_download(int sock, const char *file_name)
{
	assert(sock > 0);
	assert(file_name != NULL);
	
	ldebug("download file(%s), please wait a moment...", file_name);

	struct trans_data_t *ptrans = NULL;
	int max_len = sizeof(struct trans_data_t) + MAX_TRANSMIT_DATA_SIZE;

	ptrans = (struct trans_data_t *)calloc(1, max_len);
	if (NULL == ptrans)
	{
		lerror("calloc failed, err: %s", strerror(errno));
		return -1;
	}
	ptrans->cmd = UD_DOWNLOAD;
	ptrans->data_len = 0;
	snprintf(ptrans->filename, MAX_FILE_NAME_LENGTH-1, file_name);
	ptrans->filename[MAX_FILE_NAME_LENGTH-1] = '\0';

	if (-1 == send_cmd(sock, ptrans))
	{
		lerror("send cmd(%s) failed", string_cmd(ptrans->cmd));
		goto ERR;
	}

	if (-1 == recv_data(sock, ptrans, max_len))
	{
		lerror("recv data failed, sock: %d", sock);
		goto ERR;
	}
	ldebug("data: %s", ptrans->data);

	ldebug("download file success");
	return 0;
ERR:
	if (NULL != ptrans)
	{
		free(ptrans);
	}
	return -1;
}

static int check_param(int cmd, const char *file)
{
	ASSERT_CMD_TYPE(cmd);
	assert(file != NULL);
	return 0;
}

static void help()
{
	printf("Usage: ./client.c [-u --upload] [-d --download] [-f --file] file\n");
}

int main(int argc, char **argv)
{
	int ch = 0;
	int cmd = UD_MIN;
	int sock = -1;
	char *file = NULL;
	char * const short_options = "uds:t:";
	struct option log_options[] = {
		{ "upload",		0,	NULL,	'u' },
		{ "download",	0,	NULL,	'd' },
		{ "file",	1,	NULL,	's' },
	};

	if (argc < 2)
	{
		help();
		return -1;
	}

	while ((ch = getopt_long(argc, argv, short_options, log_options, NULL)) != EOF)
	{
		switch (ch)
		{
		case 'u':
			cmd = UD_UPLOAD;
			break;
		case 'd':
			cmd = UD_DOWNLOAD;
			break;
		case 's':
			file = optarg;
			break;
			break;
		default:
			lwarn("unknown argument, %s", argv[optind]);
			break;
		}
	}

	if (-1 == check_param(cmd, file))
	{
		lerror("check param failed");
		help();
		goto ERR;
	}

	linfo("cmd: %s, file: %s", string_cmd(cmd), file);

	sock = create_client_socket();
	if (-1 == sock)
	{
		lerror("call create_and_connect_socket() failed");
		goto ERR;
	}
	ldebug("create socket success, sock: %d", sock);

	switch (cmd)
	{
	case UD_UPLOAD:
		(void)client_upload(sock, file);
		break;
	case UD_DOWNLOAD:
		(void)client_download(sock, file);
		break;
	default:
		break;
	}

	return 0;
ERR:
	if (sock >= 0)
	{
		close(sock);
	}
	return -1;
}
