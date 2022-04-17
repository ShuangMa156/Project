/*
 * 程序名：demo33.cpp，此程序用于演示同步通信的效率和异步通信效率（IO复用技术）的客户端。
*/
#include "/weather/project/public/_public.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./demo33 ip port\nExample:./demo33 192.168.174.10 5005\n\n");
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
    int ibuflen = 0;
    CLogFile logfile; //实例化一个日志文件对象
    logfile.Open("/tmp/demo33.log", "a+");
    int j = 0; //记录收到的回应报文数量

    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    for (int i = 0; i < 10000; ++i) {
        SPRINTF(buffer, sizeof(buffer), "这是第%d个女生，编号为%03d.", i + 1, i + 1);
        if (TcpClient.Write(buffer) == false) { //向服务端发送请求报文
            break;
        }
        logfile.Write("发送：%s\n", buffer);
        //接受服务端的回应报文
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            if (TcpRead(TcpClient.m_connfd,buffer,&ibuflen,-1) == false) { //接受服务端的回应报文
                break;
            }
            logfile.Write("接受：%s\n", buffer);
            j++;
        }
        
    }
    while (j<10000) {
        memset(buffer, 0, sizeof(buffer));
        if (TcpRead(TcpClient.m_connfd,buffer,&ibuflen) == false) { //接受服务端的回应报文
            break;
        }
        logfile.Write("接受：%s\n", buffer);
        j++;
    }
}
/*
* bool TcpRead(const int sockfd, char* buffer, int* ibuflen, const int itimeout = 0); // 接收socket的对端发送过来的数据。
  sockfd：可用的socket连接。
  buffer：接收数据缓冲区的地址。
  ibuflen：本次成功接收数据的字节数。
  itimeout：接收等待超时的时间，单位：秒，-1-不等待；0-无限等待；>0-等待的秒数。
  返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
*/
