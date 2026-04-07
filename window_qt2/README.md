# Qt01 登录按钮点击工具

向 qt_01.exe 的登录窗口发送鼠标坐标消息，模拟点击登录按钮。

## 两种方式

### 方式1: simple_click.exe（推荐，最简单）

不需要DLL注入，直接从外部向 qt_01.exe 窗口发送鼠标消息。

```
编译后直接运行 simple_click.exe
```

### 方式2: DLL注入版

将 click_login.dll 注入到 qt_01.exe 进程内部执行。

```
1. 先启动 qt_01.exe
2. 运行 injector.exe（需要管理员权限）
```

## 编译方法

### 用 Visual Studio 命令行：

```batch
cl /EHsc simple_click.cpp /Fe:simple_click.exe user32.lib
cl /EHsc injector.cpp /Fe:injector.exe user32.lib
cl /LD main.cpp /Fe:click_login.dll user32.lib
```

### 用 CMake + MinGW：

```batch
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### 用 CMake + MSVC：

```batch
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 调整坐标

如果点击位置不对，需要修改代码中的坐标：

```cpp
#define LOGIN_BTN_X  780    // 登录按钮的X坐标
#define LOGIN_BTN_Y  545    // 登录按钮的Y坐标
```

### 如何确定正确坐标：

1. 用 Spy++ (Visual Studio 自带) 的准星工具拖到登录按钮上
2. 或者用截图工具测量按钮在窗口内的像素位置
3. 注意是相对于窗口客户区的坐标，不是屏幕坐标

## 注意事项

- simple_click.exe 一般不需要管理员权限
- injector.exe 需要管理员权限才能注入
- 如果 qt_01.exe 以管理员运行，simple_click.exe 也需要管理员权限
- 坐标需要根据实际窗口大小调整
