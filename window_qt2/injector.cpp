#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

// 通过进程名找到PID
DWORD FindProcessId(const char* processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (Process32First(hSnap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName) == 0) {
                CloseHandle(hSnap);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return 0;
}

// DLL注入
BOOL InjectDll(DWORD pid, const char* dllPath) {
    // 1. 打开目标进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        printf("OpenProcess 失败! 错误码: %lu\n", GetLastError());
        printf("提示: 请以管理员身份运行\n");
        return FALSE;
    }

    // 2. 在目标进程分配内存，写入DLL路径
    size_t pathLen = strlen(dllPath) + 1;
    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, pathLen, 
                                       MEM_COMMIT | MEM_RESERVE, 
                                       PAGE_READWRITE);
    if (!remoteMem) {
        printf("VirtualAllocEx 失败!\n");
        CloseHandle(hProcess);
        return FALSE;
    }

    WriteProcessMemory(hProcess, remoteMem, dllPath, pathLen, NULL);

    // 3. 获取 LoadLibraryA 地址
    FARPROC loadLib = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    // 4. 创建远程线程执行 LoadLibraryA(dllPath)
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, 
                                         (LPTHREAD_START_ROUTINE)loadLib, 
                                         remoteMem, 0, NULL);
    if (!hThread) {
        printf("CreateRemoteThread 失败! 错误码: %lu\n", GetLastError());
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // 5. 等待注入完成
    WaitForSingleObject(hThread, INFINITE);

    // 6. 清理
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}

int main(int argc, char* argv[]) {
    printf("=== Qt01 登录按钮点击工具 ===\n\n");

    // 查找 qt_01.exe 进程
    DWORD pid = FindProcessId("qt_01.exe");
    if (!pid) {
        printf("找不到 qt_01.exe 进程!\n");
        printf("请先启动 Qt01 程序\n");
        system("pause");
        return 1;
    }
    printf("找到 qt_01.exe, PID: %lu\n", pid);

    // DLL路径（改成你的实际路径）
    char dllPath[MAX_PATH];
    GetFullPathNameA("click_login.dll", MAX_PATH, dllPath, NULL);
    printf("DLL路径: %s\n", dllPath);

    // 注入
    printf("正在注入...\n");
    if (InjectDll(pid, dllPath)) {
        printf("注入成功! DLL已加载到目标进程\n");
    } else {
        printf("注入失败!\n");
    }

    system("pause");
    return 0;
}
