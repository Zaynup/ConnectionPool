#include "ConnectionPool.h"

ConnectionPool::ConnectionPool()
{
    // 加载配置项
    if (!loadConfigFile())
    {
        LOG("配置项加载失败！");
        return;
    }

    // 创建初始连接的数量
    for (int i = 0; i < initSize_; ++i)
    {
        Connection *pConn = new Connection();
        pConn->connect(ip_, port_, username_, password_, dbname_);
        connectionQue_.push(pConn);
        connectionCnt_++;
    }

    // 启动一个新的线程，作为连接的生产者
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTasks, this));
}

// 线程安全的懒汉单例函数接口
ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool pool;
    return &pool;
}

// 给外部提供接口，从连接池中获取一个可用的空闲连接
std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(queMutex_);
    if (connectionQue_.empty())
    {
        cv_.wait_for(lock, std::chrono::microseconds(connectionTimeout_));
        if (connectionQue_.empty())
        {
            LOG("获取空闲连接超时。。。");
            return nullptr;
        }
    }

    /*
    shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于
    调用connection的析构函数，connection就被close掉了。
    这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue当中
    */
    std::shared_ptr<Connection> sp(connectionQue_.front(),
                                   [&](Connection *pcon)
                                   {
                                       // 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
                                       std::unique_lock<std::mutex> lock(queMutex_);
                                       connectionQue_.push(pcon);
                                   });
    connectionQue_.pop();
    cv_.notify_all();
    return sp;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("mysql.conf", "r");
    if (pf == nullptr)
    {
        LOG("mysql.conf fie is not exist!");
        return false;
    }

    while (!feof(pf))
    {
        char line[1024] = {0};
        fgets(line, 1024, pf);
        std::string str = line;
        int idx = str.find('=', 0);
        if (idx == -1) // 无效的配置项
        {
            continue;
        }

        // password=123456\n
        int endidx = str.find('\n', idx);
        std::string key = str.substr(0, idx);
        std::string value = str.substr(idx + 1, endidx - idx - 1);

        if (key == "ip")
        {
            ip_ = value;
        }
        else if (key == "port")
        {
            port_ = atoi(value.c_str());
        }
        else if (key == "username")
        {
            username_ = value;
        }
        else if (key == "password")
        {
            password_ = value;
        }
        else if (key == "dbname")
        {
            dbname_ = value;
        }
        else if (key == "initSize")
        {
            initSize_ = atoi(value.c_str());
        }
        else if (key == "maxSize")
        {
            maxSize_ = atoi(value.c_str());
        }
        else if (key == "maxIdleTime")
        {
            maxIdletime_ = atoi(value.c_str());
        }
        else if (key == "connectionTimeOut")
        {
            connectionTimeout_ = atoi(value.c_str());
        }
    }
    return true;
}

// 运行在独立的线程中，专门负责生产新的连接
void ConnectionPool::produceConnectionTasks()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(queMutex_);
        while (!connectionQue_.empty())
        {
            cv_.wait(lock); // 队列不空，此处生产线程进入等待状态
        }

        // 连接数量没有到达上线，继续创建新的连接
        if (connectionCnt_ < maxSize_)
        {
            Connection *pConn = new Connection();
            pConn->connect(ip_, port_, username_, password_, dbname_);
            connectionQue_.push(pConn);
            connectionCnt_++;
        }

        // 通知消费者线程，可以消费连接了
        cv_.notify_all();
    }
}

//
void ConnectionPool::scannerConnectionTasks()
{
}
