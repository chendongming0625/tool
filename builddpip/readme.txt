此项目是从dpip中抽离出来的
封装了addr_do()和route_do两个接口 用于添加/删除/获取 ip/route
编译:./build
应用:直接复制libdpip.a 跟dpipdo.h builddpip目录下的头文件 到你的项目中即可使用
ps:由于涉及到版权问题 cel目录没有上传 builddpip目录下应有个cel builddpip/cel