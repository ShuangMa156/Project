/*程序名称：ftpputfiles .cpp
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
	char matchname[101]; // 待上传件的匹配规则。不匹配的文件不会上传，本字段尽可能设置精确，不建议用* 匹配全部的文件
	int ptype; //文件成功上传后，客户端文件的处理方式：1 - 什么也不做；2 * 删除；3 - 备份；如果为3，还要指定备份的目录。
	char localpathbak[301]; // 文件上传成功后，客户端文件的备份目录，此参数只有当参数ptype=3时才有效。
	char okfilename[301]; //已上传成功文件名清单
	int timeout; //进程心跳的超时时间
	char pname[51]; //进程名，建议用“ftpputfiles_后缀”的方式
} starg;
struct st_fileinfo {
	char filename[301]; //文件名称
	char mtime[21]; //文件时间
};
vector<struct st_fileinfo> vlistfile1; //已上传成功文件名的容器，从okfilename中加载
vector<struct st_fileinfo> vlistfile2; //上传前列出的客户端文件名的容器。从nlist文件中加载
vector<struct st_fileinfo> vlistfile3; //本次不需要上传的文件的容器
vector<struct st_fileinfo> vlistfile4; //本次需要上传的文件的容器

CLogFile logfile; //日志文价操作类
Cftp ftp; //ftp操作类
CPActive PActive; //进程心跳类
void EXIT(int sig);//程序退出和信号处理函数
void _help(); //帮助文档
bool _xmltoarg(char *strxmlbuffer); //把xml解析到参数starg结构中
bool LoadLocalFile(); //把localpath目录下的文件加载到容器vlistfile2中
bool LoadOKFile(); //加载okfilename文件中的内容到容器vlistfile1中
bool CmpVector(); //比较容器vlistfile2和vlistfiel1，得到vlistfile3和vlistfile4
bool WriteToOkFile(); //把容器vlistfiel3中的内容希尔okfilename文件。覆盖之前的旧okfielname文件
bool AppendToOKFile(struct st_fileinfo* stfilename); //如果ptype=1，把上传成功的文件记录追加到okfilename文件中
bool _ftpputfiles(); //上传文件功能的主函数
int main(int argc, char* argv[]) {
	/*
	  计划一：把服务器上某目录的文件全部上传到本地目录（可以指定文件名的匹配规则）
	* 参数一：日志文件名
	* 参数二：ftp服务器的IP和端口地址
	* 参数三：ftp的传输模式（主动或被动），缺省为被动
	* 参数四：ftp的用户名
	* 参数五：ftp的密码
	* 参数六：服务器存放文件的目录名
	* 参数七：本地存放文件的目录名
	* 参数八：上传文件名匹配的规则
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
	_ftpputfiles();
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
	printf("Using:/weather/project/tools1/bin/ftpputfiles logfilename xmlbuffer\n\n");
	printf("Example:/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log \"<host>127.0.0.1:21</host><mode>1</mode><username>another</username><password>ftpNeed</password><localpath>/tmp/idc/surfdata</localpath><remotepath>/tmp/ftpputest</remotepath><matchname>SURF_ZH*.JSON</matchname><ptype>1</ptype><localpathbak>/tmp/idc/surfdatabak</localpathbak><okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename><timeout>80</timeout><pname>ftpputfiles_surfdata</pname>\"\n\n\n");
	printf("本程序是通用的功能模块，用于把远程ftp服务器的文件上传到本地目录。\n");
	printf("logfilename是本程序运行的日志文件。\n");
	printf("xmlbuffer为文件上传的参数。如下：\n");
	printf("<host>127.0.0.1:21</host>远程服务器的Ip地址的端口号\n");
	printf("<mode>1</mode> 传输模式，1-被动模式,2-主动模式,缺省采用被动模式。\n");
	printf("<username>another</username> 远程服务器的ftp用户名。\n");
	printf("<password>ftpNeed</password> 远程服务器的ftp用户的密码。\n");
	printf("<remotepath>/tmp/ftpputest</remotepath> 远程服务器存放文件的目录。\n");
	printf("<localpath>/tmp/idc/surfdata</localpath> 本地文件存放的目录。\n");
	printf("<matchname>SURF_ZH*.JSON</matchname> 待上传文件的匹配规则。不匹配的文件不会上传，本字段尽可能设置精确，不建议用*匹配全部的文件\n");
	printf("<ptype>1</ptype> 文件上传成功后，本地文件的处理方法：1-什么也不做；2*删除；3-备份；如果为3，还要指定备份的目录。\n");
	printf("<localpathbak>/tmp/idc/surfdatabak</remotepathbak> 文件上传成功后，本地文件的备份目录，此参数只有当参数ptype=3时才有效。\n");
	printf("<okfilename>/idcdata/ftplist/ftpputfiles _surfdata.xml</okfilename> 已上传成功文件名清单，此参数只有ptype=1时才有效.\n");
	printf("<timeout>80</timeout> 上传文件超时时间，单位:秒,视文件大小、网络带宽而定。\n");
	printf("<pname>ftpputfiles _surfdata</pname> 进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n\n");
}
bool _xmltoarg(char* strxmlbuffer) { //从xml字符串中获取所需参数
	memset(&starg, 0, sizeof(struct st_arg));
	GetXMLBuffer(strxmlbuffer, "host", starg.host, 30); //远程服务器的IP和端口诋毁，指定长度为30，防止内存溢出
	if (strlen(starg.host) == 0) {
		logfile.Write("host is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "mode", &starg.mode); //传输模式,1-被动模式,2-主动模式,缺省采用被动模式
	if (starg.mode!= 2) {
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
	GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname, 100); //待上传文件的匹配规则
	if (strlen(starg.matchname) == 0) {
		logfile.Write("matchname is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype); //上传后本地文件的处理方式：
	if ((starg.ptype != 1) && (starg.ptype != 2) && (starg.ptype != 3)) {
		logfile.Write("ptype is error.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "localpathbak", starg.localpathbak, 300); //上传后本地文件的备份目录
	if ((starg.ptype == 3) && (strlen(starg.localpathbak) == 0)) {
		logfile.Write("localpathbak is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "okfilename", starg.okfilename, 300); //已上传成功文件名清单
	if ((starg.ptype == 1) && (strlen(starg.okfilename) == 0)) {
		logfile.Write("okfilename is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout); //上传文件的超时时间
	if (starg.timeout == 0) {
		logfile.Write("timeout is null.\n");
		return false;
	}
	GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 500); //进程名
	if (strlen(starg.pname) == 0) {
		logfile.Write("pname is null.\n");
		return false;
	}
	return true;
}
bool LoadLocalFile() { //把ftp.nlist()方法获取到的list文件加载到容器vfilelist中
	vlistfile2.clear(); //清空容器
	CDir Dir;
	Dir.SetDateFMT("yyyymmddhh24miss"); //设置时间的格式
	/*//注意：该方法不包括子目录。
	        如果本地文件目录下的总数超过10000（缺省值），增量上传文件功能将有问题，
			建议用deletefiles程序计时清理本地文件目录中的历史文件
	*/
	if (Dir.OpenDir(starg.localpath,starg.matchname)==false) { 
		logfile.Write("Dir.OpenDir(%s) 失败。\n", starg.localpath);
		return false;
	}
	/*
	bool OpenDir(const char* in_DirName, const char* in_MatchStr, const unsigned int in_MaxCount = 10000, const bool bAndChild = false, bool bSort = false);// 打开目录，获取目录中的文件列表信息，存放于m_vFileName容器中。
    in_DirName，待打开的目录名，采用绝对路径，如/tmp/root。
    in_MatchStr，待获取文件名的匹配规则，不匹配的文件被忽略，具体请参见开发框架的MatchStr函数。
    in_MaxCount，获取文件的最大数量，缺省值为10000个。
    bAndChild，是否打开各级子目录，缺省值为false-不打开子目录。
    bSort，是否对获取到的文件列表（即m_vFileName容器中的内容）进行排序，缺省值为false-不排序。
    返回值：true-成功，false-失败，如果in_DirName参数指定的目录不存在，OpenDir方法会创建该目录，如果创建失败，返回false，如果当前用户对in_DirName目录下的子目录没有读取权限也会返回false。
	*/
	struct st_fileinfo stfileinfo; //存放文件信息的结构体
	while (true) {
		memset(&stfileinfo, 0, sizeof(struct st_fileinfo)); //初始化结构体
		if (Dir.ReadDir() == false) {
			break;
		}
		strcpy(stfileinfo.filename, Dir.m_FileName); //将打开的目录中的文件名放到结构体中,不包括目录名
		strcpy(stfileinfo.mtime, Dir.m_ModifyTime); //将打开的目录中的时间放到结构体中
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
	if (file1.Open(starg.okfilename, "r") == false) { //只读方式打开文件
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
bool AppendToOKFile(struct st_fileinfo* stfilename) { //如果ptype=1，把上传成功的文件记录追加到okfilename文件中
	CFile file;
	if (file.Open(starg.okfilename, "a") == false) {
		logfile.Write("file.Open(%s) failed.\n", starg.okfilename);
		return false;
	}
	file.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n", stfilename->filename, stfilename->mtime);
	return true;
}
bool _ftpputfiles() {
    //把localpath目录下的文件加载到vlistfile2容器中
	if (LoadLocalFile() == false) {
		logfile.Write("LoadLocalFile() failed.\n");
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
	//遍历容器vfilelist2
	for (unsigned int i = 0; i < vlistfile2.size(); ++i) {
		SNPRINTF(strremotefilename, sizeof(strremotefilename), 300, "%s/%s", starg.remotepath, vlistfile2[i].filename); //拼接服务器文件的全路径
		SNPRINTF(strlocalfilename, sizeof(strlocalfilename), 300, "%s/%s", starg.localpath, vlistfile2[i].filename); //拼接本地文件的全路径
		//调用ftp.put()方法(需要全路径的文件名)把文件上传到服务器
		logfile.Write("put %s ...", strlocalfilename);
		if (ftp.put(strlocalfilename, strremotefilename,true) == false) {
			logfile.WriteEx("failed.\n");
			return false;
		}
		logfile.WriteEx("ok.\n");
		PActive.UptATime(); //更新进程心跳
		if (starg.ptype == 1) { //如果ptype=1，把上传成功的文件记录追加到okfilename文件中
			AppendToOKFile(&vlistfile2[i]);
		}
		if (starg.ptype == 2) { //删除文件
			if (REMOVE(strlocalfilename) == false) {
				printf("本地文件删除失败，请查看日志文件。\n");
				logfile.Write("REMOVE(%s) failed.\n", strlocalfilename);
				return false;
			}
		}
		if (starg.ptype == 3) { //将本地的文件转存到备份目录
			char strlocalfilenamebak[301];
			SNPRINTF(strlocalfilenamebak, sizeof(strlocalfilenamebak), 300, "%s/%s", starg.localpathbak, vlistfile2[i].filename);
			if (RENAME(strlocalfilename, strlocalfilenamebak) == false) { //给文件改名
				printf("本地文件重命名失败，请查看日志文件。\n");
				logfile.Write("RENAME(%s,%s) failed。\n", strlocalfilename, strlocalfilenamebak);
				return false;
			}

		}
		/*
		* bool put(const char* localfilename, const char* remotefilename, const bool bCheckSize = true);// 向ftp服务器发送文件。（一个文件是否发生变化只能用文件的时间来衡量，不能用文件的大小）
	    localfilename：本地待发送的文件名。
	    remotefilename：发送到ftp服务器上的文件名。
	    bCheckSize：文件传输完成后，是否核对本地文件和远程文件的大小，保证文件的完整性。
	    返回值：true-成功；false-失败。
	    注意：文件在传输的过程中，采用临时文件命名的方法，即在remotefilename后加".tmp"，在传输
	   完成后才正式改为remotefilename。
		*/
	}
	return true;
}
