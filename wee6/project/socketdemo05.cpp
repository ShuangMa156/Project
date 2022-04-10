/*
 * 程序名：demo05.cpp，此程序用于演示粘包的socket客户端（通过调用封装的函数解决粘包和分包问题）。
*/
#include "/weather/project/public/_public.h"
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./demo05 ip port\nExample:./demo05 127.0.0.1 5005\n\n"); return -1;
    }

    // 第1步：创建客户端的socket。
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { perror("socket"); return -1; }

    // 第2步：向服务器发起连接请求。
    struct hostent* h;
    if ((h = gethostbyname(argv[1])) == 0)   // 指定服务端的ip地址。
    {
        printf("gethostbyname failed.\n"); close(sockfd); return -1;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2])); // 指定服务端的通讯端口。
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)  // 向服务端发起连接清求。
    {
        perror("connect"); close(sockfd); return -1;
    }

    int iret;
    char buffer[1024];

    // 第3步：与服务端通讯，连续发送1000个报文。
    for (int ii = 0; ii < 1000; ii++)
    {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "这是第%d个超级女生，编号%03d。", ii + 1, ii + 1);

        if (TcpWrite(sockfd, buffer, strlen(buffer)) == false) // 向服务端发送请求报文。
        {
            break;
        }

        printf("发送：%s\n", buffer);
    }

    // 第4步：关闭socket，释放资源。
    close(sockfd);
}
/*
* bool TcpWrite(const int sockfd, const char* buffer, const int ibuflen = 0); // 向socket的对端发送数据。
  sockfd：可用的socket连接。
  buffer：待发送数据缓冲区的地址。
  ibuflen：待发送数据的字节数，如果发送的是ascii字符串，ibuflen填0或字符串的长度，如果是二进制流数据，ibuflen为二进制数据块的大小。
  返回值：true-成功；false-失败，如果失败，表示socket连接已不可用。
  bool TcpWrite(const int sockfd, const char* buffer, const int ibuflen)
  {
    if (sockfd == -1) return false;

    int ilen = 0;  // 报文长度。

    // 如果ibuflen==0，就认为需要发送的是字符串，报文长度为字符串的长度。
    if (ibuflen == 0) ilen = strlen(buffer);
    else ilen = ibuflen;

    int ilenn = htonl(ilen);    // 把报文长度转换为网络字节序。

    char TBuffer[ilen + 4];     // 发送缓冲区。
    memset(TBuffer, 0, sizeof(TBuffer));  // 清区发送缓冲区。
    memcpy(TBuffer, &ilenn, 4);           // 把报文长度拷贝到缓冲区。
    memcpy(TBuffer + 4, buffer, ilen);      // 把报文内容拷贝到缓冲区。

    // 发送缓冲区中的数据。
    if (Writen(sockfd, TBuffer, ilen + 4) == false) return false;

    return true;
  }

   sockfd：已经准备好的socket连接。
   buffer：待发送数据缓冲区的地址。
   n：待发送数据的字节数。
   返回值：成功发送完n字节的数据后返回true，socket连接不可用返回false。
bool Writen(const int sockfd, const char* buffer, const size_t n)  // 向已经准备好的socket中写入数据。
{
    int nLeft = n;  // 剩余需要写入的字节数。
    int idx = 0;    // 已成功写入的字节数。
    int nwritten; // 每次调用send()函数写入的字节数。

    while (nLeft > 0)
    {
        if ((nwritten = send(sockfd, buffer + idx, nLeft, 0)) <= 0) return false;

        nLeft = nLeft - nwritten;
        idx = idx + nwritten;
    }

    return true;
}

*/
