#include "ConnectionPool.h"

// 线程安全的懒汉单例函数接口
ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool pool;
    return &pool;
}
