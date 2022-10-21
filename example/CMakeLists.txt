set(SRC_LIST
    main.cc)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(main ${SRC_LIST})

target_link_libraries(main connpool)