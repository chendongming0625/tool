此项目是从dpip中抽离出来的
封装了addr_do()和route_do两个接口 用于添加/删除/获取 ip/route
编译:./build
{
gcc -c *.c -D DPVS_MAX_LCORE=64 -I./cel/include 
ar crv libdpip.a *.o
rm -f *.o
}
使用:直接复制libdpip.a 跟dpipdo.h 到你的项目中即可使用
test目录下有使用例子