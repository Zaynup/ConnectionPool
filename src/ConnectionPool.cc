#include "ConnectionPool.h"

// 线程安全的懒汉单例函数接口
ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool pool;
    return &pool;
}

// 从配置文件中加载配置
void ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("mysql.conf", "r");
    if (pf == nullptr)
    {
        LOG("mysql.conf fie is not exist!");
        return;
    }
}
