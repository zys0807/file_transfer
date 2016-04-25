#ifndef __UD_COMM_H__
#define __UD_COMM_H__

#ifdef _cpulspuls
extern "C" {
#endif

	enum {
		UD_MIN = 0,
		UD_UPLOAD,
		UD_DOWNLOAD,
		UD_MAX,
	};

#define MAX_FILE_NAME_LENGTH (256)
#define MAX_FILE_PATH_LENGTH (512)
	/// 目前最大支持一次传输数据的字节数
#define MAX_TRANSMIT_DATA_SIZE (4*1024*1024)

	struct trans_data_t
	{
		int		cmd;								/**< 取枚举值*/
		char	filename[MAX_FILE_NAME_LENGTH];		/**< 文件名字*/
		int		data_len;							/**< 数据长度*/
		char	data[0];							/**< 数据*/
	};

	#define linfo(fmt, args...)		printf("info,	[%s][%d], "fmt"%c", __FUNCTION__, __LINE__, ##args, '\n')
	#define ldebug(fmt, args...)	printf("debug,	[%s][%d], "fmt"%c", __FUNCTION__, __LINE__, ##args, '\n')
	#define lwarn(fmt, args...)		printf("warn,	[%s][%d], "fmt"%c", __FUNCTION__, __LINE__, ##args, '\n')
	#define lerror(fmt, args...)	printf("err,	[%s][%d], "fmt"%c", __FUNCTION__, __LINE__, ##args, '\n')

	int get_file_size(const char *file_path, int *pfile_size);

#ifdef _cpulspuls
};
#endif
#endif