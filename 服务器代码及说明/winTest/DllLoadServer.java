package winTest;
import java.io.File;
import java.io.FileInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;


//DLL下载服务器
class DllLoadServer extends Thread
{
    private final int port;
    private ServerSocket serverSocket;

    private String dll32Path;
    private String dll64Path;

    private final SimpleDateFormat dateFormat;

    //构造函数
    public DllLoadServer(int port, String dll32Path, String dll64Path)
    {
        //设置日志时间板式
        dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

        //绑定端口和dll路径
        this.port = port;
        this.dll32Path = dll32Path;
        this.dll64Path = dll64Path;

        //创建套接字
        try {
            serverSocket = new ServerSocket(port, 20);
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }

    //线程入口
    public void run()
    {
        writeLog("Server Started!");

        while(true)
        {
            try
            {
                //等待客户端连接
                Socket client = serverSocket.accept();
                BufferedReader reader = new BufferedReader(new InputStreamReader(client.getInputStream(),"gb2312"));
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(client.getOutputStream(), "gb2312"));

                String clientAddr = client.getInetAddress().getHostAddress()+":"+client.getPort();

                writeLog("New connection:" + clientAddr);

                //发送请求的文件
                String cmdLine = reader.readLine();
                if(cmdLine.equalsIgnoreCase("DLL32")) //32位DLL
                {
                    writeLog("Sending dll32 to '" + clientAddr + "'...");
                    SendFile(client, dll32Path);
                    writeLog("Sending dll32 to '" + clientAddr + "' finished!");
                }
                else if(cmdLine.equalsIgnoreCase("DLL64")) //64位DLL
                {
                    writeLog("Sending dll64 to '" + clientAddr + "'...");
                    SendFile(client, dll64Path);
                    writeLog("Sending dll64 to '" + clientAddr + "' finished!");
                }
                else
                {
                    writer.write("WTF are you requesting?!");
                    writer.flush();
                }

                //关闭连接
                reader.close();
                writer.close();
                client.close();

                writeLog("Connection '" + clientAddr + "' closed!");
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    //向用户套接字发送文件
    private void SendFile(Socket clientSocket, String filePath)
    {
        File file = new File(filePath);

        //检查文件是否存在
        if(!file.exists())
        {
            System.out.println("Can not open file:" + filePath);
            return;
        }

        //输出文件
        FileInputStream fin = null;
        DataOutputStream dout = null;
        try
        {
            fin = new FileInputStream(file);
            dout = new DataOutputStream((clientSocket.getOutputStream()));

            int sendedLength = 0;
            byte[] byteBuffer = new byte[1024];

            while ((sendedLength = fin.read(byteBuffer, 0, 1024)) > 0)
            {
                dout.write(byteBuffer, 0, sendedLength);
                dout.flush();
            }
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
        finally
        {
            try {
                if(dout != null) { dout.close(); }
                if(fin != null) { fin.close(); }
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    //写出日志到控制台
    private void writeLog(String logLine)
    {
        String log = "DllLoadServer " + dateFormat.format(new Date()) + " | " + logLine;
        System.out.println(log);
        Adapter.dllNotice(log);
    }
}