#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

// ============================================
// 方法1: PostMessage 发送鼠标坐标消息（DLL注入版）
// ============================================

// 登录按钮在窗口客户区内的坐标（千牛工作台登录界面）
// 根据截图估算：登录按钮在右侧面板中下部
#define LOGIN_BTN_X  680
#define LOGIN_BTN_Y  470

DWORD WINAPI ClickLoginThread(LPVOID param) {
    Sleep(2000);  // 等待2秒，确保DLL加载完成

    // 通过窗口标题查找千牛工作台
    HWND hwnd = FindWindowW(NULL, L"\u5343\u725b\u5de5\u4f5c\u53f0");  // "千牛工作台"

    // 如果找不到，尝试其他可能的标题
    if (!hwnd) {
        hwnd = FindWindowW(NULL, L"\u5343\u725b");  // "千牛"
    }
    if (!hwnd) {
        hwnd = FindWindowA(NULL, "AliWorkbench");  // 英文进程名可能的标题
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