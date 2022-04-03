/*程序名称：ftpgetfiles.cpp
* 程序位置：/weather/project/tools1/c
*/
#include "_ftp.h"
struct st_arg {
	char host[31]; //远程服务器的Ip地址的端口号
	int mode; // 传输模式，1 - 被动模式, 2 - 主动模式, 缺省采用被动模式
	char username[31]; //远程服务器的ftp用户名
	char password[31]; // 程服务器的ftp用户的密码
	char remotepath[301]; // 远程服务器存放文件的目录
	char localpath[301]; // 本地文件存放的目录
	char matchname[101]; // 待下载文件的匹配规则。不匹配的文件不会下载，本字段尽可能设置精确，不建议用* 匹配全部的文件
	char listfilename[301]; // 下载前列出服务器文件名的文件
	int ptype; //文件成功下载后，远程服务器文件的处理方式：1 - 什么也不做；2 * 删除；3 - 备份；如果为3，还要指定备份的目录。
	char remotepathbak[301]; // 文件下载成功后，服务器文件的备份目录，此参数只有当参数ptype=3时才有效。
	char okfilename[301]; //已下载成功文件名清单
	bool checkmtime; //是否需要检查服务端文件的时间，true-需要，false-不需要，缺省为false
	int timeout; //进程心跳的超时时间
	char pname[51]; //进程名，建议用“ftpgetfiles_后缀”的方式
} starg;
struct st_fileinfo {
	char filename[301]; //文件名称
	char mtime[21]; //文件时间
};
vector<struct st_fileinfo> vlistfile1; //已下载成功文件名的容器，从okfilename中加载
vector<struct st_fileinfo> vlistfile2; //下载前列出的服务器名的容器。从nlist文件中加载
vector<struct st_fileinfo> vlistfile3; //本次不需要下载的文件的容器
vector<struct st_fileinfo> vlistfile4; //本次需要下载的文件的容器

CLogFile logfile; //日志问价操作类
Cftp ftp; //ftp操作类
CPActive PActive; //进程心跳类
void EXIT(int sig);//程序退出和信号处理函数
void _help();
bool _xmltoarg(char *strxmlbuffer); //把xml解析到参数starg结构中
bool LoadListFile();
bool LoadOKFile(); //加载okfilename文件中的内容到容器vlistfile1中
bool CmpVector(); //比较容器vlistfile2和vlistfiel1，得到vlistfile3和vlistfile4
bool WriteToOkFile(); //把容器vlistfiel3中的内容希尔okfilename文件。覆盖之前的旧okfielname文件
bool AppendToOKFile(struct st_fileinfo *stfilename); //如果ptype=1，把下载成功的文件记录追加到okfilename文件中
bool _ftpgetfiles(); //下载文件功能的主函数
int main(int argc, char* argv[]) {
	/*
	  计划一：把服务器上某目录的文件全部下载到本地目录（可以指定文件名的匹配规则）
	* 参数一：日志文件名
	* 参数二：ftp服务器的IP和端口地址
	* 参数三：ftp的传输模式（主动或被动），缺省为被动
	* 参数四：ftp的用户名
	* 参数五：ftp的密码
	* 参数六：服务器存放文件的目录名
	* 参数七：本地存放文件的目录名
	* 参数八：下载文件名匹配的规则
	* 用xml文件实现传递参数：可读性号、扩展性好、缺省参数可以不写
	*/
	if (argc != 3) {
		_help();
		return -1;
	}
	//处理程序的退出信号
	CloseIOAndSignal(); //关闭全部的信号和输出信号
	signal(SIGINT, EXIT); //设置信号值为2的信号的退出，在shell状态下可用“kill+进程号”正常终止这些进程，但不要用“kill -9 +进程号”强行终止
	signal(SIGTERM, EXIT); //设置信号值为5的信号的退出
	//打开日志文件
	if (logfile.Open(argv[1], "a+") == false) { //日志文件名为第一个参数，即argv[1]
		printf("打开日志文件失败（%s）。\n", argv[1]);
		return -1;
	}
	//解析xml，得到程序的运行参数
	if (_xmltoarg(argv[2]) == false) {
		printf("xml解析失败，请在日志文件中查看失败原因。\n");
		return -1;
	}
	/* 
	 解析xml格式字符串的函数族。
	 bool GetXMLBuffer(const char* xmlbuffer, const char* fieldname, char* value, const int ilen = 0);
     bool GetXMLBuffer(const char* xmlbuffer, const char* fieldname, bool* value);
     bool GetXMLBuffer(const char* xmlbuffer, const char* fieldname, unsigned int* value);
     bool GetXMLBuffer(const char* xmlbuffer, const char* fieldname, long* value);
     bool GetXMLBuffer(const char* xmlbuffer, const char* fieldname, unsigned long* value);
     bool GetXMLBuffer(const char* xmlbuffer, const char* fieldname, double* value)

     xml格式的字符串的内容如下：
     <filename>/tmp/_public.h</filename><mtime>2020-01-01 12:20:35</mtime><size>18348</size>
     <filename>/tmp/_public.cpp</filename><mtime>2020-01-01 10:10:15</mtime><size>50945</size>
     xmlbuffer：待解析的xml格式字符串。
     fieldname：字段的标签名。
     value：传入变量的地址，用于存放字段内容，支持bool、int、insigned int、long、unsigned long、double和char[]。
     注意，当value参数的数据类型为char []时，必须保证value数组的内存足够，否则可能发生内存溢出的问题，也可以用ilen参数限定获取字段内容的长度，ilen的缺省值为0，表示不限长度。
     返回值：true-成功；如果fieldname参数指定的标签名不存在，返回失败。
	*/
	//将进程的心跳信息写入共享内存
	PActive.AddPInfo(starg.timeout, starg.pname);
	//登录ftp服务器
	if (ftp.login(starg.host, starg.username, starg.password, starg.mode) == false) {
		printf("登录失败，请查看日志文件。\n");
		logfile.Write("ftp.login(%s,%s,%s) failed.\n", starg.host, starg.username, starg.password);
		return -1;
	}
	printf("登录成功。\n");
	logfile.Write("ftp.login ok.\n"); //程序正常运行后，可注释掉
	_ftpgetfiles();
	//注销
	ftp.logout();
	return 0;

}
void EXIT(int sig) {
	printf("程序退出,sig=%d\n\n", sig);
	exit(0);
}
void _help() {
	printf("\n");
	printf("Using:/weather/project/tools1/bin/ftpgetfiles logfilename xmlbuffer\n\n");
	printf("Example:/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log \"<host>127.0.0.1:21</host><mode>1</mode><username>another</username><password>ftpNeed</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename><ptype>1</ptype><remotepathbak>/tmp/idc/surfdatabak</remotepathbak><okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename><checkmtime>true</checkmtime><timeout>80</timeout><pname>ftpgetfiles_surfdata</pname>\"\n\n\n");
	printf("本程序是通用的功能模块，用于把远程ftp服务器的文件下载到本地目录。\n");
	printf("logfilename是本程序运行的日志文件。\n");
	printf("xmlbuffer为文件下载的参数。如下：\n");
	printf("<host>127.0.0.1:21</host>远程服务器的Ip地址的端口号\n");
	printf("<mode>1</mode> 传输模式，1-被动模式,2-主动模式,缺省采用被动模式。\n");
	printf("<username>another</username> 远程服务器的ftp用户名。\n");
	printf("<password>ftpNeed</password> 远程服务器的ftp用户的密码。\n");
	printf("<remotepath>/tmp/idc/surfdata</remotepath> 远程服务器存放文件的目录。\n");
	printf("<localpath>/idcdata/surfdata</localpath> 本地文件存放的目录。\n");
	printf("<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname> 待下载文件的匹配规则。不匹配的文件不会下载，本字段尽可能设置精确，不建议用*匹配全部的文件\n");
	printf("<listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename> 本地文件存放的目录。\n");
	printf("<ptype>1</ptype> 文件下载成功后，远程服务器文件的处理方法：1-什么也不做；2*删除；3-备份；如果为3，还要指定备份的目录。\n");
	printf("<remotepathbak>/tmp/idc/surfdatabak</remotepathbak> 文件下载成功后，服务器文件的备份目录，此参数只有当参数ptype=3时才有效。\n");
	printf("<okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename> 已下载成功文件名清单，此参数只有ptype=1时才有效.\n");
	printf("<checkmtime>true</checkmtime> 是否需要检查服务端文件的时间。true-需要，false-不需要,缺省时为false。\n");
	printf("<timeout>80</timeout> 下载文件超时时间，单位:秒,视文件大小、网络带宽而定。\n");
	printf("<pname>ftpgetfiles_surfdata</pname> 进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n\n");
}
bool _xmltoarg(char *strxmlbuffer) { //从xml字符串中获取所需参数
	memset(&starg, 0, sizeof(struct st_arg));
	GetXMLBuffer(strxmlbuffer, "host", starg.host, 30); //远程服务器的IP和端口诋毁，指定长度为30，防止内存溢出
	if (strlen(starg.host) == 0) {
		logfile.Write("host is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "mode", &starg.mode); //传输模式,1-被动模式,2-主动模式,缺省采用被动模式
	if (strlen(starg.host) != 2) {
		starg.mode = 1;
	}
	GetXMLBuffer(strxmlbuffer, "username", starg.username, 30); //远程服务器用户名
	if (strlen(starg.username) == 0) {
		logfile.Write("username is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "password", starg.password, 30); //远程服务器的密码
	if (strlen(starg.password) == 0) {
		logfile.Write("password is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "remotepath", starg.remotepath, 300); //远程服务器存放文件的目录
	if (strlen(starg.remotepath) == 0) {
		logfile.Write("remotepath is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "localpath", starg.localpath, 300); //本地文件存放的目录远程服务器的IP和端口诋毁，指定长度为30，防止内存溢出
	if (strlen(starg.localpath) == 0) {
		logfile.Write("localpath is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname, 100); //待下载文件的匹配规则
	if (strlen(starg.matchname) == 0) {
		logfile.Write("matchname is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "listfilename", starg.listfilename, 300); //下载前列出服务器文件名的文件
	if (strlen(starg.listfilename) == 0) {
		logfile.Write("listfilename is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype); //下载后服务器文件的处理方式：
	if ((starg.ptype != 1) && (starg.ptype != 2)&& (starg.ptype!= 3)) {
		logfile.Write("ptype is error.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "remotepathbak", starg.remotepathbak, 300); //下载后服务器文件的备份目录
	if ((starg.ptype == 3) && (strlen(starg.remotepathbak) == 0)) {
		logfile.Write("remotepathbak is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "okfilename", starg.okfilename, 300); //已下载成功文件名清单
	if ((starg.ptype == 1) && (strlen(starg.okfilename) == 0)) {
		logfile.Write("okfilename is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "checkmtime", &starg.checkmtime); //是否需要检查服务端文件的时间。true-需要，false-不需要,此参数只有ptype=1时才有效
	GetXMLBuffer(strxmlbuffer, "timeout",&starg.timeout); //下载文件的超时时间
	if (starg.timeout == 0) {
		logfile.Write("timeout is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 500); //进程名
	if(strlen(starg.pname) == 0) {
		logfile.Write("pname is null.\n");
		return false;
	}
	return true;
}
bool LoadListFile() { //把ftp.nlist()方法获取到的list文件加载到容器vfilelist中
	vlistfile2.clear(); //清空容器
	CFile File;
	if (File.Open(starg.listfilename, "r") == false) { //用只读的方式打开
		logfile.Write("File.Open(%s) 失败。\n", starg.listfilename);
		return false;
	}
	struct st_fileinfo stfileinfo; //存放文件信息的结构体
	while (true) {
		memset(&stfileinfo, 0, sizeof(struct st_fileinfo)); //初始化结构体
		if (File.Fgets(stfileinfo.filename, 300, true) == false) {
			break;
		}
		if (MatchStr(stfileinfo.filename,starg.matchname) == false) {
			continue;
		}
		if ((starg.ptype == 1) && (starg.checkmtime == true)) {
			//获取ftp服务端的时间
			if (ftp.mtime(stfileinfo.filename) == false) {
				logfile.Write("ftp.mtime(%s) failed.\n", stfileinfo.filename);
				return false;
			}
			strcpy(stfileinfo.mtime,ftp.m_mtime);
		}
		vlistfile2.push_back(stfileinfo);
	}
	/*
	  bool FGETS(const FILE* fp, char* buffer, const int readsize, const char* endbz = 0); // 从文本文件中读取一行。
      fp：已打开的文件指针。
      buffer：用于存放读取的内容，buffer必须大于readsize+1，否则可能会造成读到的数据不完整或内存的溢出。
      readsize：本次打算读取的字节数，如果已经读取到了行结束标志，函数返回。
      endbz：行内容结束的标志，缺省为空，表示行内容以"\n"为结束标志。
      返回值：true-成功；false-失败，一般情况下，失败可以认为是文件已结束。
   */
	/*
	  bool MatchStr(const string& str, const string& rules); //正则表达式，判断一个字符串是否匹配另一个字符串。
      str：需要判断的字符串，是精确表示的，如文件名"_public.cpp"。
      rules：匹配规则的表达式，用星号"*"代表任意字符串，多个表达式之间用半角的逗号分隔，如"*.h,*.cpp"。
      注意：1）str参数不支持"*"，rules参数支持"*"；2）函数在判断str是否匹配rules的时候，会忽略字母的大小写。
	*/
	/*用于测试文件内容是否读取成功
	for (unsigned int i = 0; i < vlistfile2.size(); ++i) {
		logfile.Write("filename=%s=\n", vlistfile2[i].filename);
	}
	*/
	return true;
}
bool LoadOKFile() { //加载okfilename文件中的内容到容器vlistfile1中
	vlistfile1.clear(); //清空容器
	CFile file1;
	if (file1.Open(starg.okfilename,"r") == false) { //只读方式打开文件
		return true; //程序第一次运行时，okfilename不存在
	}
	char strbuffer[501]; 
	struct st_fileinfo stfileinfo; //存放文件信息的结构体
	while (true) {
		memset(&stfileinfo, 0, sizeof(struct st_fileinfo)); //初始化结构体
		if (file1.Fgets(strbuffer, 300, true) == false) {
			break;
		}
		GetXMLBuffer(strbuffer, "filename", stfileinfo.filename);
		GetXMLBuffer(strbuffer, "mtime", stfileinfo.mtime);
		vlistfile1.push_back(stfileinfo);
	}
	return true;
}
bool CmpVector() { //比较容器vlistfile2和vlistfiel1，得到vlistfile3和vlistfile4
	vlistfile3.clear();
	vlistfile4.clear();
	unsigned int i, j;
	for (i = 0; i < vlistfile2.size(); ++i) { //遍历vlistfile2
		for (j = 0; j < vlistfile1.size(); ++j) { //在vlistfile1中查找vlistfile2的记录
			if ((strcmp(vlistfile2[i].filename, vlistfile1[j].filename) == 0) && (strcmp(vlistfile2[i].mtime, vlistfile1[j].mtime) == 0)) { //如果找到了，则把该记录放到vlistfile3中
				vlistfile3.push_back(vlistfile2[i]);
				break;
			}
		}
		if (j == vlistfile1.size()) { //如果没找到，则把该记录放到vlistfile中
			vlistfile4.push_back(vlistfile2[i]);
		}
 	}
	return true;
}
bool WriteToOkFile() { //把容器vlistfiel3中的内容希尔okfilename文件。覆盖之前的旧okfielname文件
	CFile file;
	if (file.Open(starg.okfilename, "w") == false) {
		logfile.Write("file.Open(%s) failed.\n", starg.okfilename);
		return false;
	}
	for (unsigned int i = 0; i < vlistfile3.size(); ++i) {
		file.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n", vlistfile3[i].filename, vlistfile3[i].mtime);
	}
	return true;
}
bool AppendToOKFile(struct st_fileinfo *stfilename) { //如果ptype=1，把下载成功的文件记录追加到okfilename文件中
	CFile file;
	if (file.Open(starg.okfilename, "a") == false) {
		logfile.Write("file.Open(%s) failed.\n", starg.okfilename);
		return false;
	}
	file.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n", stfilename->filename, stfilename->mtime);
	return true;
}
bool _ftpgetfiles() {
	//进入ftp服务器存放文件的目录
	if (ftp.chdir(starg.remotepath) == false) {
		printf("服务器目录打开失败，请查看日志文件.\n");
		logfile.Write("ftp.chdir(%s) failed.\n", starg.remotepath);
		return false;
	}
	//调用ftp.nlist()方法列出服务器目录中的文件，结果存放到本地文件中
	/*if (ftp.nlist(starg.remotepath, starg.listfilename) == false) { //得到文件的全路径
		logfile.Write("ftp.nlist(%s,%s) failed.\n", starg.remotepath, starg.listfilename);
		return false;
	}*/
	if (ftp.nlist(".", starg.listfilename) == false) { //只得到文件名，默认服务器的目录为已进入的服务器目录
		logfile.Write("ftp.nlist(%s) failed.\n", starg.remotepath);
		return false;
	}
	PActive.UptATime(); //更新进程心跳
	//把ftp.nlist()方法获取到的list文件加载到容器vfilelist中
	if (LoadListFile() == false) {
		logfile.Write("LoadListFile() failed.\n");
		return false;
	}
	PActive.UptATime(); //更新进程心跳
	if (starg.ptype == 1) {
		//加载okfilename文件中的内容到容器vlistfile1中
		LoadOKFile();
		//比较vlistfile2和vlistfile1,得到vlistfile3和vlistfile4
		CmpVector();
		//把容器vlistfile3中的内容写到okfilename文件，覆盖之前的旧okfilename文件
		WriteToOkFile();
		//把vlistfile4中的内容复制到vlistfiel2中
		vlistfile2.clear();
		vlistfile2.swap(vlistfile4);
	}
	PActive.UptATime(); //更新进程心跳
	char strremotefilename[301]; //服务器文件路径
	char strlocalfilename[301]; //本地文件路径
	char strremotefilenamebak[301];
	//遍历容器vfilelist2
	for (unsigned int i = 0; i < vlistfile2.size(); ++i) {
		SNPRINTF(strremotefilename,sizeof(strremotefilename),300,"%s/%s", starg.remotepath,vlistfile2[i].filename); //拼接服务器文件的全路径
		SNPRINTF(strlocalfilename, sizeof(strlocalfilename), 300, "%s/%s", starg.localpath, vlistfile2[i].filename); //拼接本地文件的全路径
		//调用ftp.get()方法(需要全路径的文件名)从服务器上下载文件
		logfile.Write("get %s ...", strremotefilename);
		if (ftp.get(strremotefilename, strlocalfilename) == false) {
			logfile.WriteEx("failed.\n", strremotefilename);
			return false;
		}
		logfile.WriteEx("ok.\n");
		PActive.UptATime(); //更新进程心跳
		if (starg.ptype == 1) { //如果ptype=1，把下载成功的文件记录追加到okfilename文件中
			AppendToOKFile(&vlistfile2[i]);
		}
		if (starg.ptype == 2) { //删除文件
			if (ftp.ftpdelete(strremotefilename) == false) {
				printf("服务器文件删除失败，请查看日志文件。\n");
				logfile.Write("ftp.ftpdelete(%s) failed.\n", strremotefilename);
				return false;
			}
		}
		if (starg.ptype == 3) { //将服务器上的文件转存到备份目录
			SNPRINTF(strremotefilenamebak, sizeof(strremotefilenamebak), 300, "%s/%s",starg.remotepathbak, vlistfile2[i].filename);
			if (ftp.ftprename(strremotefilename, strremotefilenamebak) == false) { //给文件改名
				printf("服务器文件重命名失败，请查看日志文件。\n");
				logfile.Write("ftp.ftprename(%s,%s) failed。\n", strremotefilename, strremotefilenamebak);
				return false;
			}
	
		}
		/* 
		 bool ftprename(const char* srcremotefilename, const char* dstremotefilename);// 重命名ftp服务器上的文件。
	     srcremotefilename：ftp服务器上的原文件名。
	     dstremotefilename：ftp服务器上的目标文件名。
	     返回值：true-成功；false-失败。
		*/
	}
	return true;
}
