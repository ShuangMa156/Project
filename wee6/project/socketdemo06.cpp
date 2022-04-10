/*
 * 程序名：demo06.cpp，此程序用于演示粘包的socket服务端（通过调用封装的函数解决粘包和分包问题）。
*/
#include "/weather/project/public/_public.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Using:./demo06 port\nExample:./demo06 5005\n\n"); return -1;
    }

    // 第1步：创建服务端的socket。
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { perror("socket"); return -1; }

    // 第2步：把服务端用于通讯的地址和端口绑定到socket上。
    struct sockaddr_in servaddr;    // 服务端地址信息的数据结构。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET。
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);          // 任意ip地址。
    servaddr.sin_port = htons(atoi(argv[1]));  // 指定通讯端口。
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("bind"); close(listenfd); return -1;
    }

    // 第3步：把socket设置为监听模式。
    if (listen(listenfd, 5) != 0) { perror("listen"); close(listenfd); return -1; }

    // 第4步：接受客户端的连接。
    int  clientfd;                  // 客户端的socket。
    int  socklen = sizeof(struct sockaddr_in); // struct sockaddr_in的大小
    struct sockaddr_in clientaddr;  // 客户端的地址信息。
    clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, (socklen_t*)&socklen);
    printf("客户端（%s）已连接。\n", inet_ntoa(clientaddr.sin_addr));

    int iret;
    char buffer[1024];

    // 第5步：与客户端通讯，接收客户端发过来的报文。
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int ibuflen = 0; //初始化本次成功接收的字节数
        if (TcpRead(clientfd, buffer, &ibuflen)==false) // 接收客户端的请求报文。
        {
           break;
        }

        printf("接收：%s\n", buffer);
    }

    // 第6步：关闭socket，释放资源。
    close(listenfd); close(clientfd);
}
/*
* bool TcpRead(const int sockfd, char* buffer, int* ibuflen, const int itimeout = 0);// 接收socket的对端发送过来的数据。
*  sockfd：可用的socket连接。
   buffer：接收数据缓冲区的地址。
   ibuflen：本次成功接收数据的字节数。
   itimeout：接收等待超时的时间，单位：秒，-1-不等待；0-无限等待；>0-等待的秒数。
   返回值：true-成功；false-失败，失败有两种情况：1）等待超时；2）socket连接已不可用。
* bool TcpRead(const int sockfd, char* buffer, int* ibuflen, const int itimeout)
 {
    if (sockfd == -1) return false;

    // 如果itimeout>0，表示需要等待itimeout秒，如果itimeout秒后还没有数据到达，返回false。
    if (itimeout > 0)
    {
        struct pollfd fds;
        fds.fd = sockfd;
        fds.events = POLLIN;
        if (poll(&fds, 1, itimeout * 1000) <= 0) return false;
    }

    // 如果itimeout==-1，表示不等待，立即判断socket的缓冲区中是否有数据，如果没有，返回false。
    if (itimeout == -1)
    {
        struct pollfd fds;
        fds.fd = sockfd;
        fds.events = POLLIN;
        if (poll(&fds, 1, 0) <= 0) return false;
    }

    (*ibuflen) = 0;  // 报文长度变量初始化为0。

    // 先读取报文长度，4个字节(固定)。
    if (Readn(sockfd, (char*)ibuflen, 4) == false) return false;

    (*ibuflen) = ntohl(*ibuflen);  // 把报文长度由网络字节序转换为主机字节序。

    // 再读取报文内容。
    if (Readn(sockfd, buffer, (*ibuflen)) == false) return false;

    return true;
 }

  sockfd：已经准备好的socket连接。
  buffer：接收数据缓冲区的地址。
  n：本次接收数据的字节数。
  返回值：成功接收到n字节的数据后返回true，socket连接不可用返回false。
bool Readn(const int sockfd, char* buffer, const size_t n)// 从已经准备好的socket中读取数据。

{
    int nLeft = n;  // 剩余需要读取的字节数。
    int idx = 0;    // 已成功读取的字节数。
    int nread;    // 每次调用recv()函数读到的字节数。

    while (nLeft > 0)
    {
        if ((nread = recv(sockfd, buffer + idx, nLeft, 0)) <= 0) return false;

        idx = idx + nread;
        nLeft = nLeft - nread;
    }

    return true;
}
*/



