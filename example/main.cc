#include <iostream>
#include <memory>

#include "Connection.h"
#include "ConnectionPool.h"

void test_conn()
{
    Connection conn;
    conn.connect("127.0.0.1", 3306, "root", "123456", "connpool");
    char sql1[1024] = {0};
    sprintf(sql1, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");
    conn.update(sql1);
}

void test_workbench_unusePool()
{
    std::clock_t begin = clock();

    for (int i = 0; i < 1000; ++i)
    {
        Connection conn;
        conn.connect("127.0.0.1", 3306, "root", "123456", "connpool");
        char sql[1024] = {0};
        sprintf(sql, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");
        conn.update(sql);
    }
    std::clock_t end = clock();

    std::cout << "花费时间：" << (end - begin) << "ms" << std::endl; // 643320ms
}

void test_workbench_usePool()
{
    std::clock_t begin = clock();
    ConnectionPool *pool = ConnectionPool::GetInstance();
    for (int i = 0; i < 1000; ++i)
    {
        std::shared_ptr<Connection> sp = pool->getConnection();
        sp->connect("127.0.0.1", 3306, "root", "123456", "connpool");

        char sql[1024] = {0};
        sprintf(sql, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");

        sp->update(sql);
    }
    std::clock_t end = clock();

    std::cout << "花费时间：" << (end - begin) << "ms" << std::endl; // 71337ms
}

void test_workbench_usePoolAndMultiThread()
{
    std::clock_t begin = clock();

    std::thread t1([]()
                   {
        ConnectionPool *pool = ConnectionPool::GetInstance();
        for (int i = 0; i < 250; ++i)
        {
            std::shared_ptr<Connection> sp = pool->getConnection();
            sp->connect("127.0.0.1", 3306, "root", "123456", "connpool");

            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");

            sp->update(sql);
        } });

    std::thread t2([]()
                   {
        ConnectionPool *pool = ConnectionPool::GetInstance();
        for (int i = 0; i < 250; ++i)
        {
            std::shared_ptr<Connection> sp = pool->getConnection();
            sp->connect("127.0.0.1", 3306, "root", "123456", "connpool");

            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");

            sp->update(sql);
        } });

    std::thread t3([]()
                   {
        ConnectionPool *pool = ConnectionPool::GetInstance();
        for (int i = 0; i < 250; ++i)
        {
            std::shared_ptr<Connection> sp = pool->getConnection();
            sp->connect("127.0.0.1", 3306, "root", "123456", "connpool");

            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");

            sp->update(sql);
        } });

    std::thread t4([]()
                   {
        ConnectionPool *pool = ConnectionPool::GetInstance();
        for (int i = 0; i < 250; ++i)
        {
            std::shared_ptr<Connection> sp = pool->getConnection();
            sp->connect("127.0.0.1", 3306, "root", "123456", "connpool");

            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");

            sp->update(sql);
        } });

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    std::clock_t end = clock();

    std::cout << "花费时间：" << (end - begin) << "ms" << std::endl; // 71337ms
}

int main()
{
    // test_workbench_unusePool();
    // test_workbench_usePool();
    test_workbench_usePoolAndMultiThread();

    return 0;
}
