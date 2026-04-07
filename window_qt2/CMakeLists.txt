cmake_minimum_required(VERSION 3.10)
project(ClickLogin)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# ==============================
# 方法1: 简单版（推荐先用这个）
# 不需要DLL注入，直接从外部发消息
# ==============================
add_executable(simple_click simple_click.cpp)
target_link_libraries(simple_click PRIVATE user32)

# ==============================
# 方法2: DLL注入版
# ==============================
# 注入用的DLL
add_library(click_login SHARED main.cpp)
target_link_libraries(click_login PRIVATE user32)

# 注入器
add_executable(injector injector.cpp)
target_link_libraries(injector PRIVATE user32)
