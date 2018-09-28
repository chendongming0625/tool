#ifndef _DPIP_do_H__
#define _DPIP_do_H__

#include "cel/arraylist.h"

#define DPIP_ADD 	0
#define DPIP_DEL	1
#define DPIP_SHOW   	3

#ifdef __cplusplus
extern "C" {
#endif

struct dpip_route {
	char             af[16];
	char			 dst[64];    /* all-zero for default */
	char			 via[64];
	char			 src[64];
	char            ifname[32];
	unsigned int    mtu;
	unsigned char   tos;
	unsigned char   scope;
	unsigned char   metric;
	unsigned char   proto;  /* routing protocol */
	unsigned char   flags;
};
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