###########################################################
# 启动数据中心后台服务程序的脚本
###########################################################

#检查服务程序是否超时，配置在/etc/rc/local中，由root用户执行
#/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/checkproc

#压缩数据中心后台服务程序的备份日志
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/gzipfiles /log/idc "*.log.20*" 0.04

#生成用于测试的全国气象站点观测的分钟数据
/weather/project/tools1/bin/procctl 60 /weather/project/idc1/bin/crtsurfdata /weather/project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv

#清理原始的全国气象站点观测的分钟数据目录/tmp/idc/surfdata中的历史文件
/weather/project/tools1/bin/procctl 300 /weather/project/tools1/bin/deletefiles /tmp/idc/surfdata "*" 0.04

#采集全国气象站点观测的分钟数据
/weather/project/tools1/bin/procctl 30 /weather/project/tools1/bin/ftpgetfiles /log/idc/ftpgetfiles_surfdata.log "<host>127.0.0.1:21</host><mode>1</mode><username>another</username><password>ftpNeed</password><localpath>/idcdata/surfdata</localpath><remotepath>/tmp/idc/surfdata</remotepath><matchname>SURF_ZH*.XML</matchname><listfilename>/idcdata/ftplist/ftpgetfiles_surfdata.list</listfilename><ptype>1</ptype><okfilename>/idcdata/ftplist/ftpgetfiles_surfdata.xml</okfilename><checkmtime>true</checkmtime><timeout>80</timeout><pname>ftpgetfiles_surfdata</pname>"
