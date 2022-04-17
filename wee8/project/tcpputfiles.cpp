/*
 * 程序名：tcpputfiles.cpp，采用tcp协议，实现文件发送的客户端。
*/
#include "_public.h"
struct st_arg {
    int clienttype; // 客户端类型：1-上传文件  2-下载文件
    char ip[31]; //服务端的IP地址
    int port; //服务端的端口
    int ptype; //文件上传成功后文件的处理方式 1-删除文件 2-移动到备份目录
    char clientpath[301]; //本地文件存放的根目录
    char clientpathbak[301]; //文件上传成功后，本地文件备份的根目录，当ptype=2时有效
    bool andchild; //是否上传clientpath目录下各级子目录的文件，true-是，false-否
    char matchname[301]; //待上传文件名的匹配方式,如"*.TXT,*.XML"（注意用大写）
    char srvpath[301]; //服务端文件存放的根目录
    int timetvl; //扫描本地目录文件的时间间隔，单位：秒
    int timeout; //进程心跳的超时时间
    char pname[51]; //进程名，建议用"tcpgetfiles_"后缀的方式
} starg;
CTcpClient TcpClient; //创建TCP连接的客户端对象
CLogFile logfile; //创建日志对象
CPActive PActive; //进程心跳对象
char strrecvbuffer[1024]; //接受报文的缓冲区
char strsendbuffer[1024]; //发送报文的缓冲区
bool bcontinue = true; //如果_tcpputfiles发送了文件，bcontinue为true,初始化为true
void EXIT(int sig); //程序退出和信号2、15的处理函数
void _help(); //帮助文档
bool _xmltoarg(char* strxmlbuffer); //把xml解析到参数starg结构中
bool ActiveTest(); //心跳
bool Login(const char *argv); //登录业务
bool _tcpputfiles(); //文件上传的主函数，执行一次文件上传的任务
bool SendFile(const int sockfd,const char *filename,const int filesize); //把文件内容发送给服务端
bool AckMessage(const char *strrecvbuffer); //删除或者转存本地的文件
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
        printf("打开日志文件失败(%s)。\n",argv[1]);
        return -1;
    }
    //解析xml,得到程序运行的参数
    if (_xmltoarg(argv[2]) == false) {
        return -1;
    }
    PActive.AddPInfo(starg.timeout, starg.pname); // 把进程的心跳信息写入共享内存
    //向服务端发出连接请求
    if (TcpClient.ConnectToServer(starg.ip,starg.port) == false)
    {
        logfile.Write("TcpClient.ConnectToServer(%s,%d) failed.\n", starg.ip, starg.port);
        EXIT(-1);
    }
    if (Login(argv[2]) == false) { //登录业务
        logfile.Write("Login() failed.\n");
        EXIT(-1);
    }
    while (true) {
        //调用文件上传的主函数，执行一次文件上传的任务
        if (_tcpputfiles() == false) {
            logfile.Write("_tcpputfiles() failed.\n");
            EXIT(-1);
        }
        if (bcontinue == false) {
            sleep(starg.timetvl);
            if (ActiveTest() == false) {
                break;
            }
        }
        PActive.UptATime();
    }
    EXIT(0);
}
void _help() 
{
    printf("\n");
    printf("Using:/weather/project/tools1/bin/tcpputfiles logfilename xmlbuffer\n\n");
    printf("Example:/weather/project/tools1/bin/procctl 20 /weather/project/tools1/bin/tcpputfiles /log/idc/tcpputfiles_surfdata.log \"<ip>192.168.174.10</ip><port>5005</port><ptype>1</ptype><clientpath>/tmp/tcp/surfdata1</clientpath><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><srvpath>/tmp/tcp/surfdata2</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n\n\n");
    printf("        /weather/project/tools1/bin/procctl 20 /weather/project/tools1/bin/tcpputfiles /log/idc/tcpputfiles_surfdata.log \"<ip>192.168.174.10</ip><port>5005</port><ptype>2</ptype><clientpath>/tmp/tcp/surfdata1</clientpath><clientpathbak>/tmp/tcp/surfdata1bak</clientpathbak><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><srvpath>/tmp/tcp/surfdata2</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n\n\n");
    printf("本程序是数据中心的公共功能模块，采用TCP协议把文件发送给服务端。\n");
    printf("logfilename  是本程序运行的日志文件。\n");
    printf("xmlbuffer    本程序的运行参数，如下：\n");
    printf("ip    服务端的Ip地址\n");
    printf("port  服务端的端口号\n");
    printf("ptype 文件上传成功后，本地文件的处理方法：1删除；2-备份；如果为2，还要指定备份的目录。\n");
    printf("clientpath  本地文件存放的根目录。\n");
    printf("clientpathbak  本地文件上传成功后，本地文件备份的根目录。\n");
    printf("andchild  是否上传clientpath目录下各级子目录的文件，true-是，false-否，缺省为false.\n");
    printf("matchname 待上传文件的匹配规则。如\"*.TXT,*.XML\n");  
    printf("srvpath   服务端文件存放的根目录\n");
    printf("timetvl   扫描本地目录文件的时间间隔，单位：秒，取值在1~30之间。\n");
    printf("timeout   上传文件超时时间，单位:秒,视文件大小、网络带宽而定，建议设置在50以上。\n");
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
    GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype); //解析文件上传后的处理方式
    if ((starg.ptype != 1) && (starg.ptype != 2)) {
        logfile.Write("ptype not in (1,2).\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath); //解析上传文件的根目录
    if (strlen(starg.clientpath) == 0) {
        logfile.Write("clientpath is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "clientpathbak", starg.clientpathbak);//解析本地文件上传后的备份目录
    if ((starg.ptype==2) && (strlen(starg.clientpathbak) == 0)) {
        logfile.Write("clientpathbak is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild); //解析是否上传子目录
    GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname); //解析上传文件的匹配规则
    if (strlen(starg.matchname) == 0) {
        logfile.Write("matchname is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath); //解析服务端文件存放的根目录
    if (strlen(starg.srvpath) == 0) {
        logfile.Write("srvpath is null.\n");
        return false;
    }
    GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl); //解析文件上传周期
    if (starg.timetvl == 0) {
        logfile.Write("timetvl is null.\n");
        return false;
    }
    if (starg.timetvl > 30) {
        starg.timetvl = 30; //扫描本地文件目录的时间间隔没有必要超过30秒
    }
    GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout); //解析上传文件的超时间
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

bool ActiveTest() { //心跳，服务端对心跳报文的响应只会成功，客户端只要能够收到服务端的报文，就认为是成功的
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //接收缓冲区初始化
    memset(strsendbuffer, 0, sizeof(strsendbuffer)); //发送缓冲区初始化
    SPRINTF(strsendbuffer, sizeof(strsendbuffer), "<activetest>ok</activetest>");
    //logfile.Write("发送：%s\n", strsendbuffer);
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    if (TcpClient.Write(strsendbuffer) == false) { //向服务端发送请求报文
        return false;
    }
    if (TcpClient.Read(strrecvbuffer, 20) == false) { //接受服务端的回应报文
        return false;
    }
    //logfile.Write("接收：%s\n", strrecvbuffer);
    return true;
}
bool Login(const char* argv) { //登录业务
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //接收缓冲区初始化
    memset(strsendbuffer, 0, sizeof(strsendbuffer)); //发送缓冲区初始化
    SPRINTF(strsendbuffer, sizeof(strsendbuffer), "%s<clienttype>1</clienttype>", argv);
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
bool _tcpputfiles() {//文件上传的主函数，执行一次文件上传的任务
    CDir Dir; //操作目录的对象
    //调用OpenDir()打开starg.clientpath路径的目录
    if (Dir.OpenDir(starg.clientpath, starg.matchname, 10000, starg.andchild) == false) { //打开子目录
        logfile.Write("Dir.OpenDir(%s) 失败。\n", starg.clientpath);
        return false;
    }
    int delayed=0; //记录未收到文件确认报文的数量
    int buflen = 0; //记录接收到的报文的长度
    bcontinue = false; //表示当前还未传输文件，处于空闲状态
    while (true) {
        memset(strsendbuffer, 0, sizeof(strsendbuffer));
        memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
        //遍历目录中的每个文件，调用ReadDir()获取一个文件名
        if (Dir.ReadDir() == false) {
            break;
        }
        bcontinue = true; ////表示当前正在传输文件，处于忙状态
        //把文件名，修改时间，文件大小组成报文发给服务端
        SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><mtime>%s</mtime><size>%d</size>", Dir.m_FullFileName, Dir.m_ModifyTime, Dir.m_FileSize);
        //logfile.Write("strsendbuffer=%s\n", strsendbuffer);
        if (TcpClient.Write(strsendbuffer) == false) {
            logfile.Write("TcpClient.Write() failed.\n");
            return false;
        }
        //把文件的内容发给服务端
        logfile.Write("send %s(%d) ...", Dir.m_FullFileName, Dir.m_FileSize); //记录要上传的文件
        if (SendFile(TcpClient.m_connfd, Dir.m_FullFileName, Dir.m_FileSize) == true) {
            logfile.WriteEx("ok.\n");
            delayed++;
        }
        else {
            logfile.WriteEx("failed.\n");
            TcpClient.Close();
            return false;
        }
        PActive.UptATime();
        //接受服务端的确认报文
        while (delayed > 0) { //存在未收到的确认报文
            memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
            if (TcpRead(TcpClient.m_connfd, strrecvbuffer, &buflen, - 1) == false) {
                break;
            }
            delayed--;
            // logfile.Write("strrecvbuffer=%s\n", strrecvbuffer);
            //删除或者转存本地的文件
            AckMessage(strrecvbuffer);
        }
    }
    while (delayed > 0) { //存在未收到的确认报文
        memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
        if (TcpRead(TcpClient.m_connfd, strrecvbuffer, &buflen,10) == false) {
            break;
        }
        delayed--;
        //删除或者转存本地的文件
        AckMessage(strrecvbuffer);
    }
    return true;
}
bool SendFile(const int sockfd, const char *filename, const int filesize) { //把文件内容发送给服务端
    int onread = 0; //每次打算读取的字节数
    int bytes = 0; //调用一次fread从文件中读取的字节数
    char buffer[1000]; //存放读取数据的buffer
    int totalbytes=0; //从文件中已经读取的字节总数
    FILE *fp = NULL; //操作文件的指针
    //以“rb”的模式打开文件
    if ((fp = fopen(filename, "rb")) == NULL) {
        return false;
    }
    while (true) {
        memset(buffer, 0, sizeof(buffer)); //初始化数据缓冲区
        //计算本次应该读取的自己数，如果剩余的数据超过1000字节，就打算读1000字节
        if (filesize - totalbytes > 1000) {
            onread = 1000;
        }
        else {
            onread = filesize - totalbytes;
        }
        //从文件中读取数据
        bytes = fread(buffer, 1, onread, fp);
        //把读取到的数据发送给服务端
        if (bytes > 0) {
            if (Writen(sockfd, buffer, bytes) == false) {
                fclose(fp);
                return false;
            }
        }
        //计算文件已读取的字节总数，如果文件已读完，跳出循环
        totalbytes = totalbytes + bytes;
        if (totalbytes == filesize) {
            break;
        }
    }
    fclose(fp);
    return true;
}
bool AckMessage(const char *strrecvbuffer) { //删除或者转存本地的文件
    char filename[301];
    char result[11];
    memset(filename, 0,sizeof(filename));
    memset(result, 0, sizeof(result));
    GetXMLBuffer(strrecvbuffer, "filename", filename, 300);
    GetXMLBuffer(strrecvbuffer, "result", result, 10);
    if (strcmp(result, "ok") != 0) { //如果服务端接收文件不成功，直接返回
        return true;
    }
    if (starg.ptype == 1) {
        if (REMOVE(filename) == false) {
            logfile.Write("REMOVE(%s) failed.\n", filename);
            return false;
        }
        //logfile.Write("REMOVE(%s) ok.\n", filename);
    }
    if (starg.ptype == 2) {
        //生成转存后的备份目录文件名
        char bakfilename[301];
        STRCPY(bakfilename,sizeof(bakfilename),filename);
        UpdateStr(bakfilename, starg.clientpath, starg.clientpathbak, false);
        if (RENAME(filename,bakfilename) == false) {
            logfile.Write("RENAME(%s,%s) failed.\n", filename,bakfilename);
            return false;
        }
        //logfile.Write("RENAME(%s,%s) ok.\n", filename, bakfilename);
    }
    return true;
}
