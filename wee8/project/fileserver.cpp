/*
 * 程序名：fileserver.cpp，采用tcp实现文件传输的服务端。
 *  (1)在多进程的服务程序中，如果杀掉一个子进程，则和这个子进程通信的客户端会断开，但是不会影响到其他的子进程与对应的客户端的通信，也不会影响父进程。
 * （2）如果杀掉父进程，不会影响正在通信中的子进程，但是新的客户端无法尽量连接
 * （3）如果用killall+程序名，可以杀掉父进程和全部的子进程
 *
 * 多进程网络服务端程序退出的三种情况：
 * （1）如果是子进程收到退出信号，该子进程断开与客户端连接的socket，然后退出---->断开某一客户端的连接
 * （2）如果是父进程收到退出信号，父进程先关闭监听的socket，然后向全部的子进程发出退出信号---->停止服务器。断开全部客户端的连接
 * （3）如果父子进程都收到退出信号，本质上与第二种情况相同---->停止服务器。断开全部客户端的连接
*/
#include "_public.h"
struct st_arg {
    int clienttype; // 客户端类型：1-上传文件  2-下载文件
    char ip[31]; //服务端的IP地址
    int port; //服务端的端口
    int ptype; //文件上传成功后文件的处理方式 1-删除文件 2-移动到备份目录
    char clientpath[301]; //本地文件存放的根目录
    bool andchild; //是否传输各级子目录的文件，true-是，false-否
    char matchname[301]; //待传输文件名的匹配方式,如"*.TXT,*.XML"（注意用大写）
    char srvpath[301]; //服务端文件存放的根目录
    char srvpathbak[301]; //服务端文件的备份目录
    int timetvl; //扫描目录文件的时间间隔，单位：秒
    int timeout; //进程心跳的超时时间
    char pname[51]; //进程名，建议用"tcpgetfiles_"后缀的方式
} starg;
CLogFile logfile; //服务程序的运行日志
CTcpServer TcpServer; //创建服务端对象
CPActive PActive; //进程心跳对象
char strrecvbuffer[1024]; //接受报文的缓冲区
char strsendbuffer[1024]; //发送报文的缓冲区
bool bcontinue = true; //如果_tcpputfiles发送了文件，bcontinue为true,初始化为true
bool _xmltoarg(char* strxmlbuffer); //把xml解析到参数starg结构中
void FathEXIT(int sig); //父进程的退出函数
void ChldEXIT(int sig); //子进程的退出函数
bool ClientLogin(); //登录
bool ActiveTest(); //心跳
bool _tcpputfiles(); //文件上传的主函数，执行一次文件上传的任务
bool SendFile(const int sockfd, const char* filename, const int filesize); //把文件内容发送给服务端
bool AckMessage(const char* strrecvbuffer); //删除或者转存本地的文件
void RecvFilesMain(); //上传文件的主函数
bool RecvFile(const int sockfd, const char *filename, const char *mtime, int filesize); //接收上传文件的内容
void SendFilesMain(); //下载文件的主函数

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("Using:./fileserver port logfilename\n");
        printf("Example:/weather/project/tools1/bin/fileserver 5005 /log/idc/fileserver.log\n");
        printf("        /weather/project/tools1/bin/procctl 10 /weather/project/tools1/bin/fileserver 5005 /log/idc/fileserver.log\n\n\n");
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
        
        //多进程
        
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
        //处理登录客户端的登录报文
        if (ClientLogin() == false) {
            ChldEXIT(-1);
        }
        //如果clienttype=1,调用上传文件的主函数
        if (starg.clienttype == 1) {
            RecvFilesMain();
        }
        //如果clienttype=2,调用下载文件的主函数
        if (starg.clienttype == 2) {
            SendFilesMain();
        }
        ChldEXIT(0); //调用子进程的退出函数使子进程退出

    }
}
bool _xmltoarg(char* strxmlbuffer) {//把xml解析到参数starg结构中(无需再对参数进行合法性判断，因为服务端已经判断过)
    memset(&starg, 0, sizeof(struct st_arg));
    GetXMLBuffer(strxmlbuffer, "clienttype", &starg.clienttype); //解析IP地址
    GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype); //解析文件上传后的处理方式
    GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath); //解析上传文件的根目录
    GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild); //解析是否上传子目录
    GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname); //解析上传文件的匹配规则
    GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath); //解析服务端文件存放的根目录
    GetXMLBuffer(strxmlbuffer, "srvpathbak", starg.srvpathbak); //解析服务端文件的备份目录

    GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl); //解析文件上传周期
    if (starg.timetvl > 30) {
        starg.timetvl = 30; //扫描本地文件目录的时间间隔没有必要超过30秒
    }
    GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout); //解析上传文件的超时间
    if (starg.timeout < 50) {
        starg.timeout = 50; //进程心跳的超时时间没有必要小于50秒
    }
    GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50); //解析进程名称
    strcat(starg.pname, "_srv");
    return true;
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
bool ClientLogin() { //登录
    memset(strsendbuffer, 0, sizeof(strsendbuffer)); //初始化发送缓冲区
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //初始化接收缓冲区
    if (TcpServer.Read(strrecvbuffer, 20) == false) {
        logfile.Write("TcpServer.Read() failed.\n");
        return false;
    }
    //logfile.Write("strrecvbuffer=%s\n", strrecvbuffer);

    //解析客户端的登录报文
    _xmltoarg(strrecvbuffer);
    if ((starg.clienttype != 1) && (starg.clienttype != 2)) {
        strcpy(strsendbuffer,"failed"); 
    }
    else {
        strcpy(strsendbuffer, "ok");
    }
    //处理业务
    if (TcpServer.Write(strsendbuffer) == false) {
        logfile.Write("TcpServer.Write() failed.\n");
        return false;
    }
    logfile.Write("%s login %s.\n", TcpServer.GetIP(),strsendbuffer);
    return true;
}
void RecvFilesMain() { //上传文件的主函数
    PActive.AddPInfo(starg.timeout,starg.pname);
    while (true) {
        memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //初始化接收缓冲区
        memset(strsendbuffer, 0, sizeof(strsendbuffer)); //初始化发送缓冲区
        PActive.UptATime();
        //接收客户端的报文
        if (TcpServer.Read(strrecvbuffer, starg.timetvl+10) == false) {
            logfile.Write("TcpServer.Read() failed.\n");
            return;
        }
        //logfile.Write("strrecvbuffer=%s\n", strrecvbuffer);
        //处理心跳报文
        if (strcmp(strrecvbuffer, "<activetest>ok</activetest>") == 0) {
            strcpy(strsendbuffer, "ok");
            //logfile.Write("strsendbuffer=%s\n", strsendbuffer);
            if (TcpServer.Write(strsendbuffer) == false) {
                logfile.Write("TcpServer.Write() failed.\n");
                return;
            }
        }
        //处理文件上传的请求报文
        if (strncmp(strrecvbuffer, "<filename>", 10) == 0) {
            //解析上传文件请求报文的xml
            char clientfilename[301]; //文件名
            memset(clientfilename, 0, sizeof(clientfilename));
            char mtime[21]; //文件修改时间
            memset(mtime, 0, sizeof(mtime));
            int filesize = 0; //文件大小
            GetXMLBuffer(strrecvbuffer, "filename", clientfilename, 300);
            GetXMLBuffer(strrecvbuffer, "mtime",mtime,19);
            GetXMLBuffer(strrecvbuffer, "size", &filesize);
            //由于客户端可服务端的文件名是不同的，所以需要生成服务端的文件名，将文件中的clientpath替换为srvpath
            char serverfilename[301];
            memset(serverfilename, 0, sizeof(serverfilename));
            strcpy(serverfilename, clientfilename);
            UpdateStr(serverfilename, starg.clientpath, starg.srvpath, false);
            //接收上传文件的内容
            logfile.Write("recv %s(%d)...", serverfilename, filesize);
            if (RecvFile(TcpServer.m_connfd, serverfilename, mtime, filesize) == true) {
                logfile.WriteEx("ok.\n");
                SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>ok</result>", clientfilename);
            }
            else {
                logfile.WriteEx("failed.\n");
                SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000, "<filename>%s</filename><result>failed</result>", clientfilename);
            }
            //把接收结果返回客户端
            //logfile.Write("strsendbuffer=%s\n", strsendbuffer);
            if (TcpServer.Write(strsendbuffer) == false) {
                logfile.Write("TcpServer.Write() failed.\n");
                return;
            }
        }
    }
}
bool RecvFile(const int sockfd, const char* filename, const char* mtime, int filesize) {//接收上传文件的内容
    //生成临时文件名
    char strfilenametmp[301];
    SNPRINTF(strfilenametmp, sizeof(strfilenametmp), 300, "%s.tmp", filename);
    int totalbytes = 0; //已接收文件的总字节数
    int onread = 0; //本次打算接收的总字节数
    char buffer[1000]; //接收文件内容的缓冲区
    FILE *fp = NULL;//创建文件指针
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
bool ActiveTest() { //心跳，服务端对心跳报文的响应只会成功，客户端只要能够收到服务端的报文，就认为是成功的
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer)); //接收缓冲区初始化
    memset(strsendbuffer, 0, sizeof(strsendbuffer)); //发送缓冲区初始化
    SPRINTF(strsendbuffer, sizeof(strsendbuffer), "<activetest>ok</activetest>");
    //logfile.Write("发送：%s\n", strsendbuffer);
    //与服务端通信，发送一个报文后等待回复，然后再发下一个报文
    if (TcpServer.Write(strsendbuffer) == false) { //向服务端发送请求报文
        return false;
    }
    if (TcpServer.Read(strrecvbuffer, 20) == false) { //接受服务端的回应报文
        return false;
    }
    //logfile.Write("接收：%s\n", strrecvbuffer);
    return true;
}
void SendFilesMain() { //下载文件的主函数
    PActive.AddPInfo(starg.timeout, starg.pname);
    while (true) {
        //调用文件下载的主函数，执行一次文件下载的任务
        if (_tcpputfiles() == false) {
            logfile.Write("_tcpputfiles() failed.\n");
            return;
        }
        if (bcontinue == false) {
            sleep(starg.timetvl);
            if (ActiveTest() == false) {
                break;
            }
        }
        PActive.UptATime();
    }
}
bool _tcpputfiles() {//文件上传的主函数，执行一次文件上传的任务
    CDir Dir; //操作目录的对象
    //调用OpenDir()打开starg.srvpath路径的目录
    if (Dir.OpenDir(starg.srvpath, starg.matchname, 10000, starg.andchild) == false) { //打开子目录
        logfile.Write("Dir.OpenDir(%s) 失败。\n", starg.srvpath);
        return false;
    }
    int delayed = 0; //记录未收到文件确认报文的数量
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
        if (TcpServer.Write(strsendbuffer) == false) {
            logfile.Write("TcpServer.Write() failed.\n");
            return false;
        }
        //把文件的内容发给服务端
        logfile.Write("send %s(%d) ...", Dir.m_FullFileName, Dir.m_FileSize); //记录要上传的文件
        if (SendFile(TcpServer.m_connfd, Dir.m_FullFileName, Dir.m_FileSize) == true) {
            logfile.WriteEx("ok.\n");
            delayed++;
        }
        else {
            logfile.WriteEx("failed.\n");
            TcpServer.CloseClient();
            return false;
        }
        PActive.UptATime();
        //接受服务端的确认报文
        while (delayed > 0) { //存在未收到的确认报文
            memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
            if (TcpRead(TcpServer.m_connfd, strrecvbuffer, &buflen, -1) == false) {
                break;
            }
            delayed--;
             //logfile.Write("strrecvbuffer=%s\n", strrecvbuffer);
            //删除或者转存本地的文件
            AckMessage(strrecvbuffer);
        }
    }
    while (delayed > 0) { //存在未收到的确认报文
        memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
        if (TcpRead(TcpServer.m_connfd, strrecvbuffer, &buflen, 10) == false) {
            break;
        }
        delayed--;
        //删除或者转存本地的文件
        AckMessage(strrecvbuffer);
    }
    return true;
}
bool SendFile(const int sockfd, const char* filename, const int filesize) { //把文件内容发送给服务端
    int onread = 0; //每次打算读取的字节数
    int bytes = 0; //调用一次fread从文件中读取的字节数
    char buffer[1000]; //存放读取数据的buffer
    int totalbytes = 0; //从文件中已经读取的字节总数
    FILE* fp = NULL; //操作文件的指针
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
bool AckMessage(const char* strrecvbuffer) { //删除或者转存本地的文件
    char filename[301];
    char result[11];
    memset(filename, 0, sizeof(filename));
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
    }
    if (starg.ptype == 2) {
        //生成转存后的备份目录文件名
        char bakfilename[301];
        STRCPY(bakfilename, sizeof(bakfilename), filename);
        UpdateStr(bakfilename, starg.srvpath, starg.srvpathbak, false);
        if (RENAME(filename, bakfilename) == false) {
            logfile.Write("RENAME(%s,%s) failed.\n", filename, bakfilename);
            return false;
        }
    }
    return true;
}

/*
* void UpdateStr(char* str, const char* str1, const char* str2, const bool bloop = true) // 字符串替换函数。
   在字符串str中，如果存在字符串str1，就替换为字符串str2。
   str：待处理的字符串。
   str1：旧的内容。
   str2：新的内容。
   bloop：是否循环执行替换。
   注意：
   1、如果str2比str1要长，替换后str会变长，所以必须保证str有足够的空间，否则内存会溢出。
   2、如果str2中包含了str1的内容，且bloop为true，这种做法存在逻辑错误，UpdateStr将什么也不做。
*/
/*
* FILE* FOPEN(const char* filename, const char* mode);// 打开文件,FOPEN函数调用fopen库函数打开文件，如果文件名中包含的目录不存在，就创建目录。
  注意：
   1、FOPEN函数的参数和返回值与fopen函数完全相同。
   2、在应用开发中，用FOPEN函数代替fopen库函数。
*/
/*bool UTime(const char* filename, const char* mtime); // 重置文件的修改时间属性。
  filename：待重置的文件名，建议采用绝对路径的文件名。
  stime：字符串表示的时间，格式不限，但一定要包括yyyymmddhh24miss，一个都不能少。
  返回值：true-成功；false-失败，失败的原因保存在errno中。
*/
