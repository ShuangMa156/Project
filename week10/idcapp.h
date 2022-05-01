/*
* 程序名称：idcapp.h，此程序时数据中心项目公用函数和类的声明文件
* 作者：马双
*/
#ifndef IDCAPP_H
#define IDCAPP_H
#include "_public.h"
#include "_mysql.h"
struct st_zhobtmind { //用字符串存放整数和浮点数，便于存放空值
	char obtid[11]; //站点代码
	char ddatetime[21]; //数据时间，精确到分钟
	char t[11]; //温度，单位：0.1摄氏度
	char p[11]; //气压，单位：0.1百帕
	char u[11]; //相对湿度，0~100之间的值
	char wd[11]; //风向，0~360之间的值
	char wf[11]; //风速，单位：0.1m/s
	char r[11]; //降雨量，0.1mm
	char vis[11]; //能见度，0.1米
};
class CZHOBTMIND //全国站点分钟观测数据操作类
{
public:
	connection* m_conn; //数据库连接指针
	CLogFile* m_logfile; //日志文件指针
	sqlstatement m_stmt; //插入表操作的sql
	char m_buffer[1024]; //存储从文件中读取的一行数据的缓冲区
	struct st_zhobtmind m_zhobtmind; //全国站点分钟观测数据的结构体
	CZHOBTMIND(); //无参构造函数
	CZHOBTMIND(connection* conn, CLogFile* logfile); //有参构造函数
	~CZHOBTMIND() {} //析构函数
	void BindConnLog(connection* conn, CLogFile* logfile); //把connection和CLogFile的地址传到类里面
	bool SplitBuffer(char* strBuffer,bool bisxml); //把从文件读到的一行数据放到结构体中
	bool InsertTable(); //把结构中的数据插入到数据库的表T_ZHOBTMIND中
};
#endif

