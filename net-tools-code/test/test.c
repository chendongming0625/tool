#include <stdio.h>
#include "arraylist.h"
#include "ifconfig.h"
#include "route.h"


 int arraylist_print_interface(void *value, void *user_data)
{
	
	 InterFaceInfo *info =(InterFaceInfo*) value;
	if (info)
	{
		printf("name=%s,ip=%s,mac=%s,status=%s\n", info->name,info->ip, info->mac,info->status);
	}
	 return 0;
 }

int arraylist_print_route(void *value, void *user_data)
{
	struct _RouteInfo *info = (struct _RouteInfo*)value;
	if (info)
	{
		 printf("id:%s,dst:%s gate:%s mask:%s flags:%s metric:%d name:%s\n",
		 info->id,info->dst, info->gateway_addr, info->netmask,info->flags,info->metric,info->ifname);
	}
	 return 0;
}

void test_route()
 {
	 CelArrayList *list=get_route_info();
	 if (list)
	 {
		 printf("Kernel IP routing table\n");

		 printf("Destination     Gateway         Genmask         "
		"Flags Metric Ref    Use Iface\n");
		 cel_arraylist_foreach(list, arraylist_print_route, NULL);
		 cel_arraylist_free(list);
		 return ;
	}
	 else
		printf("get_route_info failed\n");
	return;
}
void route_add()
{
	char *args[] = { "net","10.21.21.227","netmask",
	"255.255.255.255","gw","192.168.68.2","dev","eno16777736",NULL };
	char errbuf[128] = {0};
	int ret=set_route(1,args,errbuf);
	if (ret != 0)
	{
		printf("err:%s\n",errbuf);
	}
	return;
}

int main(int argc,char **argv)
{
	route_add();
	test_route();
	return 0;
}
