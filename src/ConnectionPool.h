#pragma once
#include <string>
#include <queue>
#include <mutex>

#include "Connection.h"

class ConnectionPool
{
public:
    static ConnectionPool *GetInstance();

private:
    ConnectionPool() {}

    // 从配置文件中加载配置
    void loadConfigFile();

private:
    std::string ip_;       // mysql的ip地址
    unsigned int port_;    // mysql的端口号 3306
    std::string username_; // mysql登录用户名
    std::string password_; // mysql登录密码

    int initSize_;          // 连接池的初始连接量
    int maxSize_;           // 连接池的最大连接量
    int maxIdletime_;       // 连接池的最大空闲时间
    int connectionTimeout_; // 连接池获取连接的超时时间

    std::queue<Connection *> connectionQue_; // 存储mysql连接的队列
    std::mutex queMutex_;                    // 维护连接队列的线程安全的互斥锁
};