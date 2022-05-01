/*
*程序名称：execsql.cpp，这是一个工具程序，用于执行一个sql脚本文件
* 作者：马双
*/
#include "_public.h"
#include "_mysql.h"
CLogFile logfile; //日志文件对象
connection conn; //用于连接数据库的对象
CPActive PActive; //进程心跳对象
void EXIT(int sig); //信号处理函数
int main(int argc, char* argv[]) {
	//帮助文档
	if (argc != 5) {
		printf("\n");
		printf("Using:./execsql sqlfile connstr charset logfile\n");
		printf("Example:/weather/project/tools1/bin/procctl 120 /weather/project/tools1/bin/execsql /weather/project/idc1/sql/cleardata.sql \"127.0.0.1,root,mysql2022,Weather_DataCenter,3306\" utf8 /log/idc/execsql.log\n\n");
		printf("这是一个工具程序，用于执行一个sql脚本文件。\n");
		printf("sqlfile sql脚本文件名，每条sql语句可以多行书写，分号表示一条sql语句的结束，不支持注释。\n");
		printf("connstr 数据库连接参数：ip,username,password,dbname,port\n");
		printf("charset 数据库的字符集。\n");
		printf("logfile 本程序运行的日志文件名。\n\n");
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
	PActive.AddPInfo(500, "obtcodetodb"); //进程心跳，间隔时间为10秒
	//注意：调试程序时，可以启用如下心跳代码，防止超时（时间太短会被守护进程杀掉）
	// PActive.AddPInfo(5000, "obtcodetodb"); //进程心跳，间隔时间为10秒
	//连接数据库，不启用事务
	if (conn.connecttodb(argv[2], argv[3],1) != 0) {
		logfile.Write("connect database(%s) failed.\n%s\n", argv[2], conn.m_cda.message);
		return -1;
	}
	logfile.Write("connect to database ok.\n", argv[2]);
	CFile File;
	if (File.Open(argv[1], "r") == false) {
		logfile.Write("File.Open(%s) failed.\n", argv[1]);
		EXIT(-1);
	}
	char strsql[1001]; //存放从SQL文件中读取到的SQL语句
	while (true) {
		memset(strsql, 0, sizeof(strsql));
		//从SQL文件中读取以分号结束的一行
		if (File.FFGETS(strsql, 1000, ";") == false) {
			break;
		}
		//删除sql语句最后的分号
		char* pp = strstr(strsql, ";");
		if (pp == 0) {
			continue;
		}
		pp[0] = 0;
		logfile.Write("%s\n", strsql);
		int iret = conn.execute(strsql); //执行sql语句
		if (iret == 0) {
			logfile.Write("exec ok(rpc=%d).\n", conn.m_cda.rpc);
		}
		else {
			logfile.Write("exec failed(%s).\n", conn.m_cda.message);
		}
		PActive.UptATime(); //进程心跳
	}
	logfile.WriteEx("\n"); //加入空行
	return 0;

}
void EXIT(int sig) { //信号处理函数
	logfile.Write("程序退出，sig=%d\n\n", sig);
	conn.disconnect(); //断开数据库连接
	exit(0);
}
