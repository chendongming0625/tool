#include "ipaddr.h"

int main()
{
	char ip[] = "192.168.22.206/32";
	char dev[] = "dev";
	char inter[] = "dpdk0.kni";
	char *arg[4] = { "add",ip,dev,inter };
	ip_addr_do(4, arg);
	return 0;
}