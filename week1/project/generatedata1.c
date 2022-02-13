/*
程序名：crtsurfdata1.cpp  本程序用于生成全国气象站点观测的分钟数据
*/
#include “_public.h”    //包含开发框架的头文件
CLogFile logfile;//日志文件类(开发框架中的，用于对日志文件进行读写操作)及其对象
int main（int argc,char *argv[]）
{
    //全国气象站点参数文件inifile 生成的测试文件存放的目录outpath 程序运行的日志logfile（后台程序必备）
    //程序有上述三个参数,所有argc的值为4
    if （argc!=4）{
      printf("Using:./crtsurfdata1 inifile outpath logfile\n"); //提示正确的方法
      printf("Example:/project/idc1/bin/crtsurfdata1 /project/idc1/ini/stcode.ini /tmp/surfdata /log/idc/crtsurfdata1.log\n\n"); //示例
      //参数说明
      printf("inifile 全国气象站点参数文件名。\n");
      printf("outpath 全国气象站点数据文件存放的目录。\n");
      printf("logfile 本程序运行的日志文件名.\n\n");
      return -1;//输入参数不正确时程序退出
    }
    if (logfile.Open(argv[3]==false)) { //日志文件打开失败
      printf("logfile.Open(%s) filed.\n",argv[3]);
      return -1;
    }
    logfile.Write("crtsurfdata1 开始运行。\n");
    logfile.Write("crtsurfdata1 运行结束。\n");
    return 0;
}
/*
CLogFile类：
Class CLogFile{
public:
  FILE *m_tracefp;//日志文件指针
  char m_filename[301];//日志文件名，建议采用绝对路径
  char m_openmode[11];//日志文件的打开方式，一般采用a+
  bool m_bEnBuffer;//写入日志时，是否启用操作系统的缓冲机制，缺省不启用
  bool m_bBackup;//是否自动切换，日志文件大小超过m_MaxLogSize将自动切换，缺省启用
  long m_MaxLogSize;//最大日志文件的大小、单位MB,缺省100MB，最小为10MB
  
  CLogFile(const long MaxLogSize=100);//构造函数
  bool Open(const char *filename,const char *openmode=0,bool bBackup=true,bEnBuffer=false);//打开日志文件
  //如果日志文件大于m_MaxLogSize的值，就把当前的日志文件名该位历史日志文件名，再创建新的当前日志文件
  //备份后的文件会在日志文件名后加上日期时间，如/tmp/log/filetodb.log.20200101123025
  //注意，在多进程的程序中，日志文件不可切换，多线程的程序中日志文件可以切换
  bool BackupLogFile();
  //把内容写入日志文件，fmt是可变参数，使用方法与printf库函数相同
  //Write方法会写入当前时间，WriteEx方法不写时间
  bool Write(const char *fmt,...);
  bool WriteEx(const *fmt,...);
  void Close();//关闭日志文件
};
*/
