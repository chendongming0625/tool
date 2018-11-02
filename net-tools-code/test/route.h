#ifndef __ROUTE_H__
#define __ROUTE_H__
#include "cel/arraylist.h"
#define ACTION_ADD			1
#define ACTION_DEL			2
#define ERRBUF_SIZE			128
typedef struct _RouteInfo {
	char		id[64];
	char		dst[32];    /* all-zero for default */
	char		gateway_addr[32];
	char		src[32];
	char            ifname[32];
	char		netmask[32];
	char		flags[8];
	unsigned char   metric;
}RouteInfo;

#ifdef __cplusplus
extern "C" {
#endif
	/*获取系统路由表
	返回值:路由列表 用完需要释放
	*/
	CelArrayList *get_route_info();
	
	/*添加/删除路由
	@args:路由信息
	@action:ACTION_ADD/ACTION_DEL
	@err_msg:错误信息缓存buffer 大小必须大于或等于128
	@返回值:成功0 失败 非0 错误信息存在err_msg中*/
	int set_route(int action, char**args,char *err_msg);
#ifdef __cplusplus
}
#endif
#endif
