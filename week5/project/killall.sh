###########################################################
# 停止数据中心后台服务程序的脚本
###########################################################

#终止调度程序
killall -9 procctl
#通知服务程序退出
killall gzipfiles crtsurfdata deletefiles

sleep 3

#让全部的服务程序强制退出
killall -9 gzipfiles crtsurfdata deletefiles
