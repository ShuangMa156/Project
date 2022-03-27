/*程序名：crtsurfdata.cpp
* 作者：马双
* 作用：把观测站点数据容器中的内容写入文件，支持csv(逗号分隔的文件格式)、xml、json
* xml:可扩展标记语言，标准通用标记语言的子集，简称“XML”
*     自我描述，简单易懂，容错性和可扩展性非常好
*     是数据处理、数据通讯和数据交换等应用场景的常用数据格式
*     有点浪费空间
* json:将数据封装为对象
*     全称：JavaScript Object Notation ,JavaScript对象表示法
*     是一种轻量级的文本数据交换格式
*     使用JavaScript语法描述数据对象，自我描述，容易理解
*/
/* (1)增加生成历史数据文件的功能，为压缩文件和清理文件模块准备历史数据
* （2）增加信号处理函数，处理信号2和信号15的信息
* （3）解决调用exit()函数退出时局部对象没有调用析构函数的问题-----将用到的类对象定义为全局对象
* （4）把心跳信息写入共享内存
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
CLogFile logfile; //日志类
CFile File; //文件操作类（产生的临时文件会在调用exit()函数时调用该类的析构函数进行清理）
CPActive PACtive; //将进程的心跳信息保存到共享内存中
//存放全国气象站点参数容器
vector<struct st_stcode> vstcode; //思考3：向量容器的使用
//存放观测数据的容器
vector<struct st_surdata> vsurfdata;
char strddatetime[21]; //观测数据的时间
bool LoadSTCode(const char* inifile);//把站点参数文件加载到vstcode容器中
//模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
void CrtSurfData(); //根据一个容器加上随机数生成另外一个容器
bool CrtSurfFile(const char* outpath, const char* datafmt);
void EXIT(int sig); //信号处理（信号2和信号15）函数
int main(int argc, char* argv[]) //思考2：两个参数的含义
{
	if ((argc != 5) && (argc != 6)) { //增加一个可选参数---历史时间（未指定历史时间，用当前时间，指定了历史时间，用历史时间）
		//如果参数非法，给出帮助文档
		printf("Using:./crtsurfdata4 inifile outpath logfile datafmt [datetime]\n"); //其中datetime为可选参数
		printf("Example:/weather/project/idc1/bin/crtsurfdata /weather/project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv\n");
		printf("        /weather/project/idc1/bin/crtsurfdata /weather/project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv 20220327120000\n");
		printf("        /weather/project/tools1/bin/procctl 60 /weather/project/idc1/bin/crtsurfdata /weather/project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv \n\n");
		printf("inifile 全国气象站点参数文件名。\n");
		printf("outpath 全国气象站点数据文件存放的目录。\n");
		printf("logfile 本程序运行的日志文件名。\n");
		printf("datafmt 生成数据文件的格式，支持xml,json和csv三种格式，中间用逗号分隔。\n");
		printf("datetime 这是一个可选参数，表示生成指定时间的数据和文件。\n\n");
		return -1;
	}
	/*信号处理：
	* （1）关闭全部的信号和输入输出
	* （2）设置信号，在shell状态下可用“kill +进程号”正常终止这些进程,但请不要用“kill -9 +进程号"强行终止
	*/
	CloseIOAndSignal(true); //关闭IO和信号
	signal(SIGINT, EXIT);
	signal(SIGTERM, EXIT);
	//打开程序的日志文件
	if (logfile.Open(argv[3], "a+", false) == false) { //如果日志文件打开失败
		printf("logfile.Open( %s) failed.\n", argv[3]); //输出文件打开失败的提示信息
		return -1;
	}
	logfile.Write("crtsurfdata 开始运行。\n"); //成功打开后写入提示信息
	if (LoadSTCode(argv[1]) == false) {
		return -1;
	}
	PACtive.AddPInfo(20, "crtsurfdata"); //设置超时时间和进程名称（由于程序运行时间很短，所以不用更新心跳时间）
	//获取当前时间，当做观测时间
	memset(strddatetime, 0, sizeof(strddatetime)); //初始化字符串
	if (argc == 5) {
		LocalTime(strddatetime, "yyyymmddhh24miss"); //调用函数获取操作系统的时间
	}
	else {
		STRCPY(strddatetime, sizeof(strddatetime), argv[5]); //字符串拷贝
	}
	//模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
	CrtSurfData();
	//把容器vsurfdata中的全国气象站点分钟观测数据写入文件
	if (strstr(argv[4], "xml") != 0) { //如果在第四个参数中包括“xml”字符，就生成xml文件
		CrtSurfFile(argv[2], "xml");
	}
	if (strstr(argv[4], "json") != 0) { //如果在第四个参数中包括“xml”字符，就生成xml文件
		CrtSurfFile(argv[2], "json");
	}
	if (strstr(argv[4], "csv") != 0) { //如果在第四个参数中包括“xml”字符，就生成xml文件
		CrtSurfFile(argv[2], "csv");
	}
	logfile.Write("crtsurfdata 运行结束。\n");
	return 0;
}
bool LoadSTCode(const char* inifile) {
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
void CrtSurfData() { //由随机数生成观测数据 ，注意时间操作
	//播随机数种子————采用当前时间
	srand(time(0));//获取当前时间，作为观测数据的时间
	
	struct st_surdata stsurfdata;
	//遍历站点参数容器
	for (unsigned int i = 0; i < vstcode.size(); i++) {
		//用随机数填充分钟观测数据的结构体
		memset(&stsurfdata, 0, sizeof(stsurfdata)); //初始化结构体变量
		strncpy(stsurfdata.obtid, vstcode[i].obtid, 10); //思考4:strncpy函数
		strncpy(stsurfdata.ddatetime, strddatetime, 14); //用获取的时间
		stsurfdata.t = rand() % 351; //
		stsurfdata.p = rand() % 265 + 10000;
		stsurfdata.u = rand() % 100 + 1;
		stsurfdata.wd = rand() % 360;
		stsurfdata.wf = rand() % 150;
		stsurfdata.r = rand() % 16;
		stsurfdata.vis = rand() % 5001 + 100000;
		//把观测数据的结构体放入vsurfdata容器
		vsurfdata.push_back(stsurfdata);
	}
	//gdb调式
	//printf("aaa\h"); //设置断点
}
bool CrtSurfFile(const char* outpath, const char* datafmt) {
	//拼接生成数据的文件名，例如：、/tmp/surfdata/SURF_ZH_20210629092200_2254.csv   
	/*1、关注文件目录的创建（当文件目录不存在时，文件会创建失败）
	* 2、如何向文件中写入数据-----
	   （1）创建临时文件
	   （2）往临时文件中写入数据
	   （3）关闭临时文件
	   （4）把临时文件名改为正式文件
	   避免在文件写入的过程中被读取的问题
	CFile类：
	bool Open(const char *filename,const char *openmode,bool bEnBuffer=true);//打开文件时如果文件的目录不存在就会创建目录
	bool OpenForRename(const char *filename,const char *openmode,bool bEnBuffer=true);//打开filename后加.tmp的临时文件，所以openmode只能是‘a’、‘a+’、‘w’、‘w+’
	bool CloseAndRename();//关闭文件指针，并把OpenForRename方法打开的临时文件名重命名为filename
	*/
	CFile File;
	char strFileName[301]; //文件的绝对路径名
	sprintf(strFileName, "%s/SURF_ZH_%s_%d.%s", outpath, strddatetime, getpid(), datafmt); //getpid()用于获取进程编号
	//打开少年print文件,
	if (File.OpenForRename(strFileName, "w") == false) {
		logfile.Write("File.OpenForRename(%s) failed。\n", strFileName);
		return false;
	}
	//写入第一行的标题(只有数据文件的格式为CVS时，才需要写入标题)
	if (strcmp(datafmt, "csv") == 0) {
		File.Fprintf("站点代码,数据时间,气温,气压,相对湿度,风向,风速,降雨量,能见度\n");
	}
	if (strcmp(datafmt, "xml") == 0) {
		File.Fprintf("<data>\n");
	}
	if (strcmp(datafmt, "json") == 0) {
		File.Fprintf("{\"data\"}:[\n"); //对象名的双引号需要转义
	}
	//遍历存放数据的容器，将数据写入文件
	for (unsigned int i = 0; i < vsurfdata.size(); ++i) {
		//写入第一条记录
		if (strcmp(datafmt, "csv") == 0) { //文件中要求温度、压强、风速、降雨量、能见度为浮点数
			File.Fprintf("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n", vsurfdata[i].obtid, vsurfdata[i].ddatetime, vsurfdata[i].t / 10.0, \
				vsurfdata[i].p / 10.0, vsurfdata[i].u, vsurfdata[i].wd, vsurfdata[i].wf / 10.0, vsurfdata[i].r / 10.0, vsurfdata[i].vis / 10.0);
		}
		if (strcmp(datafmt, "xml") == 0) { //文件中要求温度、压强、风速、降雨量、能见度为浮点数
			File.Fprintf("<obtid>%s</obtid><ddatetome>%s</ddatetome><t>%.1f</t><p>%.1f</p><u>%d</u><wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/>\n",
				vsurfdata[i].obtid, vsurfdata[i].ddatetime, vsurfdata[i].t / 10.0, vsurfdata[i].p / 10.0, vsurfdata[i].u, \
				vsurfdata[i].wd, vsurfdata[i].wf / 10.0, vsurfdata[i].r / 10.0, vsurfdata[i].vis / 10.0);
		}
		if (strcmp(datafmt, "json") == 0) { //文件中要求温度、压强、风速、降雨量、能见度为浮点数
			File.Fprintf("{\"obtid\":\"%s\", \"ddatetime\":\"%s\",\"t\":\"%.1f\",\"p\":\"%.1f\",\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\",\"vis\":\"%.1f\"}",
				vsurfdata[i].obtid, vsurfdata[i].ddatetime, vsurfdata[i].t / 10.0, vsurfdata[i].p / 10.0, \
				vsurfdata[i].u, vsurfdata[i].wd, vsurfdata[i].wf / 10.0, vsurfdata[i].r / 10.0, vsurfdata[i].vis / 10.0);
			if (i < vsurfdata.size() - 1) {
				File.Fprintf(",\n");
			}
			else {
				File.Fprintf("\n");
			} //每个数据用便签指示分隔
		}
	}
	if (strcmp(datafmt, "xml") == 0) {
		File.Fprintf("</data>\n"); //数据写完后加上结束标签
	}
	if (strcmp(datafmt, "json") == 0) {
		File.Fprintf("]}\n"); //数据写完后加上结束标签
	}
	//关闭文件
	File.CloseAndRename();
	UTime(strFileName, strddatetime); // 修改文件的时间属性
	logfile.Write("生成数据文件%s成功。数据时间%s,记录数%d。\n", strFileName, strddatetime, vsurfdata.size());
	return true;
}
void EXIT(int sig) { //信号处理函数
	logfile.Write("程序退出，sig=%d\n\n", sig);
	exit(0);
}
