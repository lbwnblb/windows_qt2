#include <windows.h>
#include <stdio.h>

// ============================================
// 最简单的版本：不需要DLL注入
// 直接从外部发送消息给 qt_01.exe 的窗口
// ============================================

// 登录按钮在窗口客户区内的坐标（千牛工作台登录界面）
#define LOGIN_BTN_X  680
#define LOGIN_BTN_Y  470

// 通过PID查找主窗口
struct FindWindowData {
    DWORD pid;
    HWND hwnd;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    FindWindowData* data = (FindWindowData*)lParam;
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == data->pid && IsWindowVisible(hwnd)) {
        char title[256];
        GetWindowTextA(hwnd, title, 256);
        if (strlen(title) > 0) {  // 只要有标题的窗口
            data->hwnd = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

// 查找千牛工作台的窗口
HWND FindQianniuWindow() {
    // 方法1: 直接按标题找 "千牛工作台"
    HWND hwnd = FindWindowW(NULL, L"\u5343\u725b\u5de5\u4f5c\u53f0");
    if (hwnd) return hwnd;

    // 方法1b: 试试 "千牛"
    hwnd = FindWindowW(NULL, L"\u5343\u725b");
    if (hwnd) return hwnd;

    // 方法2: 遍历所有窗口，找千牛进程（AliWorkbench.exe 或 QianNiu.exe）
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return NULL;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    DWORD targetPid = 0;

    const char* processNames[] = { "AliWorkbench.exe", "QianNiu.exe", "千牛.exe", NULL };

    if (Process32First(hSnap, &pe)) {
        do {
            for (int i = 0; processNames[i]; i++) {
                if (_stricmp(pe.szExeFile, processNames[i]) == 0) {
                    targetPid = pe.th32ProcessID;
                    break;
                }
            }
            if (targetPid) break;
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);

    if (!targetPid) return NULL;

    FindWindowData data = { targetPid, NULL };
    EnumWindows(EnumWindowsProc, (LPARAM)&data);
    return data.hwnd;
}

void PrintWindowInfo(HWND hwnd) {
    char title[256];
    GetWindowTextA(hwnd, title, 256);

    RECT rect;
    GetClientRect(hwnd, &rect);

    printf("窗口标题: %s\n", title);
    printf("窗口句柄: 0x%p\n", hwnd);
    printf("客户区大小: %ld x %ld\n", rect.right, rect.bottom);
}

int main() {
    printf("=== 千牛工作台 登录点击工具 (无需注入) ===\n\n");

    // 查找窗口
    HWND hwnd = FindQianniuWindow();
    if (!hwnd) {
        printf("找不到千牛工作台窗口!\n");
        printf("请先启动千牛工作台\n");
        system("pause");
        return 1;
    }

    PrintWindowInfo(hwnd);
    printf("\n");

    // ========================================
    // 功能1: 点击登录按钮
    // ========================================
    printf("[1] 发送登录按钮点击消息...\n");
    printf("    坐标: (%d, %d)\n", LOGIN_BTN_X, LOGIN_BTN_Y);

    LPARAM pos = MAKELPARAM(LOGIN_BTN_X, LOGIN_BTN_Y);

    // 发送鼠标按下
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, pos);
    Sleep(50);
    // 发送鼠标抬起
    PostMessage(hwnd, WM_LBUTTONUP, 0, pos);

    printf("    已发送!\n\n");

    // ========================================
    // 功能2: 也可以先设置用户名密码再点击
    // ========================================
    printf("如果需要先输入用户名密码，可以用以下方式:\n");
    printf("  1. 手动在界面输入用户名密码\n");
    printf("  2. 然后运行本程序点击登录\n");
    printf("\n");
    printf("或者用 SendMessage + WM_SETTEXT 设置输入框内容\n");
    printf("（需要先找到输入框的子窗口句柄）\n");

    system("pause");
    return 0;
}