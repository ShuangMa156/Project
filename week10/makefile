# 并发框架文件头路径
PUBINCL = -I/weather/project/public
# 并发框架cpp文件名，直接包含进来，没有采用链接库，是为了方便调试
PUBCPP = /weather/project/public/_public.cpp
# mysql的头文件存放目录
MYSQLINCL = -I/usr/local/mysql/include -I/weather/project/public/db/mysql
# mysql库文件保存路径
MYSQLLIB = -L/usr/local/mysql/lib
# mysql的链接库
MYSQLLIBS= -lmysqlclient
# 开发框架中的mysql的cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试
MYSQLCPP= /weather/project/public/db/mysql/_mysql.cpp
#编译参数
CFLAGS = -g
 
all:crtsurfdata1 crtsurfdata2 crtsurfdata3 crtsurfdata4 crtsurfdata obtcodetodb obtmindtodb

crtsurfdata1:crtsurfdata1.cpp
	g++ $(CFLAGS) -o crtsurfdata1 crtsurfdata1.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata1 ../bin/.
crtsurfdata2 : crtsurfdata2.cpp
	g++ $(CFLAGS) -o crtsurfdata2 crtsurfdata2.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata2 ../bin/.
crtsurfdata3 : crtsurfdata3.cpp
	g++ $(CFLAGS) -o crtsurfdata3 crtsurfdata3.cpp $(PUBINCL)$(PUBCPP) -lm -lc
	cp crtsurfdata3 ../bin/.
crtsurfdata4 : crtsurfdata4.cpp
	g++ $(CFLAGS) -o crtsurfdata4 crtsurfdata4.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata4 ../bin/.
crtsurfdata : crtsurfdata.cpp
	g++ $(CFLAGS) -o crtsurfdata crtsurfdata.cpp $(PUBINCL) $(PUBCPP) -lm -lc
	cp crtsurfdata ../bin/.
obtcodetodb:obtcodetodb.cpp
	g++ $(CFLAGS) -o obtcodetodb obtcodetodb.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -lm -lc
	cp obtcodetodb ../bin/.
obtmindtodb:obtmindtodb.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) -o obtmindtodb obtmindtodb.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) idcapp.cpp -lm -lc
	cp obtmindtodb ../bin/.
clean：
     rm crtsurfdata1 crtsurfdata2 crtsurfdata3 crtsurfdata4 crtsurfdata obtcodetodb obtmindtodb
