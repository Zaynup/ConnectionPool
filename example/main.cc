#include "Connection.h"
#include <iostream>

int main()
{
    Connection conn;

    conn.connect("127.0.0.1", 3306, "root", "123456", "connpool");

    char sql1[1024] = {0};
    sprintf(sql1, "insert into user(username,age,sex) values ('%s','%d','%s');", "zhangy", 18, "male");

    conn.update(sql1);

    return 0;
}
