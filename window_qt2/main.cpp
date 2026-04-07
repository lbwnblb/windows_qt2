#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

// ============================================
// 方法1: PostMessage 发送鼠标坐标消息（DLL注入版）
// ============================================

// 登录按钮在窗口内的大致坐标（根据你的截图估算）
// 窗口右半部分，登录按钮大约在:
// X: 窗口宽度的 70% 左右（按钮居中在右侧面板）
// Y: 窗口高度的 75% 左右
#define LOGIN_BTN_X  780
#define LOGIN_BTN_Y  545

DWORD WINAPI ClickLoginThread(LPVOID param) {
    Sleep(2000);  // 等待2秒，确保DLL加载完成

    // 方法A: 通过窗口标题查找（改成你的实际窗口标题）
    HWND hwnd = FindWindowA(NULL, "Qt01");  
    
    // 如果找不到，尝试遍历所有窗口
    if (!hwnd) {
        hwnd = FindWindowA(NULL, "登录");  // 试试其他可能的标题
    }
    
    if (!hwnd) {
        // 方法B: 通过进程ID查找窗口
        DWORD pid = GetCurrentProcessId();
        
        struct EnumData {
            DWORD pid;
            HWND hwnd;
        } data = { pid, NULL };
        
        EnumWindows([](HWND h, LPARAM lp) -> BOOL {
            EnumData* d = (EnumData*)lp;
            DWORD wndPid;
            GetWindowThreadProcessId(h, &wndPid);
            if (wndPid == d->pid && IsWindowVisible(h)) {
                d->hwnd = h;
                return FALSE;  // 找到了，停止枚举
            }
            return TRUE;
        }, (LPARAM)&data);
        
        hwnd = data.hwnd;
    }

    if (!hwnd) {
        MessageBoxA(NULL, "找不到目标窗口!", "Error", MB_OK);
        return 0;
    }

    // 获取窗口标题（调试用）
    char title[256];
    GetWindowTextA(hwnd, title, 256);
    
    char msg[512];
    sprintf(msg, "找到窗口: %s\nHWND: 0x%p\n即将点击坐标: (%d, %d)", 
            title, hwnd, LOGIN_BTN_X, LOGIN_BTN_Y);
    MessageBoxA(NULL, msg, "Debug", MB_OK);

    // 发送鼠标点击消息
    LPARAM pos = MAKELPARAM(LOGIN_BTN_X, LOGIN_BTN_Y);
    
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, pos);
    Sleep(50);
    PostMessage(hwnd, WM_LBUTTONUP, 0, pos);

    MessageBoxA(NULL, "点击消息已发送!", "Done", MB_OK);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, ClickLoginThread, NULL, 0, NULL);
    }
    return TRUE;
}
