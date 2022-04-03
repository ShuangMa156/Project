#include "_ftp.h"
Cftp ftp;
int main()
{
	if (ftp.login("192.168.174.10:21", "another", "ftpNeed") == false) { //登录用户another，执行用户msh，实现在一台虚拟机上两个用户之间通过ftp通信
		printf("ftp.login(192.168.174.10:21) failed.\n");
		return -1;
	}
	/*bool login(const char* host, const char* username, const char* password, const int imode = FTPLIB_PASSIVE);//登录ftp服务器。
	  host：ftp服务器ip地址和端口，中间用":"分隔，如"192.168.1.1:21"。
	  username：登录ftp服务器用户名。
	  password：登录ftp服务器的密码。
	  imode：传输模式，1-FTPLIB_PASSIVE是被动模式，2-FTPLIB_PORT是主动模式，缺省是被动模式。
	*/
	printf("ftp.login(192.168.174.10) ok.\n");
	if (ftp.mtime("/home/another/ftptest/public/_public.cpp") == false) {
		printf("ftp.mtime(/home/another/ftptest/public/_public.cpp) failed.\n");
		return -1;
	}
	printf("ftp.mtime(/home/another/ftptest/public/_public.cpp) ok,mtime=%s.\n", ftp.m_mtime);
	/* bool mtime(const char* remotefilename);//获取ftp服务器上文件的时间。
	   remotefilename：待获取的文件名。
	   返回值：false-失败；true-成功，获取到的文件时间存放在m_mtime成员变量中。
	*/
	if (ftp.size("/home/another/ftptest/public/_public.cpp") == false) {
		printf("ftp.mtime(/home/another/ftptest/public/_public.cpp) failed.\n");
		return -1;
	}
	printf("ftp.size(/home/another/ftptest/public/_public.cpp) ok,size=%d.\n", ftp.m_size);
	/*bool size(const char* remotefilename);//获取ftp服务器上文件的大小。
	  remotefilename：待获取的文件名。
	  返回值：false-失败；true-成功，获取到的文件大小存放在m_size成员变量中。
	*/
	if (ftp.nlist("/home/another/ftptest/public", "/tmp/ftptest/rlist.lst") == false) {
		printf("ftp.nlist(/home/another/ftptest/public) failed.\n");
		return -1;
	}
	printf("ftp.nlist(home/another/ftptest/public) ok.\n");
	/*bool nlist(const char* remotedir, const char* listfilename);//从ftp服务器上获取文件(发送NLST命令只列出ftp服务器目录中的子目录名（子目录下的文件不会被列出）和文件名)
	   remotefilename：待获取ftp服务器上的文件名。
	   localfilename：保存到本地的文件名。
	   bCheckMTime：文件传输完成后，是否核对远程文件传输前后的时间，保证文件的完整性。
	   返回值：true-成功；false-失败。
	   注意：文件在传输的过程中，采用临时文件命名的方法，即在localfilename后加".tmp"，在传输完成后才正式改为localfilename。
	*/
	//下载
	if (ftp.get("/home/another/ftptest/public/_public.cpp", "/tmp/ftptest/_public.cpp.bak", true) == false) {
		printf("ftp.get() failed.\n");
		return -1;
	}
	printf("ftp.get() ok.\n");
	//上传
	if (ftp.put("/home/msh/ftpclient.cpp", "/home/another/ftptest/ftpclient.cpp.bak", true) == false) {
		printf("ftp.put() failed.\n");
		return -1;
	}
	printf("ftp.put() ok.\n");
	ftp.logout();
	//bool logout();// 注销。
	return 0;
}
/*编译指令：g++ -g -o ftpclient ftpclient.cpp /weather/project/public/_ftp.cpp /weather/project/public/_public.cpp -I/weather/project/public -L/weather/project/public -lftp -lm -lc
                    (编译后的指令)(编译的文件) (头文件的路径)                     (头文件的路径)                       (头文件的搜索目录)         (库文件的搜索目录)       (链接库ftp)
g++ -g -o ftpclient ftpclient.cpp /weather/project/public/_ftp.cpp /weather/project/public/_public.cpp -I/weather/project/public -L/weather/project/public -lftp -lm -lc -Wl,-rpath="/weather/project/public"
*/
