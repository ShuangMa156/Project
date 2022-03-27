/*程序名称：procheart.cpp
程序功能：将进程的心跳信息写入共享内存
*/
#include "_public.h"
CPActive Active; //进程心跳操作类
void EXIT(int sig) { //退出程序
	printf("sig=%d\n", sig);
	if (sig==2) exit(0); //用exit()函数退出需要将对象定义为全局对象
}
/*注意：exit()函数与析构函数的区别
* （1）调用exit()函数退出的时候不会调用局部对象的析构函数
* （2）调用exit()函数退出的时候会调用全局对象的析构函数
* （3）在main()函数中调用return语句退出，局部对象和全局对象的析构函数都会被调用
*/
int main(int argc, char* argv[])
{
	if (argc!=3) { 
		printf("Using:./book procname timeout\n");
		return 0;
	}
	signal(2, EXIT); //设置2的信号
	signal(15, EXIT); //设置15的信号
	Active.AddPInfo(atoi(argv[2]),argv[1]); //把当前进程的心跳信息加入共享内存进程组中
	while (true) {
		//让程序超时Active.UptATime(); //更新共享内存进程组中当前进程的心跳时间
		sleep(10);
	}
	return 0;
}
