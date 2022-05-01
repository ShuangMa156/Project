/*
*程序名称：obtmindtodb.cpp，本程序用于把全国气象站分钟观测数据保存到数据库Weather_DataCenter的T_ZHOBTCODE表中,支持xml和csv格式
* 作者：马双
*/
#include "idcapp.h"
CLogFile logfile; //日志文件对象
connection conn; //用于连接数据库的对象
CPActive PActive; //进程心跳对象
void EXIT(int sig); //信号处理函数
bool _obtmindtodb(char* pathname, char* connstr, char* charset); //业务处理主函数
int main(int argc, char* argv[]) {
	//帮助文档
	if (argc != 5) {
		printf("\n");
		printf("Using:./obtmindtodb pathname connstr charset logfile\n");
		printf("Example:/weather/project/tools1/bin/procctl 120 /weather/project/idc1/bin/obtmindtodb /idcdata/surfdata \"127.0.0.1,root,mysql2022,Weather_DataCenter,3306\" utf8 /log/idc/obtmindtodb.log\n\n");
		printf("本程序用于把全国站点分钟观测数据保存到数据库的表T_ZHOBTMIND中，数据只插入，不更新\n");
		printf("pathname 全关站点分钟观测数据存放的目录。\n");
		printf("connstr 数据库连接参数：ip,username,password,dbname,port\n");
		printf("charset 数据库的字符集。\n");
		printf("logfile 本程序运行的日志文件名。\n");
		printf("程序每120秒运行一次，由procctl调度。\n\n\n");
		return -1;
	}
	//处理程序的退出信号
	CloseIOAndSignal(); //关闭全部信号的输入和输出
	signal(SIGINT, EXIT); //设置信号，在shell状态下可用“kill+进程号"正常终止程序
	signal(SIGTERM, EXIT);
	//打开日志文件
	if (logfile.Open(argv[4], "a+") == false) {
		printf("打开日志文件失败(%s)。\n", argv[4]);
		return -1;
	}
	PActive.AddPInfo(30, "obtmindtodb"); //进程心跳，间隔时间为10秒
	//注意：调试程序时，可以启用如下心跳代码，防止超时（时间太短会被守护进程杀掉）
	 //PActive.AddPInfo(5000, "obtcodetodb"); //进程心跳，间隔时间为10秒
	//业务处理主函数
	_obtmindtodb(argv[1], argv[2], argv[3]);
	return 0;
}

void EXIT(int sig) { //信号处理函数
	logfile.Write("程序退出，sig=%d\n\n", sig);
	conn.disconnect(); //断开数据库连接
	exit(0);
}
bool _obtmindtodb(char* pathname, char* connstr, char* charset) {//业务处理主函数
	CDir Dir; //创建目录对象
	//打开目录
	if (Dir.OpenDir(pathname, "*.xml,*.csv") == false) {
		logfile.Write("Dir.OpenDir(%s) failed.\n", pathname);
		return false;
	}
	CFile File;
	CZHOBTMIND ZHOBTMIND(&conn, &logfile); //创建CZHOBTMIND类的对象
	int totalcount = 0; //文件的总记录数
	int insertcount = 0; //成功插入的记录数
	CTimer Timer; //计时器,记录每个数据文件的处理耗时
	bool bisxml = false; //记录文件的格式：true-xml,false-csv
	//遍历目录中的文件名
	while (true) {
		//读取目录，得到一个数据文件名
		if (Dir.ReadDir() == false) {
			break;
		}
		if (MatchStr(Dir.m_FullFileName, "*.xml") == true) {
			bisxml = true;
		}
		else {
			bisxml = false;
		}
		if (conn.m_state == 0) { //未连接数据库
			//连接数据库
			if (conn.connecttodb(connstr,charset) != 0) {
				logfile.Write("connect database(%s) failed.\n%s\n", connstr, conn.m_cda.message);
				return false;
			}
			logfile.Write("connect to database ok.\n", connstr);
		}
		//logfile.Write("filename=%s\n", Dir.m_FullFileName);
		totalcount = 0;
		insertcount = 0;
		//打开文件
		if (File.Open(Dir.m_FullFileName,"r") == false) {
			logfile.Write("File.Open(%s) Failed.\n", Dir.m_FullFileName);
			return false;
		}
		char strBuffer[1001]; //存放从文件中读取的一行

		while (true) {
			if (bisxml == true) {
				if (File.FFGETS(strBuffer, 1000, "<endl/>") == false) {
					break;
				}
			}
			else{
				if (File.Fgets(strBuffer, 1000, true) == false) {
					break;
				}
				if (strstr(strBuffer, "站点")!=0) { //把文件中的第一行扔掉
					continue;
				}
			}
			//logfile.Write("strBuffer=%s", strBuffer); //把读取的一行内容写入日志文件，方便调试
			//处理文件中的每一行
			totalcount++;
			ZHOBTMIND.SplitBuffer(strBuffer,bisxml);
			if (ZHOBTMIND.InsertTable() == true) {
				insertcount++;
			}
		}
		//删除文件，提交事务
		File.CloseAndRemove();
		conn.commit(); //提交事务
		logfile.Write("已处理文件%s(totalcount=%d,insertcount=%d),耗时%.2f秒。\n", Dir.m_FullFileName, totalcount, insertcount, Timer.Elapsed());
	}
	return true;
}
