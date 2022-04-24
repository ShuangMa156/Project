/*
* 程序名称：deletetable.cpp
*/
#include "_mysql.h"
int main(int argc, char* argv[])
{
	connection conn; //创建数据库连接对象
	//登录数据库,返回值：0-成功，其他是失败，存放了MySQL的错误代码
	//失败代码在conn.m_cda.rc中，失败描述在conn.m_cda.message中
	if (conn.connecttodb("127.0.0.1,root,mysql2022,demo,3306", "utf8") != 0) {
		printf("connect database failed.\n%s\n", conn.m_cda.message);
		return -1;
	}
	//绑定数据库连接
	sqlstatement stmt(&conn); //操作sql语句的对象
	int iminid, imaxid; //删除记录中的最小id和最大id
	stmt.prepare("delete from girls where id>=:1 and id<=:2");
	stmt.bindin(1, &iminid);
	stmt.bindin(2, &imaxid);
	iminid = 1; //指定待删除记录的最小id
	imaxid = 3; //指定待删除记录的最大id
	if (stmt.execute() != 0) {
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql, stmt.m_cda.message);
		return -1;
	}
	printf("本次删除了girls表的%ld条记录。\n", stmt.m_cda.rpc);
	conn.commit(); //提交数据库事务
	return 0;
}
