#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include "comm.h"

static int client_upload(int sock, char *file_path)
{
	assert(file_path != NULL);
	assert(sock >= 0);

	ldebug("be called, file_path: %s", file_path);
	linfo("upload files, please wait a moment...");

	int file_size = 0;
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

	ptrans = (struct trans_data_t *)calloc(1, sizeof(struct trans_data_t) + file_size);
	if (NULL == ptrans)
	{
		lerror("malloc memory failed, err:%s", strerror(errno));
		abort();
	}
	ptrans->data_len = file_size;
	strcpy(ptrans->filename, basename(file_path));
	ptrans->filename[MAX_FILE_NAME_LENGTH - 1] = '0';

	if (-1 == read_file(ptrans, file_path))
	{
		lerror("call read_data() failed, file_path: %s", file_path);
	}
	
	if (-1 == send_data(sock, ptrans))
	{
		lerror("call send_data() failed, sock: %d, data_len: %d", sock, ptrans->data_len);
		return -1;
	}
	return 0;
ERR:
	if (ptrans != NULL)
	{
		free(ptrans);
	}
	return -1;
}

int main(int argc, char **argv)
{
	int ch = 0;
	int cmd = UD_MIN;
	int sock = -1;
	char *src_file_path = NULL, *dst_file_path = NULL;
	struct option log_options[] = {
		{ "upload",		0,	NULL,	'u' },
		{ "download",	0,	NULL,	'd' },
		{ "srcfile",	1,	NULL,	's' },
		{ "dstfile",	1,	NULL,	't' },
	};

	while ((ch == getopt_long(argc, argv, NULL, log_options, NULL)) != EOF)
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
			src_file_path = optarg;
			break;
		case 't':
			dst_file_path = optarg;
			break;
		default:
			lwarn("unknown argument, %s", argv[optind]);
			break;
		}
	}

	if (-1 == check_param(cmd, src_file_path, dst_file_path))
	{
		lerror("check param failed");
		goto ERR;
	}

	if (-1 == create_client_socket(&sock))
	{
		lerror("call create_and_connect_socket() failed");
		goto ERR;
	}
	ldebug("create socket success, sock: %d", sock);

	switch (cmd)
	{
	case UD_UPLOAD:
		(void)client_upload(sock, src_file_path);
		break;
	case UD_DOWNLOAD:
		(void)client_download(sock, src_file_path, dst_file_path);
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
