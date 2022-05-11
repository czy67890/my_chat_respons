file(GLOB chatserver ./*.cc)
add_library(chat_lib ${chatserver})
target_link_libraries(chat_lib muduo_base muduo_net)