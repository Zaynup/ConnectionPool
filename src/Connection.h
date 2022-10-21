#pragma once

#include <string>
#include <mysql/mysql.h>
#include <ctime>

#include "public.h"

class Connection
{
public:
    // 初始化数据库连接
    Connection();

    // 释放数据库连接资源
    ~Connection();

    // 连接数据库
    bool connect(std::string ip, unsigned short port,
                 std::string user, std::string password,
                 std::string dbname);

    // 更新操作 insert、delete、update
    bool update(std::string sql);

    // 查询操作 select
    MYSQL_RES *query(std::string sql);

    // 刷新一下连接的起始的空闲时间点
    void refreshAliveTime() { aliveTime_ = clock(); }

    // 返回存活的时间
    std::clock_t getAliveTime() const { return clock() - aliveTime_; }

private:
    MYSQL *conn_;            // 表示和MySQL Server的一条连接
    std::clock_t aliveTime_; // 记录进入空闲状态后的起始时间
};