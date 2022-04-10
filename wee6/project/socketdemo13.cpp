/*
 * 程序名：demo13.cpp，此程序用于演示网银App软件的客户端,增加了心跳机制。
*/
#include "/weather/project/public/_public.h"
CTcpClient TcpClient; //创建TCP连接的客户端对象
bool srv000(); //心跳
bool srv001(); //登录业务
bool srv002(); //我的账户（查询余额）
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./demo13 ip port\nExample:./demo13 127.0.0.1 5005\n\n");
        return -1;
    }

    //向服务端发出连接请求
    if (TcpClient.ConnectToServer(argv[1], atoi(argv[2])) == false)
    {
        printf("TcpClient.ConnectToServer(%s,%d) failed.\n", argv[1], argv[2]);
        return -1;
    }
    if (srv001() == false) { //登录业务
        printf("srv001() failed.\n");
        return -1;
    }
    sleep(3);
    if (srv002() == false) { //我的账户（查询余额）
        printf("srv002() failed.\n");
        return -1;
    }
    sleep(10);
    for (int i = 3; i < 5; ++i) {
        if(srv000()==false)
            break;
        sleep(i);
    }
    if (srv002() == false) { //我的账户（查询余额）
        printf("srv002() failed.\n");
        return -1;
    }
    return 0;
}
bool srv000() { //心跳，服务端对心跳报文的响应只会成功，客户端只要能够收到服务端的报文，就认为是成功的
    char buffer[1024];
    SPRINTF(buffer, sizeof(buffer), "<srvcode>0</srvcode>");
    printf("发送：%s\n", buffer);
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    if (TcpClient.Write(buffer) == false) { //向服务端发送请求报文
        return false;
    }
    memset(buffer, 0, sizeof(buffer));
    if (TcpClient.Read(buffer) == false) { //接受服务端的回应报文
        return false;
    }
    printf("接受：%s\n", buffer);
    return true;
}
bool srv001() { //登录业务
    char buffer[1024];
    SPRINTF(buffer, sizeof(buffer), "<srvcode>1</srvcode><tel>13739277495</tel><password>123456</password>");
    printf("发送：%s\n", buffer);
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    if (TcpClient.Write(buffer) == false) { //向服务端发送请求报文
        return false;
    }
    //printf("发送：%s\n", buffer);
    memset(buffer, 0, sizeof(buffer));
    if (TcpClient.Read(buffer) == false) { //接受服务端的回应报文
        return false;
    }
    printf("接受：%s\n", buffer);
    //解析服务器返回的xml
    int iretcode = -1;
    GetXMLBuffer(buffer, "retcode", &iretcode); //获取返回代码
    if (iretcode != 0) {
        printf("登录失败。\n");
        return false;
    }
    printf("登录成功。\n");
    return true;
}
bool srv002() { //我的账户（查询余额）
    char buffer[1024];
    SPRINTF(buffer, sizeof(buffer), "<srvcode>2</srvcode><cardid>62622022040918</cardid><password>123456</password>");
    printf("发送：%s\n", buffer);
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    if (TcpClient.Write(buffer) == false) { //向服务端发送请求报文
        return false;
    }
    //printf("发送：%s\n", buffer);
    memset(buffer, 0, sizeof(buffer));
    if (TcpClient.Read(buffer) == false) { //接受服务端的回应报文
        return false;
    }
    printf("接受：%s\n", buffer);
    //解析服务器返回的xml
    int iretcode = -1;
    GetXMLBuffer(buffer, "retcode", &iretcode); //获取返回代码
    if (iretcode != 0) {
        printf("余额查询失败。\n");
        return false;
    }
    double ye = 0;
    GetXMLBuffer(buffer, "ye", &ye);
    printf("余额查询成功(%.2f)。\n", ye);

    return true;
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
