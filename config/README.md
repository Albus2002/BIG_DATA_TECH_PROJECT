## recordConfig文件参数

1表示开启，0表示关闭，每行以``configName 0/1``表示，不写默认关闭

可用参数如下：

```
//全局级别统计
throughput: 记录throughput，1s级别
activeFlowNum：记录活跃流数量，1s级别
protocolCount：记录网络层、传输层协议的包个数及总带宽
packetSize：记录所有包的大小分布
heavyLittleFlowSize：分别统计大小流的总带宽与总包数
//流级别统计
fiveTupleFlowInfo：统计五元组流中包数量、流大小、流持续时间
heavyLittleFlowInfo：分别统计大小流的流级别的包数量、流持续时间分布
```

