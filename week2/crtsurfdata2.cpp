
/*程序名：crtsurfdata2.cpp 本程序用于将全国气象站点参数文件加载到站点参数的容器中
全国气象站点参数文件的表头如下：
省      站号   站名    纬度     经度      海拔高度
*/
#include "_public.h"
struct st_stcode
{
     char provname[31]; //省
     char obtid[11];   //站号
     char obtname[31];//站名
     double lat; //纬度
     double lon; //经度
     double height; //高度

};
vector<struct st_stcode> vstcode; //存放站点参数的容器
bool LoadSTCode(const char *inifile);
//该函数完成把气象站点参数文件加载到vstcode容器中，参数为文件名
CLogFile logfile;
int main(int argc,char *argv[])
{
    if (argc!=4) {
      //如果参数非法，给出帮助文档
     printf("Using:./crtsurfdata2 inifile outpath logfile\n");
     printf("Example:/project/idc1/bin/crtsurfdata2 /project/idc1/ini/stcode.ini /tmp/surfdata /log/idc/crtsurfdatd2.log\n\n");
     printf("inifile 全国气象站点参数文件名。\n");
     printf("outpath 全国气象站点参数文件目录。\n");
     printf("logfile 本程序运行的日志文件名.\n\n");
     return -1;//输入参数不正确时程序退出
    }
if (logfile.Open(argv[3]==false)) { //日志文件打开失败
      printf("logfile.Open(%s) filed.\n",argv[3]);
      return -1;
    }
    logfile.Write("crtsurfdata2 开始运行。\n");
    if (LoadSTCode(argv[1]==false)) //判断站点参数文件加载是否成功
        return -1;
    logfile.Write("crtsurfdata2 运行结束。\n");
    return 0;
}
//该函数完成把气象站点参数文件加载到vstcode容器中，参数为文件名
bool LoadSTCode(const char *inifile)
{
    CFile File;
    //打开站点参数文件
    if (File.Open(inifile,"r")==false) //只读方式打开文件
    {
        logfile.Write("File.Open(%s) failed.\n",inifile);
        return false;
    }
    char strBuffer[301];
    CCmdStr CmdStr;
    struct st_stcode stcode;
    while (true) {
    //从站点参数文件中读取一行，如果已读取完，跳出循环
   // memset(strBuffer,0,sizeof(strBuffer));//字符串变量每次使用时最好初始化，否则容易有bug
    if (CFile.Fgets(strBuffer,300,true)==false)
      break;
    //logfile.Write("=%s=\n",strBuffer);//将读取出来的每一行数据写入日志文件
    //把读取到的一行拆分
   /*开发框架中用于拆分有分隔符字符串的类CCmdStr*/
   CmdStr.SplitToCmd(strBuffer,",",true);//调用CCmdStr类的成员函数SplitToCmd拆分字符串,分隔符时“，”,第三个参数为true表示删除空格
   if (CmdStr.CmdCount()!=6) 
      continue;//扔掉无效的行
    //把站点参数的每个数据项保存到站点参数结构体中
    CmdStr.GetValue(0,stcode.provname,30); //省
   CmdStr.GetValue(1,stcode.obtid,10); //站号
   CmdStr.GetValue(2,stcode.obtname,30); //站名
   CmdStr.GetValue(3,&stcode.lat);  //纬度
   CmdStr.GetValue(4,&stcode.lon);  //经度
   CmdStr.GetValue(5,&stcode.height); //海拔高度
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
    //filename:待打开的文件名，建议采用绝对路径的文件名
    //openmode:打开文件的模式，与fopen库函数的打开模式相同
    //bEnBuffer:是否启用缓冲，true-启用，false-不启用，缺省时启用
    bool CloseAndRemove();//关闭文件指针，并删除文件
    bool Fgets(char *buffer,const int readsize,bool bdelcrt=false);//从文件中读取一行
    //buffer：用于存放读取的内容，buffer必须大于readsize+1，否则可能会造成读到的数据不完整或内存的溢出
    //readsize：本次打算读取的字节数，如果已读到结束标志"\n"，函数返回
    //bdelcrt:是否删除行结束标志“\r”和"\n"，true-删除，false-不删除，缺省值时false
    //返回值：true-成功，false-失败，一般情况下，失败可以认为是文件已结束
    
};
*/
