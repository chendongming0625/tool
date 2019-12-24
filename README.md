# tool
此工具封装了net_tool iproute2 dpip 提供了一些接口给项目中使用 如获取系统ip列表 路由列表 修改ip等

例如项目中如果想使用ip addr 增加或者删除ip时直接调用ip_addr_do()接口即可 具体使用说明可参考README_libipaddr
若在项目中想使用dpip添加ip 或者获取ip列表 可以调用addr_do()
若在项目中想使用dpip添加路由 或者获取路由列表 可以调用route_do()具体使用说明可参考builddpip/test
