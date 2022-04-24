/*
* 程序名：updatetable.cpp,此程序用于演示开发框架操作Mysql数据库（修改表中的记录）
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
	//准备修改表的sql语句,参数用“:+序号”，序号要连续（从左到右，依次增大）
	stmt.prepare("update girls set name=:1,weight=:2,btime=str_to_date(:3,'%%Y-%%m-%%d %%H:%%i:%%s') where id=:4"); // 对%进行转义
	/*注意事项：
	* 1、参数的序号从1开始，连续、递增，参数也可以用问号表示，但是问号的兼容性不好，不建议
	* 2、SQL语句中的右值才能作为参数，表名、字段名、关键字、函数名等都不能作为参数
	* 3、参数可以参与运算或用于函数的参数
	* 4、如果SQL语句的主体没有改变，只需要prepare()一次就可以
	* 5、SQL语句中的每个参数必须调用bindin()绑定变量的地址
	* 6、如果SQL语句的主体已改变，prepare()后，需要重新用bindin()绑定变量
	* 7、prepare()方法有返回值，一般不用检查，如果SQL语句有问题，调用execute()方法时能发现
	* 8、bindin()方法的返回值固定为0，不用判断返回值
	* 9、prepare()和bindin()之后，每调用一次execute()，就执行一次SQL语句，SQL语句的数据来自被绑定的变量值
	*/
	stmt.bindin(1, stgirls.name, 30);
	stmt.bindin(2, &stgirls.weight);
	stmt.bindin(3, stgirls.btime, 19);
	stmt.bindin(4, &stgirls.id);
	//模拟超女数据，修改超女信息表中的全部记录
	for (int i = 0; i < 5; ++i) {
		memset(&stgirls, 0, sizeof(struct st_girls)); //初始化结构体变量
		//为结构体成员变量赋值
		stgirls.id = i + 1; //超女编号
		sprintf(stgirls.name, "貂蝉%05dgirl", i + 1); //超女姓名
		stgirls.weight =i+48.39; //超女体重
		sprintf(stgirls.btime, "2022-04-24 11:30:%02d", i); //报名时间
		if (stmt.execute() != 0) {
			printf("stmt.execute() failed.\n %s\n%d\n%s\n", stmt.m_sql, stmt.m_cda.rc, stmt.m_cda.message);
		}
		printf("成功插入了%d条记录。\n", stmt.m_cda.rpc); //stmt.m_cda.rpc是本次执行sql语句影响的记录数
	}
	printf("update table girls ok.\n");
	conn.commit(); //提交数据库事务
	return 0;
}
