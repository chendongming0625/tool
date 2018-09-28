#include "dpipdo.h"
#include <stdio.h>
#include "cel/arraylist.h"


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

int main(int argc,char **argv)
{
	int i;
	char ip[]="10.21.21.227/32";
	CelArrayList*list = cel_arraylist_new(freefun);
	bool is_exit = false;
	char *arg[5]={ip,"via","192.168.23.1","dev","dpdk1"};
	int ret=route_do(5,arg,DPIP_SHOW,list);
	if(ret!=0)
	{
		printf("ret=%d\n",ret);
		return 0;
	}
	printf("ret=%d\n",ret);
	cel_arraylist_foreach(list,(CelEachFunc)foreach_route_list_cb, &is_exit);
	if (is_exit)
	{
		printf("is exit..\n");
	}
	//printf("nroute=%d\n",cel_arraylist_get_size(list));
	cel_arraylist_free(list);
			
}