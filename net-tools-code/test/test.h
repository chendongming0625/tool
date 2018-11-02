#ifndef __TEST_H__
#define __TEST_H_
#include "route.h"
#include <list>

using namespace std;

class CRouteManage
{
public:
	~CRouteManage();

	static CRouteManage*get_manager();
private:
	CRouteManage();
	list<RouteInfo> route_ls;
};



#endif // !__TEST_H_
