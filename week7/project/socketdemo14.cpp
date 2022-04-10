/*
 * 程序名：demo014.cpp，此程序用于演示网银App软件的服务器,增加了心跳机制。
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
bool bsession = false; //记录用户是否登录成功的变量
void FathEXIT(int sig); //父进程的退出函数
void ChldEXIT(int sig); //子进程的退出函数
bool srv000(const char* strrecvbuffer, char* strsendbuffer); //心跳
bool srv001(const char* strrecvbuffer, char* strsendbuffer); //登录
bool srv002(const char* strrecvbuffer, char* strsendbuffer); //查询余额
bool srv003(const char* strrecvbuffer, char* strsendbuffer); //转账
bool _main(const char* strrecvbuffer, char* strsendbuffer); //处理业务的主函数
int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        printf("Using:./demo14 port logfilename timeout\nExample:./demo14 5005 /tmp/demo14.log 35\n\n");
        return -1;
    }
    //signal(SIGCHLD, SIG_IGN); //忽略子进程的退出信号（避免子进程因得不到父进程的处理成为僵尸进程）
    CloseIOAndSignal(); //关闭全部的信号和输入、输出
    //父进程设置退出信号，在shell状态下可用“kill+进程号”正常终止这些进程,D但请不要用“killall -9 +进程号” 强行终止
    signal(SIGINT, FathEXIT);
    signal(SIGTERM, FathEXIT);
    if (logfile.Open(argv[2], "a+") == false) {
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
        logfile.Write("客户端（%s）已连接。\n", TcpServer.GetIP());
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
        char strrecvbuffer[1024]; //接受报文的缓冲区
        char strsendbuffer[1024]; //发送报文的缓冲区
        //与客户端通讯，接收客户端发过来的报文后，回复ok。
        while (1)
        {
            memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
            memset(strsendbuffer, 0, sizeof(strsendbuffer));
            if (TcpServer.Read(strrecvbuffer,atoi(argv[3])) == false) // 接收客户端的请求报文。
            {
                break;
            }
            logfile.Write("接收：%s\n", strrecvbuffer);
            //处理业务的主函数
            if (_main(strrecvbuffer, strsendbuffer) == false) {
                //ChldEXIT(0); //调用失败，子进程退出
                break;
            }
            if (TcpServer.Write(strsendbuffer) == false) // 向客户端发送响应结果。
            {
                break;
            }
            logfile.Write("发送：%s\n", strsendbuffer);
        }
        //return 0; //或者exit(0)，使子进程结束
        ChldEXIT(0); //调用子进程的退出函数使子进程退出
    }
}
void FathEXIT(int sig) { //父进程的退出函数
    //以下代码实为了防止信号处理函数在执行过程被信号中断
    signal(SIGINT, SIG_IGN); //忽略信号2
    signal(SIGTERM, SIG_IGN); //忽略信号15
    logfile.Write("父进程退出。sig=%d。\n", sig);
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
    logfile.Write("子进程退出。sig=%d。\n", sig);
    //与客户端断开连接
    TcpServer.CloseClient(); //关闭客户端的socket
    //自己退出
    exit(0);
}
bool _main(const char* strrecvbuffer, char* strsendbuffer) { //处理业务的主函数
    //解析strrecvbuffer,获取服务代码（业务代码）
    int isrvcode = -1;
    GetXMLBuffer(strrecvbuffer, "srvcode", &isrvcode);
    if ((isrvcode != 1) && (bsession == false)) {
        strcpy(strsendbuffer, "<retcode>-1</retcode><message>用户未登录</message>");
        return true;
    }
    //处理每种业务
    switch (isrvcode) {
    case 0: //心跳
        srv000(strrecvbuffer, strsendbuffer);
        break;
    case 1: //登录
        srv001(strrecvbuffer, strsendbuffer);
        break;
    case 2: //查询余额
        srv002(strrecvbuffer, strsendbuffer);
        break;
    case 3: //转账
        srv003(strrecvbuffer, strsendbuffer);
        break;

    default:
        logfile.Write("业务代码不合法：%s\n", strrecvbuffer);
        return false;
    }
    return true;
}
bool srv000(const char* strrecvbuffer, char* strsendbuffer) { //心跳
    strcpy(strsendbuffer, "<retcode>0</retcode><message>成功。</message>"); //服务端对心跳报文的响应只会成功
    return true;
}
bool srv001(const char* strrecvbuffer, char* strsendbuffer) { //登录
    //<srvcode>1</srvcode><tel>13739277495</tel><password>123456</password>
    //解析strrecvbuffer，获取业务代码
    char tel[21];
    char password[31];
    GetXMLBuffer(strrecvbuffer, "tel", tel, 20);
    GetXMLBuffer(strrecvbuffer, "password", password, 30);
    //处理业务
    if ((strcmp(tel, "13739277495") == 0) && (strcmp(password, "123456") == 0)) {
        strcpy(strsendbuffer, "<retcode>0</retcode><message>成功.</message>");//把处理结果放到strsendbuffer
        bsession = true;
    }
    else {
        strcpy(strsendbuffer, "<retcode>-1</retcode><message>失败.</message>");
    }
    return true;
}
bool srv002(const char* strrecvbuffer, char* strsendbuffer) { //我的账户（余额查询）
    //<srvcode>2</srvcode><cardid>62622022040918</cardid><password>123456</password>
    //解析strrecvbuffer，获取业务代码
    char cardid[31];
    GetXMLBuffer(strrecvbuffer, "cardid", cardid, 30);
    //处理业务
    if ((strcmp(cardid, "62622022040918") == 0)) {
        strcpy(strsendbuffer, "<retcode>0</retcode><message>成功.</message><ye>100.0</ye>");//把处理结果放到strsendbuffer
    }
    else {
        strcpy(strsendbuffer, "<retcode>-1</retcode><message>失败.</message>");
    }
    return true;
}
bool srv003(const char* strrecvbuffer, char* strsendbuffer) { //转账

    return true;
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
