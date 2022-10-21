# aux_source_directory(. SRC_LIST)
set(SRC_LIST
    Connection.cc
    ConnectionPool.cc)
add_library(connpool ${SRC_LIST})
target_link_libraries(connpool mysqlclient pthread)