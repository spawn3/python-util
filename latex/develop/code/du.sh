# 检查磁盘分区
df -lh

# 检查目录所占容量
TMP='/';for i in $(/bin/ls $TMP); do du -sh $TMP/$i; done
