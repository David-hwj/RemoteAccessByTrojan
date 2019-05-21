package winTest;

import java.util.ArrayList;
import java.util.Vector;

import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JTextArea;

public class Adapter {

	static Vector<String> allDllNotice=new Vector();//所有DllServer历史通知
	static Vector<String> allConNotice=new Vector();//所有ControlServer历史通知
	static Vector<String> allInterNotice=new Vector();//所有交互历史通知
	
//	static Vector<Client> actiUsers=new Vector();//活跃客户端
	static ArrayList<SocketNode> SocketPack; //Socket列表
	static Vector<String> commands=new Vector();//已实现命令
	
	static JLabel dSPanel;//显示DLL服务地址和端口
	static JLabel cSPanel;//显示控制服务地址和端口
	
	static JTextArea dllNotice;//显示dll服务连接通知
	static JTextArea conNotice;//显示Control服务连接通知
	
	static JLabel clientNums;//显示活跃客户端数
	static JComboBox selectClient;//选择交互用户
	static JTextArea interMsg;//显示控制交互信息
	static JComboBox commandSelect;//命令选择框
	
	public static void initCommands() {
		commands.add("SHELL|ipconfig");
		commands.add("SHELL|date");
		commands.add("SHELL|net user");
		commands.add("SHELL|dir C:\\Users");
		commands.add("FETCHFILE|C:\\Users\\233.mp4");
		commands.add("SENDFILE|C:\\Users\\Administrator\\Desktop\\fileFromServer.txt|C:\\");
	}
	
	public static void dllNotice(String notice) {
		dllServerConnected(notice);
	}
	public static void controlNotice(String notice) {
		controlServerNotice(notice);
	}
	public static void commandNotice(String notice) {
		allInterNotice.addElement(notice);
		refrehInterMsg();
	}
	
	/**
	 * dll连接
	 * @param notice
	 */
	public static void dllServerConnected(String notice) {
		allDllNotice.add(notice);
		refreshAllDllNotice();
	}
	public static void controlServerNotice(String notice) {
		allConNotice.add(notice);
		refreshAllConNotice();
	}
	/**
	 * control连接
	 * @param notice
	 * @param client
	 */
	public static void controlServerConnected(String notice,SocketNode node) {
		controlServerNotice(notice);
		SocketPack.add(node);
		refreshSelectClient();
	}
	/**
	 * control断开连接
	 * @param notice
	 * @param client
	 */
	public static void controlServerDisConnected(String notice,SocketNode node) {
		controlServerNotice(notice);
		SocketPack.remove(node);
		refreshSelectClient();
	}
	
	/**
	 * 发送一个指令
	 * @param notice
	 */
	public static void sendCommand(String notice) {
		allInterNotice.addElement(notice);
	}
	/**
	 * 刷新DLL通知
	 */
	public static void refreshAllDllNotice() {
		StringBuilder sb=new StringBuilder();
		if(allDllNotice.size()>0)sb.append(allDllNotice.elementAt(0));
		for(int i=1;i<allDllNotice.size();i++) {
			sb.append("\r\n"+allDllNotice.elementAt(i));
		}
		dllNotice.setText(sb.toString());
	}
	/**
	 * 刷新Control通知
	 */
	public static void refreshAllConNotice() {
		StringBuilder sb=new StringBuilder();
		if(allConNotice.size()>0)sb.append(allConNotice.elementAt(0));
		for(int i=1;i<allConNotice.size();i++) {
			sb.append("\r\n"+allConNotice.elementAt(i));
		}
		conNotice.setText(sb.toString());
		
	}
	/**
	 * 刷新用户选择框
	 */
	public static void refreshSelectClient() {
		selectClient.removeAllItems();
		for(int i=0;i<SocketPack.size();i++)
			selectClient.addItem(SocketPack.get(i).toString());
	}
	/**
	 * 刷新交互信息
	 */
	public static void refrehInterMsg() {
		StringBuilder sb=new StringBuilder();
		if(allInterNotice.size()>0)sb.append(allInterNotice.elementAt(0));
		for(int i=1;i<allInterNotice.size();i++) {
			sb.append("\r\n"+allInterNotice.elementAt(i));
		}
		interMsg.setText(sb.toString());
	}
	/**
	 * 刷新客户端数
	 */
	public static void refrehClientNums() {	
		clientNums.setText("ActivityNums:"+SocketPack.size());
	}
	
}
