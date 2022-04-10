/*
 * 程序名：demo08.cpp，此程序用于演示socket通讯的服务端(通过调用框架中的CTcpServer类实现)。
*/
#include "/weather/project/public/_public.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Using:./demo08 port\nExample:./demo08 5005\n\n");
        return -1;
    }
    CTcpServer TcpServer;
    //服务端初始化
    if (TcpServer.InitServer(atoi(argv[1]))== false) { 
        printf("TcpServer.InitServer(%s) failed.\n",argv[1]);
        return -1;
    }
    //等待客户端的连接
    if(TcpServer.Accept() == false) {
        printf("TcpServer.Accept() failed.\n");
        return -1;
     }
    printf("客户端（%s）已连接。\n",TcpServer.GetIP());
    char buffer[102400];

    //与客户端通讯，接收客户端发过来的报文后，回复ok。
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        if (TcpServer.Read(buffer) ==false) // 接收客户端的请求报文。
        { 
            break;
        }
        printf("接收：%s\n", buffer);

        strcpy(buffer, "ok");
        if (TcpServer.Write(buffer)==false) // 向客户端发送响应结果。
        {
            break;
        }
        printf("发送：%s\n", buffer);
    }
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
