/*
 * 程序名：demo31.cpp，此程序用于演示同步通信的效率和异步通信效率（多进程）的客户端。
*/
#include "/weather/project/public/_public.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./demo31 ip port\nExample:./demo31 192.168.174.10 5005\n\n");
        return -1;
    }
    CTcpClient TcpClient; //创建TCP连接的客户端对象
    //向服务端发出连接请求
    if (TcpClient.ConnectToServer(argv[1], atoi(argv[2])) == false)
    {
        printf("TcpClient.ConnectToServer(%s,%d) failed.\n", argv[1], argv[2]);
        return -1;
    }
    char buffer[102400];
    CLogFile logfile; //实例化一个日志文件对象
    logfile.Open("/tmp/demo31.log", "a+");
    //用多进程实现异步通信
    int pid = fork(); //创建子进程

    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    for (int i = 0; i < 10000; ++i) {
        if (pid > 0) { //父进程发送报文
            SPRINTF(buffer, sizeof(buffer), "这是第%d个女生，编号为%03d.", i + 1, i + 1);
            if (TcpClient.Write(buffer) == false) { //向服务端发送请求报文
                break;
            }
            logfile.Write("发送：%s\n", buffer);
        }
        else { //子进程接收报文
            memset(buffer, 0, sizeof(buffer));
            if (TcpClient.Read(buffer) == false) { //接受服务端的回应报文
                break;
            }
            logfile.Write("接受：%s\n", buffer);
        }
    }
}
/*
* // socket通讯的客户端类
class CTcpClient
{
public:
    int  m_connfd;    // 客户端的socket.
    char m_ip[21];    // 服务端的ip地址。
    int  m_port;      // 与服务端通讯的端口。
    bool m_btimeout;  // 调用Read方法时，失败的原因是否是超时：true-超时，false-未超时。
    int  m_buflen;    // 调用Read方法后，接收到的报文的大小，单位：字节。

    CTcpClient();  // 构造函数。

    // 向服务端发起连接请求。
    // ip：服务端的ip地址。
    // port：服务端监听的端口。
    // 返回值：true-成功；false-失败。
    bool ConnectToServer(const char* ip, const int port);

    // 接收服务端发送过来的数据。
    // buffer：接收数据缓冲区的地址，数据的长度存放在m_buflen成员变量中。
    // itimeout：等待数据的超时时间，单位：秒，缺省值是0-无限等待。
    // 返回值：true-成功；false-失败，失败有两种情况：1）等待超时，成员变量m_btimeout的值被设置为true；2）socket连接已不可用。
    bool Read(char* buffer, const int itimeout = 0);

    // 向服务端发送数据。
    // buffer：待发送数据缓冲区的地址。
    // ibuflen：待发送数据的大小，单位：字节，缺省值为0，如果发送的是ascii字符串，ibuflen取0，如果是二进制流数据，ibuflen为二进制数据块的大小。
    // 返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
    bool Write(const char* buffer, const int ibuflen = 0);

    // 断开与服务端的连接
    void Close();

    ~CTcpClient();  // 析构函数自动关闭socket，释放资源。
};
*/
