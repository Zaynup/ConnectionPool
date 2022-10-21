#include "ConnectionPool.h"
#include "public.h"

// 线程安全的懒汉单例函数接口
ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool pool; // lock和unlock
    return &pool;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("mysql.conf", "r");
    if (pf == nullptr)
    {
        LOG("mysql.ini file is not exist!");
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

// 连接池的构造
ConnectionPool::ConnectionPool()
{
    // 加载配置项了
    if (!loadConfigFile())
    {
        return;
    }

    // 创建初始数量的连接
    for (int i = 0; i < initSize_; ++i)
    {
        Connection *p = new Connection();
        p->connect(ip_, port_, username_, password_, dbname_);
        p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
        connectionQue_.push(p);
        connectionCnt_++;
    }

    // 启动一个新的线程，作为连接的生产者 linux thread => pthread_create
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTasks, this));
    produce.detach();

    // 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTasks, this));
    scanner.detach();
}

// 运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTasks()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(queMutex_);
        while (!connectionQue_.empty())
        {
            cv_.wait(lock); // 队列不空，此处生产线程进入等待状态
        }

        // 连接数量没有到达上限，继续创建新的连接
        if (connectionCnt_ < maxSize_)
        {
            Connection *p = new Connection();
            p->connect(ip_, port_, username_, password_, dbname_);
            p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
            connectionQue_.push(p);
            connectionCnt_++;
        }

        // 通知消费者线程，可以消费连接了
        cv_.notify_all();
    }
}

// 给外部提供接口，从连接池中获取一个可用的空闲连接
std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(queMutex_);
    while (connectionQue_.empty())
    {
        // sleep
        if (std::cv_status::timeout == cv_.wait_for(lock, std::chrono::milliseconds(connectionTimeout_)))
        {
            if (connectionQue_.empty())
            {
                LOG("获取空闲连接超时了...获取连接失败!");
                return nullptr;
            }
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
                                       pcon->refreshAliveTime(); // 刷新一下开始空闲的起始时间
                                       connectionQue_.push(pcon);
                                   });

    connectionQue_.pop();
    cv_.notify_all(); // 消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接

    return sp;
}

// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
void ConnectionPool::scannerConnectionTasks()
{
    for (;;)
    {
        // 通过sleep模拟定时效果
        std::this_thread::sleep_for(std::chrono::seconds(maxIdletime_));

        // 扫描整个队列，释放多余的连接
        std::unique_lock<std::mutex> lock(queMutex_);
        while (connectionCnt_ > initSize_)
        {
            Connection *p = connectionQue_.front();
            if (p->getAliveTime() >= (maxIdletime_ * 1000))
            {
                connectionQue_.pop();
                connectionCnt_--;
                delete p; // 调用~Connection()释放连接
            }
            else
            {
                break; // 队头的连接没有超过_maxIdleTime，其它连接肯定没有
            }
        }
    }
}