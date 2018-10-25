#ifndef _DPIP_do_H__
#define _DPIP_do_H__

#include "cel/arraylist.h"
#include "ifconfig.h"
#define DPIP_ADD 		0
#define DPIP_DEL		1
#define DPIP_SHOW		3
#ifdef __cplusplus
extern "C" {
#endif

struct _RouteInfo {
	char			id[64];
	char			 dst[32];    /* all-zero for default */
	char			 gateway_addr[32];
	char			 src[32];
	char            ifname[32];
	char			netmask[32];
	char			flags[8];
	unsigned char   metric;
};

/*功能:获取dpdk虚拟网卡列表
@param:arr 列表(new时须设置回调free_interface_list_fun())
@return:0成功 非0失败
*/
int get_dpdk_interface_list(CelArrayList*arr);

/*功能:利用dpip添加/删除ip
@param:argc参数个数
@param:argv参数列表如char*arg[]={ip,"dev","dpdk0"};
@param:cmd DPIP_ADD/DPIP_DEL
ps:由于ip接口里面会进行替换,故不能传常量
*/
int addr_do(int argc, char **argv, int cmd);

/*功能:利用dpip添加/删除route
@param:argc参数个数
@param:argv参数列表如char*arg[]={ip,"dev","via","192.168.23.1","dev","dpdk1"};
@param:cmd DPIP_ADD/DPIP_DEL/DPIP_SHOW
@param:当需要获取路由列表时传arr进去 会返回路由列表 
ps:由于ip接口里面会进行替换,故不能传常量
*/
int route_do(int argc, char**argv, int cmd,CelArrayList*arr);
#ifdef __cplusplus
}
#endif
#endif