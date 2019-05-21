package winTest;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

//Socket结点类
class SocketNode
{
    private Socket socket; //当前套接字
    private String hostName; //当前套接字主机名
    private String hostAddress; //当前套接字地址
    private int port; //当前套接字端口号
    private boolean lock; //交互锁


    //构造函数
    SocketNode(Socket socket)
    {
        this.socket = socket;
        this.hostName = socket.getInetAddress().getHostName();
        this.hostAddress = socket.getInetAddress().getHostAddress();
        this.port = socket.getPort();
        this.lock = false;
    }

    //获取连接Socket
    public Socket GetSocket()
    {
        return socket;
    }

    //获取主机名称
    public String GetHostName()
    {
        return hostName;
    }

    //获取主机地址
    public String GetHostAddress()
    {
        return hostAddress;
    }

    //获取套接字端口号
    public int GetPort() { return port; }

    //判断套接字是否正常
    public boolean IsAlive()
    {
        boolean alive = false;
        try
        {
            OutputStream out = socket.getOutputStream();
            InputStream in = socket.getInputStream();

            //发送OK
            String test = "OK";
            out.write(test.getBytes());
            out.flush();

            //判断是否返回OK
            byte[] buffer = new byte[1024];
            int len = in.read(buffer);
            String response = new String(buffer, 0, len);
            if(response.equalsIgnoreCase("OK")) { alive = true; }
        }
        catch(IOException e)
        {
            alive = false;
        }
        return alive;
    }

    public String toString() {
    	return this.hostAddress+":"+this.port;
    }
    //当前套接字上锁
    public void Lock()
    {
        this.lock = true;
    }

    //当前套接字解锁
    public void Unlock()
    {
        this.lock = false;
    }

    //检查套接字是否锁定
    public boolean IsLock()
    {
        return this.lock;
    }
}