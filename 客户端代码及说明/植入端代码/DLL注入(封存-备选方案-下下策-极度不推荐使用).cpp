#include <windows.h>
#include <psapi.h>
#include <string>
#include<vector>

#pragma comment(lib,"Psapi.lib")
#pragma comment(linker, "subsystem:windows/entry:mainCRTStartup")

//提升进程访问权限
bool enableDebugPriv()
{
    HANDLE hToken; //句柄
    LUID sedebugnameValue; //本地唯一标识ID
    TOKEN_PRIVILEGES tkp; //存储访问令牌信息的结构体

    //无法获取目标进程的访问令牌
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        return false;
    }

    //无法获取当前系统的特权信息的LUID
    if(!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
    {
        CloseHandle(hToken);
        return false;
    }

    //设置访问令牌的信息
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = sedebugnameValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    //无法根据设置的访问令牌调整进程的权限
    if(!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
    {
        CloseHandle(hToken);
        return false;
    }
    return true;
}


//根据传入的进程名称取得进程ID，如果有多个运行实例则返回第一个枚举出来的进程ID
std::vector<DWORD> getAppropriateProcessList()
{
    std::vector<DWORD> processList;

    DWORD processId[1024], cbNeeded, dwProcessesCount; //用于存放进程ID, 读取到的字节数, 进程数量
    HANDLE hProcess; //进程句柄
    HMODULE hMod; //(进程的某一)模块地址?

    char szProcessName[MAX_PATH] = "UnknownProcess"; //存放进程名称
    DWORD dwArrayInBytes = sizeof(processId) * sizeof(DWORD); //进程列表缓存的最大字节数

    //未能获取到进程(EnumProcess用于枚举进程，直到缓冲区已满或者枚举完所有的进程为止?)
    if(!EnumProcesses(processId, dwArrayInBytes, &cbNeeded))
    {
        return processList;
    }

    dwProcessesCount = cbNeeded / sizeof(DWORD); //计算获取到的进程数(数组中的元素个数)

    enableDebugPriv(); //提升当前进程额访问权限？

    for(UINT i = 0; i < dwProcessesCount; i++) //遍历进程列表
    {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId[i]); //打开当前进程
        
        if(hProcess) //未能打开当前进程
        {
            processList.push_back(processId[i]);
            CloseHandle(hProcess);
        }
    }
    return processList;
}

//自动选取进程注入DLL
bool InsertDll(std::string DllPath)
{
    const char * szFileName = DllPath.c_str();

    //提升当前进程的访问权限
    if(!enableDebugPriv()) { return false; }
    //获取进程列表
    std::vector<DWORD> processList = getAppropriateProcessList();
    if(processList.size() == 0) { return false; }
    //打开进程
    HANDLE hTargetProcess = NULL;
    for(int i = 0; i < processList.size(); i++)
    {
        hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processList[i]);
        if(hTargetProcess) { break; }
    }
    if(!hTargetProcess) { system("pause"); return false; }

    //计算DLL文件名称占用的存储空间
    int memorySize = (DllPath.length() + 1) * sizeof(char);

    //在目标中开辟存储空间，用来存放DLL的文件名称
    char * pszFileNameRemote = (char *) VirtualAllocEx(hTargetProcess, 0, memorySize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if(!pszFileNameRemote) { return false; }

    //将DLL的文件名写入目标进程地址空间
    if(!WriteProcessMemory(hTargetProcess, pszFileNameRemote, (LPVOID)szFileName, memorySize, NULL)) { return false; }
    //获取kernel32.dll中的LoadLibraryA函数的地址
    PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

    //向进程中注入线程，线程执行由pfnStartAddr指定的LoadLibraryA函数，传入的参数是指明要加载的dll文件的位置的pszFileNameRemote参数
    HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess, NULL, 0, pfnStartAddr, pszFileNameRemote, 0, NULL);

    //等待创建的线程执行结束
    //WaitForSingleObject(hRemoteThread, INFINITE);
    //释放申请的虚拟内存空间
    VirtualFreeEx(hTargetProcess, 0, memorySize, MEM_RELEASE);
    //如果远程线程未关闭则手动关闭
    if(!hRemoteThread) { CloseHandle(hTargetProcess); }
}

int main(int argc, char * argv[])
{
    std::string DllPath = "C://Dll-generate.dll";

    InsertDll(DllPath);
    
    return 0;
}

