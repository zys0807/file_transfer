#ifndef __UD_COMM_H__
#define __UD_COMM_H__

#include <assert.h>
#include <stdio.h>

#ifdef _cpulspuls
extern "C" {
#endif

	enum {
		UD_MIN = 0,
		UD_UPLOAD,
		UD_DOWNLOAD,
		UD_MAX,
	};

#define ASSERT_CMD_TYPE(cmd) assert(cmd > UD_MIN && cmd < UD_MAX)

	const char *string_cmd(int cmd_type);

#define MAX_FILE_NAME_LENGTH (64)
#define MAX_FILE_PATH_LENGTH (256)
	/// 目前最大支持一次传输数据的字节数
#define MAX_TRANSMIT_DATA_SIZE (4*1024*1024)
	/// 服务端数据存放的位置
#define  STORE_SERVER_DATA_PATH "/tmp/server/"

	struct trans_data_t
	{
		int		cmd;								/**< 取枚举值*/
		char	filename[MAX_FILE_NAME_LENGTH];		/**< 文件名字*/
		int		data_len;							/**< 数据长度*/
		char	data[0];							/**< 数据*/
	};

	#define linfo(fmt, args...)		printf("info-[%s][%s][%d]-"fmt"%c", __FILE__, __FUNCTION__, __LINE__, ##args, '\n')
	#define ldebug(fmt, args...)	printf("dbg -[%s][%s][%d]-"fmt"%c", __FILE__, __FUNCTION__, __LINE__, ##args, '\n')
	#define lwarn(fmt, args...)		printf("warn-[%s][%s][%d]-"fmt"%c", __FILE__, __FUNCTION__, __LINE__, ##args, '\n')
	#define lerror(fmt, args...)	printf("err -[%s][%s][%d]-"fmt"%c", __FILE__, __FUNCTION__, __LINE__, ##args, '\n')

	int get_file_size(const char *file_path, int *pfile_size);
	int read_file(struct trans_data_t *ptrans, const char *file_path);
	int write_file(const struct trans_data_t *ptran, const char *file_path);
	int send_data(int sock, const void* buff, int buf_len);
	int recv_data(int sock, void *buff, int max_len);
	inline int send_cmd(int sock, const struct trans_data_t *ptrans);

	inline int get_trans_data_t_size(const struct trans_data_t *ptrans);
#ifdef _cpulspuls
};
#endif
#endif