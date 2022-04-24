/*
* 程序名称：selecttable.cpp，此程序用于演示开发框架操作MySQL数据库（查询表中的记录）
* 作者：马双
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
	//定义用于存放超女信息的结构体，与表中的字段对应（名称最好相同，不容易出错，字段的长度最好也相同）
	struct st_girls
	{
		long id; //超女编号
		char name[31]; // 超女姓名，长度比数据库中多一个，因为有一个默认的\0结束
		double weight; //超女体重
		char btime[20]; //报名时间
	} stgirls;
	int iminid, imaxid; // 查询条件中的最下id和最大id
	//准备查询表的sql语句,参数用“:+序号”，序号要连续（从左到右，依次增大）（查询的结果会生成一个结果集）
	//stmt.prepare("select id,name,weight,date_format(btime,'%%Y-%%m-%%d %%H:%%i:%%s') from girls where id in (2,3,4)"); // 对%进行转义
	//采用传递参数的方法书写sql语句
	stmt.prepare("select id,name,weight,date_format(btime,'%%Y-%%m-%%d %%H:%%i:%%s') from girls where id>=:1 and id<=:2"); // 对%进行转义
	/*注意事项：
	* 1、如果SQL语句的主体没有改变，只需要prepare()一次就可以
	* 2、结果集中的字段，调用bindout()绑定变量的地址
	* 3、bindout()方法的返回值固定为0，不用判断返回值
	* 4、如果SQL语句的主体已经改变，prepare()后，需重新用bindout()绑定变量
	* 5、调用execute()方法执行SQL语句，然后再循环调用next()方法回去结果集中的记录
	* 6、每调用一次next()方法，从结果集中获取一条记录，字段内容保存已绑定的变量中
	*/
	//绑定输入变量的地址
	stmt.bindin(1, &iminid); //绑定最小id
	stmt.bindin(2, &imaxid); //绑定最大id
	//绑定输出变量的地址
	stmt.bindout(1, &stgirls.id);
	stmt.bindout(2, stgirls.name, 30);
	stmt.bindout(3, &stgirls.weight);
	stmt.bindout(4, stgirls.btime, 19);

	iminid = 1; //指定待查询记录的最小id值
	imaxid = 3; //指定待查询记录的最大id值
	//执行SQL语句，一定要判断返回值：0-成功，1-失败
	// 失败代码在stmt.m_cda,rc中，失败描述在stmt.m_cda.message中
	if (stmt.execute()!=0) {
		printf("stmt.execute() failed.\n%s\n%s\n", stmt.m_sql, stmt.m_cda.message);
		return -1;
	}
	//本程序执行的是查询语句，执行stmt.execute()后，将会在数据库的缓冲区中产生一个结果集
	while (true) {
		memset(&stgirls, 0, sizeof(struct st_girls)); //初始化结构体
		//从结果集中获取一条记录，一定要判断返回值：0-成功，1403-无记录，其他-失败
		//在实际开发中，除了0和1403，其他的情况极少出现
		if (stmt.next()!=0) { 
			break;
		}
		//把获取到的记录打印出来
		printf("id=%ld,name=%s,weight=%.02f,btime=%s\n", stgirls.id, stgirls.name, stgirls.weight, stgirls.btime);
	}
	//请注意，stmt.m_cda.rpc变量非常重要，它保存了SQL被执行后影响的记录数
	printf("本次查询了girls表中%ld条记录\n",stmt.m_cda.rpc);
	return 0;
}
/*
* 	int next(); // 从结果集中获取一条记录。
	如果执行的SQL语句是查询语句，调用execute方法后，会产生一个结果集（存放在数据库的缓冲区中）。
	next方法从结果集中获取一条记录，把字段的值放入已绑定的输出变量中。
	返回值：0-成功，1403-结果集已无记录，其它-失败，失败的代码在m_cda.rc中，失败的描述在m_cda.message中。
	返回失败的原因主要有两种：1）与数据库的连接已断开；2）绑定输出变量的内存太小。
	每执行一次next方法，m_cda.rpc的值加1。
	程序中必须检查next方法的返回值。
*/
