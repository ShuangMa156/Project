/*#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
void handler(int sig) {//信号处理函数，sig为接收到的信号的值
	printf("接受到了%d信号.\n", sig);//打印收到的信号
}
void alarmfunc(int num) { //时钟信号的处理函数
	printf("接受到了%d信号.\n", num);//打印收到的信号
	alarm(3);//每3秒钟发出一次时钟信号
}
int main() 
{
	for (unsigned int i = 1; i <= 64; ++i) { //Linux中的信号有64种，9的信号是强制杀死程序的信号（不能被捕获，也不能被忽略）
		signal(i, handler);//singal()函数用于设置信号的处理方法
	}
	//signal(15, SIG_IGN);//忽略信号，第一个参数为信号的值，第二个参数为固定的宏
	//signal(15, SIG_DFL);//将信号设置为系统缺省的动作，第一个参数为信号的值，第二个参数为固定的宏
	signal(SIGALRM, alarmfunc);//时钟信号的处理动作（只收到一次）
	alarm(3);//设置闹钟，时间为3秒
	while (1) { //程序无限循环，每一秒钟执行一次文本
		printf("执行了一次任务。\n");
		sleep(1);
	}
}*/
/*Linux下程序的终止：按ctrl+C 或者输入killall（向正在运行的程序发送信号---系统缺省的对信号的处理方法为终止程序的运行）
*/
/*
* windows下支持，需要引入头文件windows.h，切记Sleep首字母大写
    #include <windows.h>
    Sleep(5000);
linux下支持，需要引入头文件unistd.h
    #include <iostream>
    #include <unistd.h>
    using namespace std;
    int main()
   {
	    //5秒后输出Hi,Gril!
	    sleep(5);
	    std::cout << "Hi,Gril!" << std::endl;
 
	   //3000000微妙(相当于3秒)输出Hi,Boy!
	   usleep(3000000);
	   std::cout << "Hi,Boy!" << std::endl;
	   return 0;
   }
*/
//用信号通知服务程序退出
/*#include<stdio.h> //printf()函数的头文件
#include<signal.h> 
void EXIT(int sig) {//信号处理函数，sig为接收到的信号的值
	printf("接受到了%d信号,程序将退出。\n", sig);//打印收到的信号
	//在这里编写善后代码
	0(0);//程序退出
}
int main()
{
	for (unsigned int i = 1; i <= 64; ++i) {
		signal(i, SIG_IGN);//忽略所有信号
	}
	signal(SIGINT, EXIT);//信号2的处理函数,SIGINT为信号2（Ctrl+c）的信号名
	signal(SIGTERM, EXIT);//信号2的处理函数,SIGITERM为信号15（kill,killall）的信号名
	while (1) { //程序无限循环，每一秒钟执行一次文本
		printf("执行了一次任务。\n");
		sleep(1);
	}
}
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> //Linux中支持延时的头文件，不同于Windows
#include<signal.h>
#include<sys/types.h> //wait()函数的头文件
#include<sys/wait.h>  //wait()函数的头文件
void func(int sig) { //信号处理函数
	int sts;
	wait(&sts);
}
int main()
{
	//signal(SIGCHLD, SIG_IGN);//方法一：忽略信号SIGCHLD
	signal(SIGCHLD, func);//信号发出后，软中断父进程的进行①
	int pid = fork();//获取fork()函数的返回值
	if (pid == 0) {//子进程
		printf("这是子进程%d,将执行子进程的任务。\n", getpid());
		sleep(5);
	}
	if (pid > 0) {//父进程
		printf("这是父进程%d,将执行父进程的任务。\n", getpid());
		//在父进程中增加子进程退出的程序----->会阻塞父进程的继续执行
		//int sts;
		//wait(&sts);//wait()函数可以等待子进程的退出
		sleep(10);
	}
}
