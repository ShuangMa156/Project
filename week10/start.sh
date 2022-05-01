###########################################################
# 启动数据中心后台服务程序的脚本
###########################################################

#检查服务程序是否超时，配置在/etc/rc.local中，由root用户执行
#/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/checkproc

#压缩数据中心后台服务程序的备份日志
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/gzipfiles /log/idc "*.log.20*" 0.04

#生成用于测试的全国气象站点观测的分钟数据
/weather/project/tools1/bin/procctl 60 /weather/project/idc1/bin/crtsurfdata /weather/project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv

#清理原始的全国气象站点观测的分钟数据目录/tmp/idc/surfdata中的历史文件
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/deletefiles /tmp/idc/surfdata "*" 0.04

#下载全国气象站点观测的分钟数据的xml文件
/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>msh</username><password>MSos007</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename><checkmtime>true</checkmtime><timeout>80</timeout><pname>ftpgetfiles_surfdata</pname>"

#上传全国气象站气象站点观测分钟数据的xml文件
/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/ftpputfiles /log/idc/ftpputfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>msh</username><password>MSOS007</password><localpath>/tmp/idc/surfdata</localpath><remotepath>/tmp/ftpputest</remotepath><matchname>SURF_ZH*.JSON</matchname><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename><timeout>80</timeout><pname>ftpputfiles_surfdata</pname>"

#清理原始的全国气象站点观测的分钟数据目录/idcdata/surfdata中的历史文件
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/deletefiles /idcdata/surfdata "*" 0.04

#清理原始的全国气象站点观测的分钟数据目录/tmp/ftpputest中的历史文件
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/deletefiles /tmp/ftpputest "*" 0.04

#文件传输的服务端程序
/weather/project/tools1/bin/procctl 10 /weather/projec/tools1/bin/fileserver 5005 /log/idc/fileserver.log

#把目录/tmp/ftpputest中的文件上传到/tmp/tcpputest目录中
/weather/project/tools1/bin/procctl 20 /weather/project/tools1/bin/tcpputfiles /log/idc/tcpputfiles_surfdata.log "<ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><clientpath>/tmp/ftpputest</clientpath><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><srvpath>/tmp/tcpputest</srvpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>"

#把目录/tmp/tcpputest中的文件下载到/tmp/tcpgetest目录中
/weather/project/tools1/bin/procctl 20 /weather/project/tools1/bin/tcpgetfiles /log/idc/tcpgetfiles_surfdata.log "<ip>127.0.0.1</ip><port>5005</port><ptype>1</ptype><srvpath>/tmp/tcpputest</srvpath><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</matchname><clientpath>/tmp/tcpgetest</clientpath><timetvl>10</timetvl><timeout>50</timeout><pname>tcpgetfiles_surfdata</pname>"

#清理采集的全国气象站点观测的分钟数据目录/tmp/tcpgetest中的历史数据文件
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/deletefiles /tmp/tcpgetest "*" 0.04

#把全国气象站点参数数据保存到数据库表中，如果站点不存在则插入，站点已存在则更新
/weather/project/tools1/bin/procctl 120 /weather/project/idc1/bin/obtcodetodb /weather/project/idc1/ini/stcode.ini "127.0.0.1,root,mysql2022,Weather_DataCenter,3306" utf8 /log/idc/obtcodetodb.log

#把全国站点分钟观测数据保存到数据库的T_ZHOBTMIND表中，数据只有插入，不更新
/weather/project/tools1/bin/procctl 120 /weather/project/idc1/bin/obtmindtodb /idcdata/surfdata "127.0.0.1,root,mysql2022,Weather_DataCenter,3306" utf8 /log/idc/obtmindtodb.log

#清理T_ZHOBTMIND表中120分之前的数据，防止磁盘空间被填满
/weather/project/tools1/bin/procctl 120 /weather/project/tools1/bin/execsql /weather/project/idc1/sql/cleardata.sql "127.0.0.1,root,mysql2022,Weather_DataCenter,3306" utf8 /log/idc/execsql.log
