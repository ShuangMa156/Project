/*
* 程序名称：idcapp.cpp,此程序是数据中心项目公共函数和类的实现文件
*/
#include "idcapp.h"
CZHOBTMIND::CZHOBTMIND() {
	m_conn = 0;
	m_logfile = 0;
}
CZHOBTMIND::CZHOBTMIND(connection* conn, CLogFile* logfile) {
	m_conn = conn;
	m_logfile = logfile;
}
void CZHOBTMIND::BindConnLog(connection* conn, CLogFile* logfile) {
	m_conn = conn;
	m_logfile = logfile;
}
bool CZHOBTMIND::SplitBuffer(char* strBuffer,bool bisxml) {
	memset(&m_zhobtmind, 0, sizeof(struct st_zhobtmind)); //初始化结构体
	if (bisxml == true) {
		GetXMLBuffer(strBuffer, "obtid", m_zhobtmind.obtid, 10);
		GetXMLBuffer(strBuffer, "ddatetime", m_zhobtmind.ddatetime, 14);
		char tmp[11];
		GetXMLBuffer(strBuffer, "t", tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.t, 10, "%d", (int)(atof(tmp) * 10));
		}
		GetXMLBuffer(strBuffer, "p", tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.p, 10, "%d", (int)(atof(tmp) * 10));
		}
		GetXMLBuffer(strBuffer, "u", m_zhobtmind.u, 10);
		GetXMLBuffer(strBuffer, "wd", m_zhobtmind.wd, 10);
		GetXMLBuffer(strBuffer, "wf", tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.wf, 10, "%d", (int)(atof(tmp) * 10));
		}
		GetXMLBuffer(strBuffer, "r", tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.r, 10, "%d", (int)(atof(tmp) * 10));
		}
		GetXMLBuffer(strBuffer, "vis", tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.vis, 10, "%d", (int)(atof(tmp) * 10));
		}
	}
	else {
		CCmdStr CmdStr;
		CmdStr.SplitToCmd(strBuffer, ",");
		CmdStr.GetValue(0, m_zhobtmind.obtid, 10);
		CmdStr.GetValue(1, m_zhobtmind.ddatetime, 14);
		char tmp[11];
		CmdStr.GetValue(2, tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.t, 10, "%d", (int)(atof(tmp) * 10));
		}
		CmdStr.GetValue(3,tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.p, 10, "%d", (int)(atof(tmp) * 10));
		}
		CmdStr.GetValue(4, m_zhobtmind.u, 10);
		CmdStr.GetValue(5,m_zhobtmind.wd, 10);
		CmdStr.GetValue(6,tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.wf, 10, "%d", (int)(atof(tmp) * 10));
		}
		CmdStr.GetValue(7,tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.r, 10, "%d", (int)(atof(tmp) * 10));
		}
		CmdStr.GetValue(8,tmp, 10);
		if (strlen(tmp) > 0) {
			snprintf(m_zhobtmind.vis, 10, "%d", (int)(atof(tmp) * 10));
		}
	}
	STRCPY(m_buffer, sizeof(m_buffer), strBuffer); //将strBuffer中的内容拷贝到m_buffer中
	return true;
}
bool CZHOBTMIND::InsertTable() {
	if (m_stmt.m_state == 0) { //未绑定数据库连接对象
		m_stmt.connect(m_conn); //绑定数据库连接对象
		m_stmt.prepare("insert into T_ZHOBTMIND(obtid,ddatetime,t,p,u,wd,wf,r,vis) value(:1,str_to_date(:2,'%%Y%%m%%d%%H%%i%%s'),:3,:4,:5,:6,:7,:8,:9)");
		m_stmt.bindin(1, m_zhobtmind.obtid, 10);
		m_stmt.bindin(2, m_zhobtmind.ddatetime, 14);
		m_stmt.bindin(3, m_zhobtmind.t, 10);
		m_stmt.bindin(4, m_zhobtmind.p, 10);
		m_stmt.bindin(5, m_zhobtmind.u, 10);
		m_stmt.bindin(6, m_zhobtmind.wd, 10);
		m_stmt.bindin(7, m_zhobtmind.wf, 10);
		m_stmt.bindin(8, m_zhobtmind.r, 10);
		m_stmt.bindin(9, m_zhobtmind.vis, 10);
	}
	if (m_stmt.execute()!= 0) {
		//1、失败的情况有哪些？1-记录重复（不用写日志，继续执行程序），2-数据内容非法 （记录非法内容，程序继续处理）
		// 是否全部的失败都要写日志？
		//2、如果失败了怎么办？程序是否需要继续？是否要rollback?是否返回false?
		if (m_stmt.m_cda.rc != 1062) { //失败原因不是记录重复时，记录失败的原因
			m_logfile->Write("Buffer=%s\n", m_buffer);
			m_logfile->Write("stmt.execute() failed.\n%s\n%s\n", m_stmt.m_sql, m_stmt.m_cda.message);

		}
		return false;
	}
	return true;
}
