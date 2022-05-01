/*
* 程序名称：obtcodetodb.cpp，本程序用于把全国气象站点参数数据保存到数据库Weather_DataCenter的T_ZHOBTCODE表中
* 作者：马双
* 时间：2022-04-30
*/
#include "_public.h"
#include "_mysql.h"
CLogFile logfile; //日志文件对象
struct st_stcode //思考1：结构体的内存分布
{
	char obtid[11]; //站号
	char cityname[31]; //站名
	char provname[31]; //省
	char lat[11]; //维度
	char lon[11]; //精度
	char height[11]; //海拔高度
};
//存放全国气象站点参数容器
vector<struct st_stcode> vstcode; //思考3：向量容器的使用
connection conn; //用于连接数据库的对象
CPActive PActive; //进程心跳对象
bool LoadSTCode(const char* inifile);//把站点参数文件加载到vstcode容器中
void EXIT(int sig); //信号处理函数
int main(int argc, char* argv[]) {
	//帮助文档
	if (argc != 5) {
		printf("\n");
		printf("Using:./obtcodetodb inifile connstr charset logfile\n");
		printf("Example:/weather/project/tools1/bin/procctl 120 /weather/project/idc1/bin/obtcodetodb /weather/project/idc1/ini/stcode.ini \"127.0.0.1,root,mysql2022,Weather_DataCenter,3306\" utf8 /log/idc/obtcodetodb.log\n\n");
		printf("本程序用于把全国站点参数数据保存到数据库的表中，如果站点不存在则插入，站点已存在则更新。\n");
		printf("inifile 站点参数文件名（全路径）.\n");
		printf("connstr 数据库连接参数：ip,username,password,dbname,port\n");
		printf("charset 数据库的字符集。\n");
		printf("logfile 本程序运行的日志文件名。\n");
		printf("程序每120秒运行一次，由procctl调度。\n\n\n");
		return -1;
	}
	//处理程序的退出信号
	CloseIOAndSignal(); //关闭全部信号的输入和输出
	signal(SIGINT, EXIT); //设置信号，在shell状态下可用“kill+进程号"正常终止程序
	signal(SIGTERM,EXIT);
	//打开日志文件
	if (logfile.Open(argv[4], "a+") == false) {
		printf("打开日志文件失败(%s)。\n", argv[4]);
		return -1;
	}
	PActive.AddPInfo(10, "obtcodetodb"); //进程心跳，间隔时间为10秒
	//注意：调试程序时，可以启用如下心跳代码，防止超时（时间太短会被守护进程杀掉）
	// PActive.AddPInfo(1000, "obtcodetodb"); //进程心跳，间隔时间为10秒
	//把全国站点参数文件加载到vstcode容器中
	if (LoadSTCode(argv[1]) == false) {
		return -1;
	}
	logfile.Write("加载参数文件（%s) 成功，站点参数（%d）。\n", argv[1], vstcode.size());
	//连接数据库
	if (conn.connecttodb(argv[2], argv[3]) != 0) {
		logfile.Write("connect database(%s) failed.\n%s\n", argv[2],conn.m_cda.message);
		return -1;
	}
	logfile.Write("connect to database ok.\n", argv[2]);
	//准备插入表的sql语句
	struct st_stcode stcode; //存储站点参数的结构体(为什么加struct？)
	sqlstatement stmtins(&conn); //创建执行插入sql语句的对象
	stmtins.prepare("insert into T_ZHOBTCODE(obtid,cityname,provname,lat,lon,height,upttime) values(:1,:2,:3,:4*100,:5*100,:6*10,now())");
	stmtins.bindin(1, stcode.obtid,10);
	stmtins.bindin(2, stcode.cityname, 30);
	stmtins.bindin(3, stcode.provname, 30);
	stmtins.bindin(4, stcode.lat, 10);
	stmtins.bindin(5, stcode.lon, 10);
	stmtins.bindin(6, stcode.height, 10);
	
	//准备更新表的sql语句
	sqlstatement stmtupt(&conn); //创建执行更新sql语句的对象
	stmtupt.prepare("update T_ZHOBTCODE set cityname=:1,provname=:2,lat=:3*100,lon=:4*100,height=:5*10,upttime=now() where obtid=:6");
	stmtupt.bindin(1, stcode.cityname, 30);
	stmtupt.bindin(2, stcode.provname, 30);
	stmtupt.bindin(3, stcode.lat, 10);
	stmtupt.bindin(4, stcode.lon, 10);
	stmtupt.bindin(5, stcode.height, 10);
	stmtupt.bindin(6, stcode.obtid, 10);
	int inscount = 0; //插入记录数初始化为0
	int uptcount = 0; //更新记录数初始化为0
	CTimer Timer; //计时器，方便记录处理时长

	//遍历vstcode容器
	for (int i = 0; i < vstcode.size(); ++i) {
		//从容器中取出一条记录放到结构体stcode中
		memcpy(&stcode, &vstcode[i],sizeof(struct st_stcode));
		//执行插入的sql语句
		if (stmtins.execute() != 0) {
			if (stmtins.m_cda.rc == 1062) {
				//如果记录已存在，就执行更新的sql语句（通过sql语句插入的返回结果判断-1062）
				if (stmtupt.execute() != 0) {
					logfile.Write("stmtupt.execute() failed.\n%s\n%s\n", stmtupt.m_sql, stmtupt.m_cda.message);
				}
				else {
					uptcount++;
				}
			}
			else {
				logfile.Write("stmtins.execute() failed.\n%s\n%s\n", stmtins.m_sql, stmtins.m_cda.message);
			}
		}
		else {
			inscount++;
		}
	}
	//把总记录数，插入记录数，更新记录数，小号时长记录日志
	logfile.Write("总记录数=%d,插入=%d,更新=%d,耗时=%.2f秒。\n", vstcode.size(), inscount, uptcount, Timer.Elapsed());
	//提交事务
	conn.commit();
	return 0;

}
bool LoadSTCode(const char* inifile) {
	CFile File; //文件操作类（产生的临时文件会在调用exit()函数时调用该类的析构函数进行清理）
	//打开站点参数文件
	if (File.Open(inifile, "r") == false) //只读方式打开文件
	{
		logfile.Write("File.Open(%s) failed.\n", inifile);
		return false;
	}
	char strBuffer[301];
	CCmdStr CmdStr;
	struct st_stcode stcode;
	while (true) {
		//从站点参数文件中读取一行，如果已读取完，跳出循环
	   //memset(strBuffer,0,sizeof(strBuffer));//字符串变量每次使用时最好初始化，否则容易有bug
		if (File.Fgets(strBuffer, 300, true) == false)
			break;
		//logfile.Write("=%s=\n",strBuffer);//将读取出来的每一行数据写入日志文件
		//把读取到的一行拆分
	   /*开发框架中用于拆分有分隔符字符串的类CCmdStr*/
		CmdStr.SplitToCmd(strBuffer, ",", true);//调用CCmdStr类的成员函数SplitToCmd拆分字符串,分隔符时“，”,第三个参数为true表示删除空格
		if (CmdStr.CmdCount() != 6)
			continue;//扔掉无效的行
		  //把站点参数的每个数据项保存到站点参数结构体中
		CmdStr.GetValue(0, stcode.provname, 30); //省
		CmdStr.GetValue(1, stcode.obtid, 10); //站号
		CmdStr.GetValue(2, stcode.cityname, 30); //站名
		CmdStr.GetValue(3, stcode.lat,10);  //纬度
		CmdStr.GetValue(4, stcode.lon,10);  //经度
		CmdStr.GetValue(5, stcode.height,10); //海拔高度
		 //把站点参数结构体放入站点参数容器
		vstcode.push_back(stcode);
	}
	//关闭文件---析构函数自动关闭
	 /*测试代码
	for (int ii=0;ii<vstcode.size();++ii)
	{
		 logfile.Write("provname=%s,obtid=%s,obtname=%s,lat=%.2f,lon=%.2f,height=%.2f\n",vstcode[ii].provname,vstcode[ii].pobtid,vstcode[ii].obtname,vstcode[ii].lat,vstcode[ii].lon,vstcode[ii].height);
	 }
*/
	return true;
}
void EXIT(int sig) { //信号处理函数
	logfile.Write("程序退出，sig=%d\n\n", sig);
	conn.disconnect(); //断开数据库连接
	exit(0);
}
/*
class CTimer  // 这是一个精确到微秒的计时器。
{
private:
public:
    struct timeval m_start;   // 开始计时的时间。
    struct timeval m_end;     // 计时完成的时间。

    // 开始计时。
    void Start();
    CTimer();  // 构造函数中会调用Start方法。

    // 计算已逝去的时间，单位：秒，小数点后面是微秒。
    // 每调用一次本方法之后，自动调用Start方法重新开始计时。
    double Elapsed();
};
*/
