#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <chrono>
#include <condition_variable>

#include "Connection.h"

class ConnectionPool
{
public:
    static ConnectionPool *GetInstance();

    // 给外部提供接口，从连接池中获取一个可用的空闲连接
    std::shared_ptr<Connection> getConnection();

private:
    ConnectionPool();

    //从配置文件中加载配置项
    bool loadConfigFile();

    void produceConnectionTasks();

    void scannerConnectionTasks();

private:
    std::string ip_;       // mysql的ip地址
    unsigned int port_;    // mysql的端口号 3306
    std::string username_; // mysql登录用户名
    std::string password_; // mysql登录密码
    std::string dbname_;   // mysql数据库名称

    int initSize_;          // 连接池的初始连接量
    int maxSize_;           // 连接池的最大连接量
    int maxIdletime_;       // 连接池的最大空闲时间
    int connectionTimeout_; // 连接池获取连接的超时时间

    std::queue<Connection *> connectionQue_; // 存储mysql连接的队列
    std::mutex queMutex_;                    // 维护连接队列的线程安全的互斥锁
    std::atomic_int connectionCnt_;          // 记录连接所创建的connection连接的总数量
    std::condition_variable cv_;             // 设置条件变量，用于连接生产线程和消费线程的通信
};