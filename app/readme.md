# smart__home 项目

## 项目描述

- 智能家居小网关,主要负责转发解析订阅到的mqtt消息到串口,消息体采用json格式,并将433模块传回来的数据以json格式发布到mqtt.

- 实现倒计时执行任务,单位为一分钟,比如十分钟后开灯等.

- 实现指定时间执行任务,比如周一8:00浇花.

## 项目结构

- 项目代码放在app文件下,app/include下是自己定义的头文件,可更改MQTTECHO.c头文件中的mqtt相关clientid,username等匹配相应的mqtt服务器.还可以更改led.h匹配不同的gpio口

- 项目根目录的include文件夹下包含的是乐鑫sdk的头文件和源文件,为方便开发,将原来在项目根目录的third_party移至项目根目录include下

## 目前具有的功能

- [x] smartconfig功能
- [x] mqtt稳定发送订阅消息
- [x] 使用uart发送接收433M协议模块命令
- [x] 使用cjson解析通过mqtt订阅的消息
- [x] 倒计时执行任务
- [x] 指定时间执行任务
- [ ] 循环指定时间执行任务

## 待解决的问题

mqtt连接异常时的处理,比如断线了之后的重连,暂时使用的是重启的方式,此方式不可运用到实际产品中,

准备