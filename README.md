# miniTAP
mini traffic analysis plantform for graduation project



## Windows

```shell
cd build
cmake -G "MinGW Makefiles" ..
cmake -G "Unix Makefiles" ..
cmake --build .
```

是否采用clickhouse存取通过common.h中的_CLICKHOUSE_STORE_开关

flushcache中的burst统计部分还没有修改，因为要flushcache所以必须建立一个全局的统计
改了是否clickhouse存储的宏，添加了flushcache中对于burst的处理

为了signal函数中可以调用，把recorder变成了全局的变量
然后测试出finish中有问题，存在段错误
经过调试发现是traverse中没有改是否用clickhouse保存，导致保存进未打开的文件发生段错误，遂解决