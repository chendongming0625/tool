#include "dpipdo.h"
#include <stdio.h>
#include "cel/arraylist.h"
#include "ifconfig.h"

int foreach_route_list_cb(void * value, void * user_data)
{
	printf("foreach_route_list_cb...\n");
	struct dpip_route*list = (struct dpip_route*)value;
	if (list)
		printf(":%s,dev %s\n", list->dst, list->ifname);
	if (strncmp("192.168.20.2/32", list->dst, sizeof("192.168.20.2/32")))
	{
		*((bool*)user_data) = true;
	}
	return 0;
}

void freefun(void*p)
{
	if (p)
	{
		printf("addr:%p\n", p);
		free(p);
		p = NULL;
	}
		
}
int foreach_dpdk_list_cb(void * value, void * user_data)
{
	struct _InterFaceInfo*list = (struct _InterFaceInfo*)value;
	if (list)
	{
		printf("%s,dev %s", list->name, list->mac);
		for(int i=0;i<list->ip_num;i++)
		{
			printf(":%s\n",list->ip[i]);
		}
		
	}
		
	return 0;
}
void interface_list_free(void*p)
{
	struct _InterFaceInfo*info = (struct _InterFaceInfo*)p;
	for(int i=0;i<info->ip_num;i++)
	{
		if (info->ip[i])
		{
			free(info->ip[i]);
			info->ip[i] = NULL;
		}
	}
	info->ip_num=0;
	if (info->ip)
	{
		free(info->ip);
		info->ip = NULL;
		if(info)
		{
			free(info);
			info=NULL;
		}
	}
	return;
}
int dpdk_interface_test()
{
	CelArrayList*list = cel_arraylist_new(free_interface_list_fun);
	int ret=get_dpdk_interface_list(list);
	printf("ret=%d\n",ret);
	cel_arraylist_foreach(list,(CelEachFunc)foreach_dpdk_list_cb,NULL);
	cel_arraylist_free(list);
}

int main(int argc,char **argv)
{
	dpdk_interface_test();
	// int i;
	// char ip[]="10.21.21.227/32";
	// CelArrayList*list = cel_arraylist_new(freefun);
	// bool is_exit = false;
	// char *arg[5]={ip,"via","192.168.23.1","dev","dpdk1"};
	// int ret=addr_do(0,NULL,DPIP_SHOW);
	// if(ret!=0)
	// {
		// printf("ret=%d\n",ret);
		// return 0;
	// }
	//printf("ret=%d\n",ret);
	// cel_arraylist_foreach(list,(CelEachFunc)foreach_route_list_cb, &is_exit);
	// if (is_exit)
	// {
		// printf("is exit..\n");
	// }
	//printf("nroute=%d\n",cel_arraylist_get_size(list));
	//cel_arraylist_free(list);
	return 0;		
}