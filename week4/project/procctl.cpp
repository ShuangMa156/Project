#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h> //wait()函数的头文件
#include<sys/wait.h>  //wait()函数的头文件
int main(int argc,char *argv[])
{
	if (argc < 3) {
		printf("Using:./procctl timetvl program argv ...\n");
		printf("Example:/weather/project/tools1/bin/procctl 5 /usr/bin/tar zcvf /tmp/tmp.tgz /usr/include\n\n");
		printf("本程序是服务程序的调度程序，周期性启动服务程序或shell脚本。\n");
		printf("timeval 运行周期，单位：秒。被调度的程序在运行结束后，在timeval秒后会被procctl重新启动。\n");
		printf("program 被调度的程序名，必须使用全路径。\n");
		printf("argv 被调度程序的参数。\n");
		printf("注意：本程序不会被kill杀死，但可以用kill -9 强行杀死。\n\n");
		return -1;
	}
	//关闭信号和I/O，本程序将不会被打扰
	for (unsigned int i = 0; i < 64; ++i) {
		signal(i, SIG_IGN);
		close(i);//关掉I/O
	}
	//生成子进程，父进程退出，让程序运行在后台，由系统1号进行托管
	if (fork() != 0) {
		exit(0);
	}
	//启用SIGCHLD信号。让父进程可以wait()子进程退出的状态
	signal(SIGCHLD, SIG_DFL);
	//先执行fork()函数，创建一个子进程，让子进程调用execl执行新的程序
	//新程序将替换子进程，不会影响父进程
	//在父进程中，可以调用wait()函数等待新程序运行的结果，这样就可以实现调度的功能
	//最终效果 ./procctl 5 /usr/bin/ls -lt /tmp/docs/sourcecode/project_20220122.tgz  (前两个参数时间间隔个被执行的程序名是必须的)
    //execv()函数用于解决参数不确定的情况
	char* pargv[argc];
	for (unsigned int i = 2; i < argc; ++i) {
		pargv[i - 2] = argv[i];
	}
	pargv[argc - 2] = NULL;
	while (true) {
		if (fork() == 0) { //子进程
			//execl("/usr/bin/ls", "usr/bin/ls", "-lt", "/tmp/docs/sourcecode/project_20220122.tgz", (char*)0);//前两个参数为程序名，后面填ls的参数，最后一个参数用0结束
			//execl(argv[2], argv[2], argv[3], argv[4], (char*)0);
			execv(argv[2], pargv);
			exit(0);//execv()调用失败时执行
		}
		else { //父进程
			int status;
			wait(&status);//调用wait()函数，等待子进程的退出
			sleep(atoi(argv[1]));//每间隔10秒钟执行一次ls
		}
	}
	//execl("/usr/bin/ls", "usr/bin/ls", "-lt", "/tmp/docs/sourcecode/project_20220122.tgz", (char*)0);//前两个参数为程序名，后面填ls的参数，最后一个参数用0结束
	//exec()系列函数用参数中指定的进程替换了当前执行的进程的正文段、数据段、堆和栈
	//printf("bbb\n");//execl()函数调用成功后不会被执行
	return 0;
}
