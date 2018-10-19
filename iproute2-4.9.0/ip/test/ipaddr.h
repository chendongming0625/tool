#ifndef __IP_ADDR_H__
#define __IP_ADDR_H__
#include "cel/arraylist.h"
#include "ifconfig.h"

#ifdef __cplusplus
extern "C" {
#endif
/*����:���/ɾ��ip (�ײ����ip addr add/delԴ��ʵ��)
@param:argc ��������
@param:argv ָ������(�Ѳ����Ž�ָ�������� 
	��:char *arg[4] = { "add","192.168.19.2","dev","dpdk0.kni" })
@return:0�ɹ� ��0ʧ��
*/

	int ip_addr_do(int argc, char **argv);

/*����:��ȡ�����б�
@param:arr �б�(newʱ�����ûص�free_interface_list_fun())
@return:0�ɹ� ��0ʧ��
*/
	int get_ipaddr_list(CelArrayList*arr);
#ifdef __cplusplus
}
#endif



#endif
