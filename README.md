# `cfrp` - 内网端口映射工具

`cfrp`是一款内网穿透工具并且采用了`c`语言编写,类似于`nginx`的反向代理服务.  
服务端分别开启两个端口, 一个用于`cfrp`客户端连接,一个用于外部访问, 每次外部访问的同时用于与客户端交互的端口发送一个创建新连接的口令,客户端收到口令后发起对服务端的连接,客户端在连接服务端的同时并创建一个连接,到要被映射本地端口的连接,最终达到数据转发功能

## 目标功能

1. 基本转发功能
2. 掉线重连
3. tcp 长连接
4. 多连接
5. 多映射
6. 分包发送
7. 会话认证

## 其他

[已实现版本](https://github.com/editso/cfrp/tags)
