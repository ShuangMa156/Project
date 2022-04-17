/*
 * 程序名：tcpgetfiles.cpp，采用tcp协议，实现文件发送的客户端。
*/
#include "_public.h"
struct st_arg {
    int clienttype; // 客户端类型：1-上传文件  2-下载文件
    char ip[31]; //服务端的IP地址
    int port; //服务端的端口
    int ptype; //文件下载成功后服务端文件的处理方式 1-删除文件 2-移动到备份目录
    char srvpath[301]; //本地文件存放的根目录
    char srvpathbak[301]; //文件下载成功后，服务端文件备份的根目录，当ptype=2时有效
    bool andchild; //是否下载srvpath目录下各级子目录的文件，true-是，false-否
    char matchname[301]; //待下载文件名的匹配方式,如"*.TXT,*.XML"（注意用大写）
    char clientpath[301]; //本地文件存放的根目录
    int timetvl; //扫描服务端目录文件的时间间隔，单位：秒
    int timeout; //进程心跳的超时时间
    char pname[51]; //进程名，建议用"tcpgetfiles_"后缀的方式
} starg;
CTcpClient TcpClient; //创建TCP连接的客户端对象
CLogFile logfile; //创建日志对象
CPActive PActive; //进程心跳对象
char strrecvbuffer[1024]; //接受报文的缓冲区
char strsendbuffer[1024]; //发送报文的缓冲区
void EXIT(int sig); //程序退出和信号2、15的处理函数
void _help(); //帮助文档
bool _xmltoarg(char* strxmlbuffer); //把xml解析到参数starg结构中
bool Login(const char* argv); //登录业务
void _tcpgetfiles(); //文件下载的主函数，执行一次文件下载的任务
bool RecvFile(const int sockfd, const char* filename, const char* mtime, int filesize); //接收文件的内容
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        _help(); //给出帮助文档
        return -1;
    }
    CloseIOAndSignal(); //关闭全部的信号和输入、输出
    signal(SIGINT, EXIT); //设置信号2，在shell状态下可用"kill+进程号"正常终止进程，但不建议用"killall -9 +进程号"强行终止
    signal(SIGTERM, EXIT); //设置信号15
    //打开日志文件
    if (logfile.Open(argv[1], "a+") == false) {
        printf("打开日志文件失败(%s)。\n", argv[1]);
        return -1;
    }
    //解析xml,得到程序运行的参数
    if (_xmltoarg(argv[2]) == false) {
        return -1;
    }
    PActive.AddPInfo(starg.timeout, starg.pname); // 把进程的心跳信息写入共享内存
    //向服务端发出连接请求
    if (TcpClient.ConnectToServer(starg.ip, starg.port) == false)
    {
        logfile.Write("TcpClient.ConnectToServer(%s,%d) failed.\n", starg.ip, starg.port);
        EXIT(-1);
    }
    if (Login(argv[2]) == false) { //登录业务
        logfile.Write("Login() failed.\n");
        EXIT(-1);
    }
    _tcpgetfiles();//调用文件下载的主函数，执行一次文件下载的任务

    EXIT(0);
}
void _help()
{
    printf("\n");
    printf("Using:/weather/project/tools1/bin/tcpgetfiles logfilename xmlbuffer\n\n");
    printf("Example:/weather/project/tools1/bin/procctl 20 /weather/project/tools1/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log \"<ip>192.168.174.10</ip><port>5005</port><ptype>1</ptype><srvpath>/tmp/tcp/surfdata2</srvpath><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><clientpath>/tmp/tcp/surfdata3</clientpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>\"\n\n\n");
    printf("        /weather/project/tools1/bin/procctl 20 /weather/project/tools1/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log \"<ip>192.168.174.10</ip><port>5005</port><ptype>2</ptype><srvpath>/tmp/tcp/surfdata2</srvpath><srvpathbak>/tmp/tcp/surfdata2bak</srvpathbak><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><clientpath>/tmp/tcp/surfdata3</clientpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>\"\n\n\n");
    printf("本程序是数据中心的公共功能模块，采用TCP协议把文件上传给服务端。\n");
    printf("logfilename  是本程序运行的日志文件。\n");
    printf("xmlbuffer    本程序的运行参数，如下：\n");
    printf("ip    服务端的Ip地址\n");
    printf("port  服务端的端口号\n");
    printf("ptype 文件下载成功后，本地文件的处理方法：1删除；2-备份；如果为2，还要指定备份的目录。\n");
    printf("srvpath  服务端文件存放的根目录。\n");
    printf("srvpathbak  本地文件下载成功后，服务端文件备份的根目录。\n");
    printf("andchild  是否下载srvpath目录下各级子目录的文件，true-是，false-否，缺省为false.\n");
    printf("matchname 待下载文件的匹配规则。如\"*.TXT,*.XML\n");
    printf("clientpath 客户端文件存放的根目录\n");
    printf("timetvl   扫描服务端目录文件的时间间隔，单位：秒，取值在1~30之间。\n");
    printf("timeout   下载文件超时时间，单位:秒,视文件大小、网络带宽而定，建议设置在50以上。\n");
    printf("pname     进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n\n");
}
bool _xmltoarg(char* strxmlbuffer) {//把xml解析到参数starg结构中
    memset(&starg, 0, sizeof(struct st_arg));
    GetXMLBuffer(strxmlbuffer, "ip", starg.ip); //解析IP地址
    if (strlen(starg.ip) == 0) {
        logfile.Write("ip is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "port", &starg.port); //解析端口号
    if (starg.port == 0) {
        logfile.Write("port is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype); //解析文件下载后的处理方式
    if ((starg.ptype != 1) && (starg.ptype != 2)) {
        logfile.Write("ptype not in (1,2).\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath); //解析下载文件的根目录
    if (strlen(starg.srvpath) == 0) {
        logfile.Write("srvpath is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "srvpathbak", starg.srvpathbak);//解析本地文件下载后的备份目录
    if ((starg.ptype == 2) && (strlen(starg.srvpathbak) == 0)) {
        logfile.Write("srvpathbak is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild); //解析是否下载子目录
    GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname); //解析下载文件的匹配规则
    if (strlen(starg.matchname) == 0) {
        logfile.Write("matchname is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath); //解析服务端文件存放的根目录
    if (strlen(starg.clientpath) == 0) {
        logfile.Write("clientpath is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl); //解析文件下载周期
    if (starg.timetvl == 0) {
        logfile.Write("timetvl is null.\n");
        return false;
    }
    if (starg.timetvl > 30) {
        starg.timetvl = 30; //扫描本地文件目录的时间间隔没有必要超过30秒
    }
    GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout); //解析下载文件的超时间
    if (starg.timeout == 0) {
        logfile.Write("timeout is null.\n");
        return false;
    }
    if (starg.timeout < 50) {
        starg.timeout = 50; //进程心跳的超时时间没有必要小于50秒
    }
    GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50); //解析进程名称
    if (strlen(starg.pname) == 0) {
        logfile.Write("pname is null.\n");
        return false;
    }
    return true;
}
bool Login(const char* argv) { //登录业务
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //接收缓冲区初始化
    memset(strsendbuffer, 0, sizeof(strsendbuffer)); //发送缓冲区初始化
    SPRINTF(strsendbuffer, sizeof(strsendbuffer), "%s<clienttype>2</clienttype>", argv);
    logfile.Write("发送：%s\n", strsendbuffer);
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    if (TcpClient.Write(strsendbuffer) == false) { //向服务端发送请求报文
        return false;
    }
    if (TcpClient.Read(strrecvbuffer, 20) == false) { //接受服务端的回应报文
        return false;
    }
    logfile.Write("接收：%s\n", strrecvbuffer);
    logfile.Write("登录(%s:%d)成功。\n", starg.ip, starg.port);
    return true;
}
void EXIT(int sig) {
    logfile.Write("程序退出，sig=%d\n\n", sig);
    exit(0);
}
void _tcpgetfiles() {//文件下载的主函数，执行一次文件下载的任务
    PActive.AddPInfo(starg.timeout, starg.pname);
    while (true) {
        memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //初始化接收缓冲区
        memset(strsendbuffer, 0, sizeof(strsendbuffer)); //初始化发送缓冲区
        PActive.UptATime();
        //接收服务端的报文
        if (TcpClient.Read(strrecvbuffer, starg.timetvl + 10) == false) {
            logfile.Write("TcpClient.Read() failed.\n");
            return;
        }
        //logfile.Write("strrecvbuffer=%s\n", strrecvbuffer);
        //处理心跳报文
        if (strcmp(strrecvbuffer, "<activetest>ok</activetest>") == 0) {
            strcpy(strsendbuffer, "ok");
            //logfile.Write("strsendbuffer=%s\n", strsendbuffer);
            if (TcpClient.Write(strsendbuffer) == false) {
                logfile.Write("TcpClient.Write() failed.\n");
                return;
            }
        }
        //处理文件下载的请求报文
        if (strncmp(strrecvbuffer, "<filename>", 10) == 0) {
            //解析上传文件请求报文的xml
            char serverfilename[301]; //文件名
            memset(serverfilename, 0, sizeof(serverfilename));
            char mtime[21]; //文件修改时间
            memset(mtime, 0, sizeof(mtime));
            int filesize = 0; //文件大小
            GetXMLBuffer(strrecvbuffer, "filename", serverfilename, 300);
            GetXMLBuffer(strrecvbuffer, "mtime", mtime, 19);
            GetXMLBuffer(strrecvbuffer, "size", &filesize);
            //由于客户端可服务端的文件名是不同的，所以需要生成客户端的文件名，将文件中的srvpath替换为clientpath
            char clientfilename[301];
            memset(clientfilename, 0, sizeof(clientfilename));
            strcpy(clientfilename, serverfilename);
            UpdateStr(clientfilename, starg.srvpath, starg.clientpath, false);
            //接收上传文件的内容
            logfile.Write("recv %s(%d)...", clientfilename, filesize);
            if (RecvFile(TcpClient.m_connfd, clientfilename, mtime, filesize) == true) {
                logfile.WriteEx("ok.\n");
                SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>ok</result>", serverfilename);
            }
            else {
                logfile.WriteEx("failed.\n");
                SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>failed</result>", serverfilename);
            }
            //把接受结果返回客户端
            //logfile.Write("strsendbuffer=%s\n", strsendbuffer);
            if (TcpClient.Write(strsendbuffer) == false) {
                logfile.Write("TcpClient.Write() failed.\n");
                return;
            }
        }
    }
}
bool RecvFile(const int sockfd, const char* filename, const char* mtime, int filesize) { //接收文件的内容
    //生成临时文件名
    char strfilenametmp[301];
    SNPRINTF(strfilenametmp, sizeof(strfilenametmp), 300, "%s.tmp", filename);
    int totalbytes = 0; //已接收文件的总字节数
    int onread = 0; //本次打算接收的总字节数
    char buffer[1000]; //接收文件内容的缓冲区
    FILE* fp = NULL;//创建文件指针
    //创建临时文件
    if ((fp = FOPEN(strfilenametmp, "wb")) == NULL) {
        return false;
    }
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        //计算本次应该接收的字节数
        if (filesize - totalbytes > 1000) {
            onread = 1000;
        }
        else {
            onread = filesize - totalbytes;
        }
        //接收文件内容
        if (Readn(sockfd, buffer, onread) == false) {
            fclose(fp);
            return false;
        }
        //把接收到的内容写入文件
        fwrite(buffer, 1, onread, fp);
        //计算已接收文件的总字节数，如果文件接收完，跳出循环
        totalbytes = totalbytes + onread;
        if (totalbytes == filesize) {
            break;
        }
    }
    //关闭临时文件
    fclose(fp);
    //重置文件时间
    UTime(strfilenametmp, mtime);
    //把临时文件改为（RENAME）正式文件
    if (RENAME(strfilenametmp, filename) == false) {
        return false;
    }
    return true;
}
