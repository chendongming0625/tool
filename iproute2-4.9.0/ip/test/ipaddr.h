#ifndef __IP_ADDR_H__
#define __IP_ADDR_H__
#include "cel/arraylist.h"
#include "ifconfig.h"

#ifdef __cplusplus
extern "C" {
#endif
/*功能:添加/删除ip (底层调用ip addr add/del源码实现)
@param:argc 参数个数
@param:argv 指针数组(把参数放进指针数组中 
	如:char *arg[4] = { "add","192.168.19.2","dev","dpdk0.kni" })
@return:0成功 非0失败
*/

	int ip_addr_do(int argc, char **argv);

/*功能:获取网卡列表
@param:arr 列表(new时须设置回调free_interface_list_fun())
@return:0成功 非0失败
*/
	int get_ipaddr_list(CelArrayList*arr);
#ifdef __cplusplus
}
#endif



#endif
