/*程序名：gzipfiles.cpp
* 作者：马双
* 程序功能：压缩历史文件
*/
#include "_public.h"
void EXIT(int sig);
int main(int argc, char* argv[])
{
	//程序的帮助
	if (argc != 4) {
		printf("\n");
		printf("Using:/weather/project/tools1/bin/gzipfiles pathname matchstr timeout\n\n");
		printf("pathname 指定的扫描目录，即文件所在的目录。\n");
		printf("matchstr 目录中要压缩的文件。\n");
		printf("timeout  超时时间，单位为天。\n");
		printf("Example:/weather/project/tools1/bin/gzipfiles /log/idc \"*.log.20*\" 0.02\n");//匹配的文件名log.20,超时时间为0.02天，“”强调参数，\为转义字符
		printf("        /weather/project/tools1/bin/gzipfiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n");
		printf("        /weather/project/tools1/bin/procctl 300  /weather/project/tools1/bin/gzipfiles /log/idc \"*.log.20*\" 0.02\n"); //由调度程序启动
		printf("        /weather/project/tools1/bin/procctl 300  /weather/project/tools1/bin/gzipfiles /tmp/idc/surfdata \"*.xml,*.json\" 0.01\n\n"); //由调度程序启动
		printf("这是一个工具程序，用于压缩历史数据文件或日志文件。\n");
		printf("本程序把pathname目录及子目录中的与timeout匹配的machstr文件全部压缩，timeout可以是小数。\n");
		printf("本程序不写日志文件，也不会在控制台输出任何信息。\n");
		printf("本程序调用usr/bin/gzip命令压缩文件。\n\n\n");
		return -1;
	}
	//关闭全部的信号和输入、输出
	/*信号处理：
	* （1）关闭全部的信号和输入输出
	* （2）设置信号，在shell状态下可用“kill +进程号”正常终止这些进程,但请不要用“kill -9 +进程号"强行终止
	*/
	//开发阶段先注释，方便调试 CloseIOAndSignal(true); //关闭IO和信号
	signal(SIGINT, EXIT);
	signal(SIGTERM, EXIT);
	//获取文件超时时间点
	char strTimeOut[21];
	LocalTime(strTimeOut, "yyyy-mm-dd hh24:mi:ss", 0 - (int)(atof(argv[3]) * 24 * 60 * 60)); //计算超时时间(往前偏移）,单位为秒
	//打开目录：CDir.OpenDir()
	CDir Dir;
	if (Dir.OpenDir(argv[1], argv[2], 10000, true) == false) { //打开子目录
		printf("Dir.OpenDir(%s) failed.\n", argv[1]);
		return -1;
	}
	/*
	bool CDir::OpenDir(const char *in_DirName,const char *in_MatchStr,const unsigned int in_MaxCount,const bool bAndChild,bool bSort)
    {
        m_pos=0;
        m_vFileName.clear();
        // 如果目录不存在，就创建该目录
        if (MKDIR(in_DirName,false) == false) return false;
        bool bRet=_OpenDir(in_DirName,in_MatchStr,in_MaxCount,bAndChild);
        if (bSort==true)
        {
             sort(m_vFileName.begin(), m_vFileName.end());
        }
        return bRet;
    }
	参数说明：打开目录，获取目录中的文件列表信息，存放于m_vFileName容器中。
    in_DirName，待打开的目录名。
    in_MatchStr，待获取文件名的匹配规则，不匹配的文件被忽略。
    in_MaxCount，获取文件的最大数量，缺省值为10000个。
    bAndChild，是否打开各级子目录，缺省值为false-不打开子目录。
    bSort，是否对获取到的文件列表（即m_vFileName容器中的内容）进行排序，缺省值为false-不排序。
    返回值：如果in_DirName参数指定的目录不存在，OpenDir方法会创建该目录，如果创建失败，返回false，还有，如果当前用户对in_DirName目录下的子目录没有读取权限也会返回false，其它正常情况下都会返回true。
   */
	char strCmd[1024]; //存放gzip压缩文件的命令
	//遍历目录中的文件名
	while (true) { 
		//得到一个文件的信息:CDir.ReadDir()
		if (Dir.ReadDir() == false) { //读取失败
			break;
		}
		//显示文件的详细信息
		//printf("DirName=%s,FileName=%s,FullFileName=%s,FileSize=%d,ModifyTime=%s,CreateTime=%s,AccessTime=%s\n",Dir.m_DirNmae,Dir.m_FileName,Dir.m_FullFileName,Dir.m_FileSize,Dir.m_ModifyTime,Dir.m_CreateTime,Dir.m_AccessTime);
		printf("FullFileName=%s\n", Dir.m_FullFileName); //显示文件全路径
		//与超时时间比较，如果更早，则需要压缩(已经被压缩的文件不需要压缩)
		if ((strcmp(Dir.m_ModifyTime, strTimeOut) < 0) && (MatchStr(Dir.m_FileName, "*.gz") == false)) {
			//压缩文件，调用操作系统的gzip命令
			
			SNPRINTF(strCmd, sizeof(strCmd), 1000, "/usr/bin/gzip -f %s 1>/dev/null 2>/dev/null",Dir.m_FullFileName); //框架中的安全函数，与sprintf()函数相同 1>/dev/null：把标准输出定位到空中 2>/dev/null：把标准错误定位到空中
			if (system(strCmd) == 0) { //调用系统功能成功
				printf("gzip %s ok.\n", Dir.m_FullFileName);
			}
			else {
				printf("gzip %s failed.\n", Dir.m_FullFileName);
			}
		}
	}
	return 0;
}
void EXIT(int sig) {
	printf("程序退出，sig=%d\n\n", sig);
		exit(0);
}
/* 正则表达式，判断一个字符串是否匹配另一个字符串。
   str：需要判断的字符串，精确表示的字符串，如文件名"_public.cpp"。
   rules：匹配规则表达式，用星号"*"表示任意字符串，多个字符串之间用半角的逗号分隔，如"*.h,*.cpp"。
   注意，str参数不支持"*"，rules参数支持"*"，函数在判断str是否匹配rules的时候，会忽略字母的大小写。
bool MatchStr(const string &str,const string &rules)
{
  // 如果用于比较的字符是空的，返回false
  if (rules.size() == 0) return false;

  // 如果被比较的字符串是"*"，返回true
  if (rules == "*") return true;

  int ii,jj;
  int  iPOS1,iPOS2;
  CCmdStr CmdStr,CmdSubStr;

  string strFileName,strMatchStr;

  strFileName=str;
  strMatchStr=rules;

  // 把字符串都转换成大写后再来比较
  ToUpper(strFileName);
  ToUpper(strMatchStr);

  CmdStr.SplitToCmd(strMatchStr,",");

  for (ii=0;ii<CmdStr.CmdCount();ii++)
  {
    // 如果为空，就一定要跳过，否则就会被配上
    if (CmdStr.m_vCmdStr[ii].empty() == true) continue;

    iPOS1=iPOS2=0;
    CmdSubStr.SplitToCmd(CmdStr.m_vCmdStr[ii],"*");

    for (jj=0;jj<CmdSubStr.CmdCount();jj++)
    {
      // 如果是文件名的首部
      if (jj == 0)
      {
        if (strncmp(strFileName.c_str(),CmdSubStr.m_vCmdStr[jj].c_str(),CmdSubStr.m_vCmdStr[jj].size()) != 0) break;
      }

      // 如果是文件名的尾部
      if (jj == CmdSubStr.CmdCount()-1)
      {
        if (strcmp(strFileName.c_str()+strFileName.size()-CmdSubStr.m_vCmdStr[jj].size(),CmdSubStr.m_vCmdStr[jj].c_str()) != 0) break;
      }

      iPOS2=strFileName.find(CmdSubStr.m_vCmdStr[jj],iPOS1);

      if (iPOS2 < 0) break;

      iPOS1=iPOS2+CmdSubStr.m_vCmdStr[jj].size();
    }

    if (jj==CmdSubStr.CmdCount()) return true;
  }

  return false;
}
*/
