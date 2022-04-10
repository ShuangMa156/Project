/*
 * 程序名：demo010.cpp，此程序用于演示采用开发框架的CTcpServer类实现socket通信的多进程服务器。
 *  (1)在多进程的服务程序中，如果杀掉一个子进程，则和这个子进程通信的客户端会断开，但是不会影响到其他的子进程与对应的客户端的通信，也不会影响父进程。
 * （2）如果杀掉父进程，不会影响正在通信中的子进程，但是新的客户端无法尽量连接
 * （3）如果用killall+程序名，可以杀掉父进程和全部的子进程
 * 
 * 多进程网络服务端程序退出的三种情况：
 * （1）如果是子进程收到退出信号，该子进程断开与客户端连接的socket，然后退出---->断开某一客户端的连接
 * （2）如果是父进程收到退出信号，父进程先关闭监听的socket，然后向全部的子进程发出退出信号---->停止服务器。断开全部客户端的连接
 * （3）如果父子进程都收到退出信号，本质上与第二种情况相同---->停止服务器。断开全部客户端的连接
*/
#include "/weather/project/public/_public.h"
CLogFile logfile; //服务程序的运行日志
CTcpServer TcpServer; //创建服务端对象
void FathEXIT(int sig); //父进程的退出函数
void ChldEXIT(int sig); //子进程的退出函数
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./demo10 port logfilename\nExample:./demo10 5005 /tmp/demo10.log\n\n");
        return -1;
    }
    //signal(SIGCHLD, SIG_IGN); //忽略子进程的退出信号（避免子进程因得不到父进程的处理成为僵尸进程）
    CloseIOAndSignal(); //关闭全部的信号和输入、输出
    //父进程设置退出信号，在shell状态下可用“kill+进程号”正常终止这些进程,D但请不要用“killall -9 +进程号” 强行终止
    signal(SIGINT, FathEXIT);
    signal(SIGTERM, FathEXIT);
    if (logfile.Open(argv[2],"a+") == false) {
        printf("logfile.Open(%s) failed.\n", argv[2]);
        return -1;
    }
    //服务端初始化
    if (TcpServer.InitServer(atoi(argv[1])) == false) {
        logfile.Write("TcpServer.InitServer(%s) failed.\n", argv[1]);
        return -1;
    }
    while (true) { //不断的等待客户端的请求
        //等待客户端的连接
        if (TcpServer.Accept() == false) {
            logfile.Write("TcpServer.Accept() failed.\n");
            //return -1;
            FathEXIT(-1); //用退出函数
        }
        logfile.Write( "客户端（%s）已连接。\n", TcpServer.GetIP());
        //printf("listenfd=%d,connectfd=%d\n", TcpServer.m_listenfd, TcpServer.m_connfd); //父进程（只负责监听客户端）
        if (fork() > 0) {
            TcpServer.CloseClient(); //关闭父进程与客户端的连接
            continue; //父进程继续回到Accept()
        }
        //子进程重新设置退出信号
        signal(SIGINT, ChldEXIT);
        signal(SIGTERM, ChldEXIT);
       // printf("listenfd=%d,connectfd=%d\n", TcpServer.m_listenfd, TcpServer.m_connfd); //子进程（只负责与客户端通信）
        TcpServer.CloseListen(); //关闭子进程对客户端的监听
        //sleep(1000); //方便查看子进程和父进程的文件描述符
        //子进程与客户端进行通信，处理业务
        char buffer[102400];

        //与客户端通讯，接收客户端发过来的报文后，回复ok。
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));
            if (TcpServer.Read(buffer) == false) // 接收客户端的请求报文。
            {
                break;
            }
            logfile.Write("接收：%s\n", buffer);

            strcpy(buffer, "ok");
            if (TcpServer.Write(buffer) == false) // 向客户端发送响应结果。
            {
                break;
            }
            logfile.Write("发送：%s\n", buffer);
        }
        //return 0; //或者exit(0)，使子进程结束
        ChldEXIT(0); //调用子进程的退出函数使子进程退出
    }
}
void FathEXIT(int sig) { //父进程的退出函数
    //以下代码实为了防止信号处理函数在执行过程被信号中断
    signal(SIGINT, SIG_IGN); //忽略信号2
    signal(SIGTERM, SIG_IGN); //忽略信号15
    logfile.Write("父进程退出。sig=%d。\n",sig);
    //关闭监听
    TcpServer.CloseListen(); //关闭监听的socket
    //给子进程发送退出信号
    kill(0, 15); //通知全部的子进程退出
    //自己退出
    exit(0);
}
void ChldEXIT(int sig) {//子进程的退出函数
    //以下代码实为了防止信号处理函数在执行过程被信号中断
    signal(SIGINT, SIG_IGN); //忽略信号2
    signal(SIGTERM, SIG_IGN); //忽略信号15
    logfile.Write("子进程退出。sig=%d。\n",sig);
    //与客户端断开连接
    TcpServer.CloseClient(); //关闭客户端的socket
    //自己退出
    exit(0);
}
/*
* // socket通讯的服务端类
class CTcpServer
{
private:
    int m_socklen;                    // 结构体struct sockaddr_in的大小。
    struct sockaddr_in m_clientaddr;  // 客户端的地址信息。
    struct sockaddr_in m_servaddr;    // 服务端的地址信息。
public:
    int  m_listenfd;   // 服务端用于监听的socket。
    int  m_connfd;     // 客户端连接上来的socket。
    bool m_btimeout;   // 调用Read方法时，失败的原因是否是超时：true-超时，false-未超时。
    int  m_buflen;     // 调用Read方法后，接收到的报文的大小，单位：字节。

    CTcpServer();  // 构造函数。

    // 服务端初始化。
    // port：指定服务端用于监听的端口。
    // 返回值：true-成功；false-失败，一般情况下，只要port设置正确，没有被占用，初始化都会成功。
    bool InitServer(const unsigned int port, const int backlog = 5);

    // 阻塞等待客户端的连接请求。
    // 返回值：true-有新的客户端已连接上来，false-失败，Accept被中断，如果Accept失败，可以重新Accept。
    bool Accept();

    // 获取客户端的ip地址。
    // 返回值：客户端的ip地址，如"192.168.1.100"。
    char* GetIP();

    // 接收客户端发送过来的数据。
    // buffer：接收数据缓冲区的地址，数据的长度存放在m_buflen成员变量中。
    // itimeout：等待数据的超时时间，单位：秒，缺省值是0-无限等待。
    // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时，成员变量m_btimeout的值被设置为true；2）socket连接已不可用。
    bool Read(char* buffer, const int itimeout = 0);

    // 向客户端发送数据。
    // buffer：待发送数据缓冲区的地址。
    // ibuflen：待发送数据的大小，单位：字节，缺省值为0，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
    // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
    bool Write(const char* buffer, const int ibuflen = 0);

    // 关闭监听的socket，即m_listenfd，常用于多进程服务程序的子进程代码中。
    void CloseListen();

    // 关闭客户端的socket，即m_connfd，常用于多进程服务程序的父进程代码中。
    void CloseClient();

    ~CTcpServer();  // 析构函数自动关闭socket，释放资源。
};
*/
