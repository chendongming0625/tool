#ifndef __IF_CONFIG_H__
#define __IF_CONFIG_H__
#include "cel/arraylist.h"
typedef struct _InterFaceInfo {
	char name[128];
	char mac[32];
	char status[8];
	char ip[32];
	char speed[32];
}InterFaceInfo;
#ifdef __cplusplus
extern "C" {
#endif
	/*功能:获取本机网卡列表信息或指定网卡信息
	@interface_name:指定网卡名
	@返回值:CelArrayList* 网卡列表 用完需释放(成员也需要释放)
	*/
	CelArrayList *get_interface_info(const char *interface_name);
#ifdef __cplusplus
}
#endif
#endif