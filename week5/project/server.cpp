#include "_public.h"
#define MAXNUMP_ 1000  //宏定义，最大的进程数量
#define SHMKEYP_ 0x5095 //宏定义，共享内存的key
#define SEMKEYP_ 0x5095 //宏定义，共享内存的key
//进程信息的结构体
struct st_pinfo
{
	int pid; //进程id
	char pname[51]; //进程名称，可以为空
	int timeout; //超时时间，单位：秒
	time_t atime; //最后一次心跳的时间，用整数表示
};
int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Using:./server proname\n");
		return 0;
	}
	//创建/获取共享内存,大小为n*sizeof(struct st_pinfo)，其中n为进程数量的上限
	int m_shmid = 0;
	if ((m_shmid = shmget(SHMKEYP_, MAXNUMP_ * sizeof(struct st_pinfo), 0640 | IPC_CREAT)) == -1) { //0640表示操作权限，IPC_CREAT表示如果共享内存存在，就获得它的ID，如果不存在就创建它
		printf("shmget(%x) failed.\n", SHMKEYP_);
		return -1;
	}

	CSEM m_sem; //用于给共享内存加锁的信号量
	if ((m_sem.init(SEMKEYP_)) == false) { //初始信号量为二值信号量
		printf("m_sem.init(%x) failed\n", SEMKEYP_);
		return -1;
	}
	//将共享内存连接到当前进程的地址空间
	struct st_pinfo* m_shm = 0;  //用结构体指针指向分配出的共享内存的首地址，可以当做结构体数组来用，也可以用地址的运算
	if ((m_shm = (struct st_pinfo*)shmat(m_shmid, 0, 0)) == (void*)-1) {
		printf("shmat failed\n");
		return -1;
	}
	printf("pid=%d,name=%s\n", m_shm->pid, m_shm->pname);
	//m_shm = (struct st_pinfo*)shmat(m_shmid, 0, 0);//结构体指针指向分配出来的共享内存的首地址
	//创建当前进程心跳信息结构体变量，把本进程的信息填进去
	struct st_pinfo stpinfo;
	memset(&stpinfo, 0, sizeof(struct st_pinfo));//共享内存分配后，系统会对它初始化，不会有垃圾产生
	//给共享内存加锁(锁持有的时间越短越好，被加锁的代码段越短越好)

	stpinfo.pid = getpid();
	STRNCPY(stpinfo.pname, sizeof(stpinfo.pname), argv[1], 50);//把当前进程信息放到结构体中
	printf("new pid=%d, new name=%s\n", stpinfo.pid, stpinfo.pname);
	/*char *STRNCPY(char* dest,const size_t destlen,const char* src,size_t n);//安全的strcat函数。
	  dest：目标字符串。
	  destlen：目标字符串dest占用内存的大小。
	  src：待追加的字符串。
	  返回值：目标字符串dest的地址。
	  注意，超出dest容量的内容将丢弃。
	*/
	stpinfo.timeout = 30; //设置进程超时时间
	stpinfo.atime = time(0); //最后一次心跳的时间，填当前时间
	int m_pos = -1;
	/*存在的问题：
	进程id是循环使用的，如果曾经历一个进程异常退出，没有清理自己的心跳信息，它的进程信息将残留在共享内存中，不巧的是，当前进程重用了上述进程的id
	这样就会在共享内存中存在两个进程id相同的记录，守护进程检查到残留进程的心跳时，会向进程id发送退出信号，这个信号将误杀当前进程。
	解决方法：
	*/
	//如果共享内存中存在当前进程的编号，一定是其他进程残留的数据，当前进程就重用该位置
	for (unsigned int j = 0; j < MAXNUMP_; ++j) {
		if (stpinfo.pid == m_shm[j].pid) {
			m_pos = j;
			break;
		}
	}
	m_sem.P();//加锁
	if (m_pos == -1) { //寻找空的位置
		for (unsigned int i = 0; i < MAXNUMP_; ++i) {
			if (m_shm[i].pid == 0) { //数组运算，地址运算(m_shm + i)->pid 
				//找到一个空位置
				m_pos = i;
				break;
			}
		}
	}
	if (m_pos == -1) { //没有空间时异常退出
		m_sem.V();//解锁
		printf("共享内容空间已用完。\n");
		return -1;
	}
	//把当前进程的心跳信息存入共享内存的进程组中
	memcpy(m_shm + m_pos, &stpinfo, sizeof(struct st_pinfo));
	m_sem.V();//写入之后，解锁
	while (true) {
		//更新共享内存中本进程的心跳时间
		m_shm[m_pos].atime = time(0); //将心跳时间设置为当前时间（无需加锁）
		sleep(10);
	}
	//把当前进程从共享内存中移去
	//方法一：将进程id设置为0
	//m_shm[m_pos].pid = 0;
	//方法二：将整个结构体初始化为0
	memset(m_shm + m_pos, 0, sizeof(struct st_pinfo)); //释放进程所占用的资源时无需加锁
	//把共享内存从当前进程中分离
	shmdt(m_shm);
	return 0;
}
