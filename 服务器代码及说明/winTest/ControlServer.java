package winTest;
import java.io.IOException;
import java.net.Socket;
import java.net.ServerSocket;
import java.text.SimpleDateFormat;

import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;


//控制服务器
public class ControlServer extends Thread
{
    private int port; //端口
    private ServerSocket serverSocket; //服务套接字

    private SimpleDateFormat dateFormat; //时间格式

//    private ArrayList<SocketNode> SocketPack; //Socket列表

//    private Window window; //交互窗口

    //构造函数
    public ControlServer(int port)
    {
        //设置日志时间板式
        dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

        //绑定端口并创建套接字
        this.port = port;
        try
        {
            serverSocket = new ServerSocket(port,20);
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        //初始化Socket列表
        Adapter.SocketPack = new ArrayList<SocketNode>();
    }

    //绑定窗口
//    public void SetWindow(Window window)
//    {
//        this.window = window;
//    }

    //线程入口
    public void run()
    {
        writeLog("Server Started!");

        while(true)
        {
            Socket client = null;
            try
            {
                client = serverSocket.accept();
                SocketNode socketNode = new SocketNode(client);
                Adapter.SocketPack.add(socketNode);
                writeLog("New connection:" + client.getInetAddress().getHostAddress()+":"+socketNode.GetPort());
                Adapter.refrehClientNums();
                Adapter.refreshSelectClient();
                
                //向列表中添加新增主机
//                if(window != null)
//                {
//                    window.hostListModel.addElement(socketNode.GetHostAddress() + " : " + socketNode.GetPort());
//                }
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    //获取SocketNode列表
    public ArrayList<SocketNode> GetSocketPack()
    {
        return Adapter.SocketPack;
    }

    //获取指定地址的连接
    public SocketNode GetSocketNode(String hostAddress)
    {
        for(SocketNode sn : Adapter.SocketPack)
        {
            if(sn.GetHostAddress().equalsIgnoreCase(hostAddress))
            {
                return sn;
            }
        }
        return null;
    }

    //刷新SocketPack
    public void RefreshSocketPack(boolean wait)
    {
        //创建并启动刷新线程
        Thread task = new Thread(
            () -> {
                writeLog("SocketPack refreshing...");
                int length = Adapter.SocketPack.size();
                int deleted = 0;
                Iterator<SocketNode> nodeIterator = Adapter.SocketPack.iterator();
                SocketNode node;
                while(nodeIterator.hasNext())
                {
                    node = nodeIterator.next();
                    if(!node.IsLock() && !node.IsAlive())
                    {
//                        if(window != null)
//                        {
//                            window.hostListModel.removeElement(node.GetHostAddress() + " : " + node.GetPort());
//                        }
                        deleted++;
                        nodeIterator.remove();
                    }
                }
                writeLog("SocketPack Refreshed finished, deleted " + deleted + " of " + length + " sockets in SocketPack");
            }
        );
        task.run();

        //是否等待刷新结束
        if(wait)
        {
            try
            {
                task.join();
            }
            catch(InterruptedException e)
            {
                e.printStackTrace();
            }
        }
    }

    //从SocketPack中
    public void RemoveSocketNode(SocketNode socketNode)
    {
        Adapter.SocketPack.remove(socketNode);
    }

    //写出日志到控制台
    private void writeLog(String logLine)
    {
        String log = "ControlServer " + dateFormat.format(new Date()) + " | " + logLine;
        System.out.println(log);
        Adapter.controlNotice(log);
//        if(window != null) { window.AppendSystemLog(log); }
    }
}