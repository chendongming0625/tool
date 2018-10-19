#include "ipaddr.h"
#include <stdio.h>
#include "cel/arraylist.h"
void ip_addr_add_test()
{
	char ip[] = "192.168.22.206/32";
	char dev[] = "dev";
	char inter[] = "dpdk0.kni";
	char *arg[4] = { "add",ip,dev,inter };
	ip_addr_do(4, arg);
	return;
}
int foreach_list_cb(void * value, void * user_data)
{
	struct _InterFaceInfo*list = (struct _InterFaceInfo*)value;
	if (list)
	{
		printf("name:%s,mac %s staus:%s ip_num:%d", list->name, list->mac,list->status,list->ip_num);
		int i;
		for (i = 0; i<list->ip_num; i++)
		{
			printf("ip:%s ", list->ip[i]);
		}
		printf("\n");
	}

	return 0;
}
void my_free(void*p)
{
	printf("free:%p\n",p);
	free(p);
}
void interface_list_free(void*p)
{
	struct _InterFaceInfo*info = (struct _InterFaceInfo*)p;
	int i;
	for (i = 0; i<info->ip_num; i++)
	{
		if (info->ip[i])
		{
			my_free(info->ip[i]);
			info->ip[i] = NULL;
		}
	}
	info->ip_num = 0;
	if (info->ip)
	{
		my_free(info->ip);
		info->ip = NULL;
		if (info)
		{
			my_free(info);
			info = NULL;
		}
	}
	else if (info)
	{
		my_free(info);
		info = NULL;
	}
	return;
}
void test_get_ipaddr_list()
{
	CelArrayList*arr = cel_arraylist_new(interface_list_free);
	int ret = get_ipaddr_list(arr);
	printf("ret=%d\n", ret);
	cel_arraylist_foreach(arr, (CelEachFunc)foreach_list_cb, NULL);
	cel_arraylist_free(arr);
	return ;
}

int main()
{
	//ip_addr_do(0, NULL);
	test_get_ipaddr_list();
}