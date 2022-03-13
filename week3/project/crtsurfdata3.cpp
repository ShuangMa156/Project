/*程序名：crtsurfdata3.cpp
* 作者：马双
* 作用：遍历站点参数容器，生成每个站点的观测数据，存放在站点观测数据容器中
*/
#include "_public.h"
#include<vector>
//全国气象站点参数结构体
struct st_stcode //思考1：结构体的内存分布
{
	char provname[31]; //省
	char obtid[11]; //站号
	char obname[31]; //站名
	double lat; //维度
	double lon; //精度
	double height; //海拔高度
};
//全国气象站点分钟观测数据结构体
struct st_surdata
{
	char obtid[11]; //站点代码
	char ddatetime[21]; //数据时间：格式为yyyymmddhh24miss
	int t; //气温，单位：0.1摄氏度
	int p; //气压：0.1百帕
	int u; //相对湿度：0-100之间的值
	int wd; //风向：0-360之间的值
	int wf; //风速：单位0.1m/s
	int r; //降雨量：0.1mm
	int vis; //能见度：0.1米
};
//存放全国气象站点参数容器
vector<struct st_stcode> vstcode; //思考3：向量容器的使用
//存放观测数据的容器
vector<struct st_surdata> vsurfdata;
//把站点参数文件加载到vstcode容器中
bool LoadSTCode(const char* inifile);
bool LoadSTCode(const char* inifile) {
	CFile File;
	/*
    CFile类
    class CFile
    {
    private:
	    FILE *m_fp; //文件指针
	    bool m_bEnBuffer;//是否启用缓冲，true-启用缓冲，false=不启用缓冲，缺省时启用
	    char m_fiename[301];//文件名，建议采用绝对路径的文件名
	    char m_filenametmp[301];//临时文件名，在m_filename后加“。tmp”
    public:
	    CFile();//构造函数
	    bool IsOpened();//判断文件是否已经打开，返回值：true-已打开。false-未打开
	    bool Open(const char *filename,const char *openmode,bool bEnBuffer=true);//打开文件
	        filename:待打开的文件名，建议采用绝对路径的文件名
	        openmode:打开文件的模式，与fopen库函数的打开模式相同
	        bEnBuffer:是否启用缓冲，true-启用，false-不启用，缺省时启用
	bool CloseAndRemove();//关闭文件指针，并删除文件
	bool Fgets(char *buffer,const int readsize,bool bdelcrt=false);//从文件中读取一行
	     buffer：用于存放读取的内容，buffer必须大于readsize+1，否则可能会造成读到的数据不完整或内存的溢出
	     readsize：本次打算读取的字节数，如果已读到结束标志"\n"，函数返回
	     bdelcrt:是否删除行结束标志“\r”和"\n"，true-删除，false-不删除，缺省值时false
	     返回值：true-成功，false-失败，一般情况下，失败可以认为是文件已结束

    };
    */
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
		CmdStr.GetValue(2, stcode.obname, 30); //站名
		CmdStr.GetValue(3, &stcode.lat);  //纬度
		CmdStr.GetValue(4, &stcode.lon);  //经度
		CmdStr.GetValue(5, &stcode.height); //海拔高度
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
//模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
void CrtSurfData(); //根据一个容器加上随机数生成另外一个容器
void CrtSurfData() { //由随机数生成观测数据 ，注意时间操作
	//播随机数种子————采用当前时间
	srand(time(0));
	//获取当前时间，作为观测数据的时间
	/*函数void LocalTime(char *stime,const *fmt=0,const int timevl=0)
	stime:用于存放获取到的时间字符串
	timevl:是啊金的偏移量，单位：秒
	     0：缺省值，表示当前时间
		 30：表示当前时间30秒之后的时间点
		 -30：表示当前时间30秒之前的时间点
	fmt:输出时间的格式
	    yyyy-年份，mm-月份,dd-日期，hh24-小时,mi-分钟，ss-秒
		缺省值：yyyy-mm-dd hh24:mi:ss
		支持的格式：yyyymmddhh24miss、yyyy-mm-dd、yyyymmdd、hh24:mi:ss、hh24miss、hh24:mi、hh24mi、hh24、mi
		注意：1、小时的表示方法时hh24,与数据库的时间表示保持一致
		     2、timetostr函数中定义了时间的格式
			 3、调用函数的时候，如果fmt与上述格式都匹配，stime的内存将为空
			 4、时间的年份是4位，其他时间参数不足两位时在前面补0
	*/
	char strddatetime[21];
	memset(strddatetime, 0, sizeof(strddatetime)); //初始化字符串
	LocalTime(strddatetime, "yyyymmddhh24miss"); //调用函数获取操作系统的时间
	struct st_surdata stsurfdata;
	//遍历站点参数容器
	for (int i = 0; i < vstcode.size(); i++) {
		//用随机数填充分钟观测数据的结构体
		memset(&stsurfdata, 0, sizeof(stsurfdata)); //初始化结构体变量
		strncpy(stsurfdata.obtid, vstcode[i].obtid, 10); //思考4:strncpy函数
		strncpy(stsurfdata.ddatetime, strddatetime, 14); //用获取的时间
		stsurfdata.t=rand() % 351; //
		stsurfdata.p = rand() % 265+10000;
		stsurfdata.u = rand() % 100+1;
		stsurfdata.wd = rand() % 360;
		stsurfdata.wf = rand() % 150;
		stsurfdata.r = rand() % 16;
		stsurfdata.vis = rand() % 5001+100000;
		//把观测数据的结构体放入vsurfdata容器
		vsurfdata.push_back(stsurfdata);
	}
	//gdb调式
	//printf("aaa\h"); //设置断点
}
CLogFile logfile; //日志类
int main(int argc, char* argv[]) //思考2：两个参数的含义
{
	if (argc != 4) {
		//如果参数非法，给出帮助文档
		printf("Using:/crtsurfdata3 inifile outpath logfile\n");
		printf("Example:/projrct/idc1/bin/crtsurfdata3 /project/idc1/ini/stcode.ini /tmp/surfdata /log/idc/crtsurfdata3.log\n\n");
		printf("inifile 全国气象站点参数文件名。\n");
		printf("outpath 全国气象站点数据文件存放的目录。\n");
		printf("logfile 本程序运行的日志文件名。\n");
		return -1;
	}
	//打开程序的日志文件
	if (logfile.Open(argv[3], "a+", false) == false) { //如果日志文件打开失败
		printf("logfile.Open( %s) failed.\n", argv[3]); //输出文件打开失败的提示信息
		return -1;
	}
	logfile.Write("crtsurfdata3 开始运行。\n"); //成功打开后写入提示信息
	if (LoadSTCode(argv[1]) == false) {
		return -1;
	}
	//模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
	CrtSurfData();
	logfile.WriteEx("crtsurfdata3 运行结束。\n");
	return 0;
}
/*时间操作*/
