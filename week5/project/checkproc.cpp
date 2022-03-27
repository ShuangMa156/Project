/*
* 程序名：checkproc.cpp
* 程序功能：用于检查后台服务程序是否超时，如果已超时，就终止它。
*/
#include "_public.h"
CLogFile logfile; //程序的运行日志
int main(int argc, char* argv[]) 
{
	//程序的帮助文档
	if (argc != 2) {
		printf("\n");
		printf("Using:./checkproc logfilename\n");
		printf("Example:/weather/project/tools1/bin/procctl 10 /weather/project/tools1/bin/checkproc /tmp/log/checkproc.log\n\n");
		printf("本程序用于检查后台服务程序是否超时，如果已超时，就终止它。\n");
		printf("注意：\n");
		printf("1)本程序由procctl启动，运行周期建议为10秒。\n");
		printf("2)为了避免被普通用户误杀，本程序应该用root用户启动。\n");//普通用户无法kill掉root用户启动的程序
		printf("3)如果要停止本程序，只能用killall -9 终止。\n\n");
		return 0;
	}
	//忽略全部的信号，不希望程序被干扰
	CloseIOAndSignal(true);
	//打开日志文件
	if (logfile.Open(argv[1], "a+") == false) {
		printf("logfile.Open(%s) failed.\n", argv[1]);
		return -1;
	}
	int shmid = 0; //共享内存的id
	//创建/获取共享内存(守护进程或服务程序),简直为SHMKEYP,大小为MAXNUMP个st_procinfo结构体的大小
	if ((shmid = shmget((key_t)SHMKEYP, MAXNUMP * sizeof(struct st_procinfo), 0600 | IPC_CREAT)) == -1) {
		logfile.Write("创建、获取共享内存(%x)失败。\n", SHMKEYP);
		return -1; //有疑问，作者写的是return false
	}
	//将共享内存连接到当前进程的地址空间
	struct st_procinfo* shm = (struct st_procinfo*)shmat(shmid, 0, 0);
	//遍历共享内存中的全部记录
	for (unsigned int i = 0; i < MAXNUMP; ++i) {
		//如果记录的pid==0,表示空记录，continue
		if (shm[i].pid == 0) {
			continue;
		}
		//如果记录的pid!=0，表示是服务程序的心跳记录（日志便于测试程序是否正常进行，服务程序稳定运行后可不写日志记录）
		logfile.Write("i=%d,pid=%d,pname=%s,timeout=%d,atime=%d\n", i, shm[i].pid, shm[i].pname, shm[i].timeout, shm[i].atime);
		//向进程发送信号0,判断它是否还存在，如果不存在，从共享内存中删除记录，continue
		int iret = kill(shm[i].pid, 0); //通过kill函数的返回值判断进程是否存在（kill函数，如果进程不存在返回-1，进程存在返回0）
		if (iret == -1) {
			//记录日志
			logfile.Write("进程pid=%d(%s)已经不存在。\n", shm[i].pid, shm[i].pname);
			//清零结构体，从共享内存中删除记录
			memset(shm+i, 0, sizeof(struct st_procinfo)); //memset函数
			continue;
		}
		//如果进程未超时，continue（判断方法：当前时间与最后一次心跳的时间的差值与进程的超时时间进行比较）
		time_t now = time(0); //获取当前时间
		if ((now - shm[i].atime) < shm[i].timeout) {
			continue; //未超时则继续
		}
		//如果已超时，发送信号15，尝试终止进程
		logfile.Write("进程pid=%d(%s)已经超时。\n", shm[i].pid, shm[i].pname);
		kill(shm[i].pid, 15); //发送信号15，尝试终止进程
		//每隔1秒判断一次进程是否存在，累计5秒（一般来说，5秒的时间足够让进程退出）
		for (unsigned int j = 0; j < 5; ++j) {
			sleep(1);
			iret = kill(shm[i].pid, 0);
			if (iret == -1) { //进程已退出
				break;
			}
		}
		//如果进程仍存在，就发送信号9，强行终止它
		if (iret == -1) {
			logfile.Write("进程pid=%d(%s)已经正常终止。\n", shm[i].pid, shm[i].pname);
		}
		else {
			kill(shm[i].pid, 9); // 如果进程还存在，就向进程发送9的强行杀死信号
			logfile.Write("进程pid=%d(%s)已经强制终止。\n", shm[i].pid, shm[i].pname);
		}
		//从共享内存中删除已超时进程的心跳记录
		memset(shm + i, 0, sizeof(struct st_procinfo)); //从共享内存中删除该记录
	}
	//把共享内存从当前进程中分离
	shmdt(shm);
	return 0;
}
