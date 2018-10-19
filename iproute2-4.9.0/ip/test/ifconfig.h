#ifndef __IF_CONFIG_H__
#define __IF_CONFIG_H__
#include "cel/arraylist.h"

#define IF_NAME_SIZE	128
#define MACSIZE			32
#define STATUSSIZE		8
#define IPSIZE			32
#define SPEEDSIZE		32

typedef struct _InterFaceInfo {
	char name[IF_NAME_SIZE];
	char mac[MACSIZE];
	char status[STATUSSIZE];
	char **ip;
	int ip_num;
	char speed[SPEEDSIZE];
}InterFaceInfo;
#ifdef __cplusplus
extern "C" {
#endif
	/*功能:获取本机网卡列表信息或指定网卡信息
	@interface_name:指定网卡名
	@返回值:CelArrayList* 网卡列表 用完需释放(成员也需要释放)
	*/
	CelArrayList *get_interface_info(const char *interface_name);

	/*功能:修改网卡信息(up/down/ip)
	@if_name:网卡名
	@ip_addr:ip地址 当只需修改状态时 传NULL
	@status:网卡状态 当只需修改ip时 传NULL
	@return:成功:0 失败 -1
	*/
	int edit_if(char *if_name, char *ip_addr, char *status);
#ifdef __cplusplus
}
#endif
#endif