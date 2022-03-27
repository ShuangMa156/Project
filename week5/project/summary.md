如何开发永不停机的服务程序
二、服务程序的调度-----调度程序用于启动服务程序
1、学习Linux信号的基础知识和使用方法
2、学习Linux多进程的基础知识和使用方法
（1）Linux的三个特别进程：0、1、2号进程
0：idle进程--->系统创建的第一个进程，加载系统
1：systemd进程--->负责系统的初始化，是所有其他用户进程的祖先
3：kthread进程---->负责所有内核线程的调度和管理
（2）Linux的每个进程都有一个非负整数表示的唯一进程ID
（3）查看进程的详细信息：ps -ef|grep 进程名
   查看全部的进程信息： ps -ef 
（4）获取进程ID:getpid(void)
     获取父进程ID:getppid(void)
（5）如何在程序中创建进程
C语言：调用fork()函数
①一个现有的进程（父进程）调用函数fork()创建一个新的进程（子进程）
②子进程和父进程继续执行fork()函数后面的代码
③fork()函数调用一次，返回两次
④fork()函数调用成功：子进程返回0，父进程返回子进程的进程ID（可根据返回值，让父进程和子进程执行不同的任务）
fork()函数调用失败：返回-1
⑤子进程是父进程的副本
⑥子进程获得了父进程的存储空间、堆和栈的副本，不是共享（子进程做了改变，不会对父进程有影响）
⑦父进程中打开的文件描述符也被复制到子进程中（文件缓冲区，子进程和父进程相互独立，在某一进程中关闭文件不会影响另一个进程）
⑧如果父进程先退出，子进程会成为孤儿进程（没有危害），将被1号进程收养（成为1号进程的子进程），由1号进程对它们完成状态收集工作
⑨如果子进程先退出，内核向父进程发送SIGCHLD信号，如果父进程不处理这个信号，子进程会成为僵尸进程（僵尸进程在父进程退出时跟着父进程一起消亡；在父进程未退出之前，僵尸进程一直保留）
如果子进程在父进程之前终止，内核为每个子进程保留一个数据结构，包括进程编号、终止状态和使用CPU时间等。父进程如果处理了子进程的退出信息，内核就会释放这个数据结构，如果父进程没有处理子进程的退出信息，内核就不会释放这个数据结构，子进程的进程编号一直就会一直被占用。但是系统可用的进程号是有限的，如果大量的产生僵尸进程，将因为没有可用的进程号而导致系统不能产生新的进程，这就是僵尸进程的危害。

解决僵尸进程的三种方法：
①方法一：在父进程中忽略子进程退出后内核发给父进程的SIGCHLD信号
②方法二：在父进程中增加等待子进程退出的代码----->会阻塞父进程的继续执行
③方法三：设置SIGCHLD的信号处理函数

3、开发服务程序调度模块
服务程序的调度需求：
①周期性的启动后台服务程序
②常驻内存中的服务程序，启动后必须一直在内存中，如果异常退出，必须在短时间内重启
解决的问题：
（1）如何在C程序中调用其他程序
C语言提供了exec系列函数：
execl, execlp, execle, execv, execvp, execvpe - execute a file
三、守护进程的实现----守护程序检查服务程序是否死机，如果死机，就终止服务程序
1、Linux共享内存--->允许多个进程访问同一块内存
（1）调用shmget函数获取或创建共享内存
如果共享内存存在：获取共享内存的地址
如果共享内存不存在：就创建共享内存
（2）调用shmat函数把共享内存连接到当前进程的地址空间
当前地址可以读写共享内存的数据
（3）调用shmdt函数把共享内存从当前进程分离
（4）调用shmctl函数删除共享内存（当前进程不在需要共享内存）
Linux中：
查看共享内存：ipcs -m
删除共享内存：ipcrm -m 共享内存的编号

共享内存存在的问题：没有提供锁机制，容易造成数据的不一致性
2、用信号量给共享内存加锁
（1）信号量：
①本质上是一个非负数（>=0）的计数器
②用于给共享内存建立一个标志，表示该共享资源被占用的情况
③进程对信号量的操作----->标识共享资源的使用情况：P操作（申请资源 -1），V操作（释放资源 +1）
（2）二值信号量：信号量的一种特殊形式，表示资源只有可用和不可用两种状态：0-不可用，1-可用
功能：与互斥锁相同，只有开合关两种状态
开服框架中处理信号量类：
class CSEM
{
private:
  union semun  // 用于信号量操作的共同体。
  {
    int val;
    struct semid_ds *buf;
    unsigned short  *arry;
  };

  int   m_semid;         // 信号量描述符。

  // 如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，
  // 在全部修改过信号量的进程（正常或异常）终止后，操作系统将把信号量恢
  // 复为初始值（就像撤消了全部进程对信号的操作）。
  // 如果信号量用于表示可用资源的数量（不变的），设置为SEM_UNDO更合适。
  // 如果信号量用于生产消费者模型，设置为0更合适。
  // 注意，网上查到的关于sem_flg的用法基本上是错的，一定要自己动手多测试。
  short m_sem_flg;
public:
  CSEM();
  // 如果信号量已存在，获取信号量；如果信号量不存在，则创建它并初始化为value。
  bool init(key_t key,unsigned short value=1,short sem_flg=SEM_UNDO); 
  bool P(short sem_op=-1); // 信号量的P操作。
  bool V(short sem_op=1);  // 信号量的V操作。
  int  value();            // 获取信号量的值，成功返回信号量的值，失败返回-1。
  bool destroy();          // 销毁信号量。
 ~CSEM();
};
3、守护程序如何判断服务程序是否死机：心跳机制

创建一块共享内存用于存放服务程序的心跳信息的结构体。每个服务程序启动的时候会查找共享内存，在数组中找一个空白的位置把自己的心跳信息写进去，运行过程中还会不断的把自己的心跳信息更新到数组中。死掉的服务程序被终止后，调度程序将重新启动它。
// 进程心跳信息的结构体。
struct st_procinfo
{
  int    pid;         // 进程id。
  char   pname[51];   // 进程名称，可以为空。
  int    timeout;     // 超时时间，单位：秒。
  time_t atime;       // 最后一次心跳的时间，用整数表示。
};

#define MAXNUMP     1000    // 最大的进程数量。
#define SHMKEYP   0x5095    // 共享内存的key。
#define SEMKEYP   0x5095    // 信号量的key。

// 查看共享内存：  ipcs -m
// 删除共享内存：  ipcrm -m shmid
// 查看信号量：    ipcs -s
// 删除信号量：    ipcrm sem semid

// 进程心跳操作类。
class CPActive
{
private:
  CSEM m_sem;                 // 用于给共享内存加锁的信号量id。
  int  m_shmid;               // 共享内存的id。
  int  m_pos;                 // 当前进程在共享内存进程组中的位置。
  st_procinfo *m_shm;         // 指向共享内存的地址空间。

public:
  CPActive();  // 初始化成员变量。

  // 把当前进程的心跳信息加入共享内存进程组中。
  bool AddPInfo(const int timeout,const char *pname=0,CLogFile *logfile=0);

  // 更新共享内存进程组中当前进程的心跳时间。
  bool UptATime();

  ~CPActive();  // 从共享内存中删除当前进程的心跳记录。
};

任务：
（1）实现服务程序在共享内存中维护自己的心跳信息
（2）开发守护程序，终止已经死机的服务程序
四、两个常用的小工具
1、开发数据压缩模块

2、开发清理历史数据文件模块
五、服务程序的运行策略
1、用脚本文件代替命令
（1）启动服务程序的脚本 start.sh
（2）停止服务程序的脚本 killall.sh
2、在操作系统启动时启动全部的服务程序
Linux中系统启动的脚本文件 /etc/rc.local
源文件：
#!/bin/bash
# THIS FILE IS ADDED FOR COMPATIBILITY PURPOSES
#
# It is highly advisable to create own systemd services or udev rules
# to run scripts during boot instead of using this file.
#
# In contrast to previous versions due to parallel execution during boot
# this script will NOT be run after all other services.
#
# Please note that you must run 'chmod +x /etc/rc.d/rc.local' to ensure
# that this script will be executed during boot.

touch /var/lock/subsys/local
修改后的文件：
#!/bin/bash
# THIS FILE IS ADDED FOR COMPATIBILITY PURPOSES
#
# It is highly advisable to create own systemd services or udev rules
# to run scripts during boot instead of using this file.
#
# In contrast to previous versions due to parallel execution during boot
# this script will NOT be run after all other services.
#
# Please note that you must run 'chmod +x /etc/rc.d/rc.local' to ensure
# that this script will be executed during boot.

touch /var/lock/subsys/local(超级用户)
#检查服务程序是否超时(启动守护程序)
#/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/checkproc

#启动数据中心的后台服务程序(超级用户)
#(超级用户)/bin/sh /weather/project/idc1/c/start.sh
#切换到有权限的普通用户执行 su - root -c “/bin/sh /weather/project/idc1/c/start.sh”
