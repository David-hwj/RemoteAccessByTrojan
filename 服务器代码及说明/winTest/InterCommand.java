package winTest;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

public class InterCommand extends JPanel{
	JLabel nums;//活跃客户端数
	JComboBox clientSelect;//活跃用户选择框
	JTextArea msg;//通知消息显示
	JComboBox commandSelect;//命令选择或输入
	
	
	
	

	public InterCommand() {
		init();
	}
	
	private void init() {
		
		this.setPreferredSize(new Dimension(600,400));
		this.setBackground(Color.lightGray);
		this.setLayout(null);
		
		nums=new JLabel("ActivityNums:"+0);Adapter.clientNums=nums;//传给适配器
		nums.setBounds(20, 20, 100, 20);
		this.add(nums);
		
		JLabel select=new JLabel("which to control:");
		select.setBounds(280, 20, 100, 20);
		this.add(select);
		
		//客户端选择框
		clientSelect=new JComboBox();Adapter.selectClient=clientSelect;//传给适配器
		clientSelect.setEditable(false);
//		for(int i=0;i<Adapter.SocketPack.size();i++)
//			clientSelect.addItem(Adapter.SocketPack.get(i).toString());
		clientSelect.setBounds(380, 20, 200, 20);
		this.add(clientSelect);
		
		//实时消息通知区域
		msg=new JTextArea();Adapter.interMsg=msg;//传给适配器
		msg.setEditable(false);
		msg.setLineWrap(true);
		msg.setText("No Message");
//		msg.setBounds(20, 50, 560, 300);//大小只够显示10行
		
		
		JScrollPane jsp=new JScrollPane( JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		jsp.setBounds(20, 50, 560, 300);
		jsp.setViewportView(msg);
		this.add(jsp);
	
		//命令输入或选择及发送按钮
		commandSelect=new JComboBox();Adapter.commandSelect=commandSelect;//传给适配器
		commandSelect.setEditable(true);
		Adapter.initCommands();
		for(int i=0;i<Adapter.commands.size();i++)
			commandSelect.addItem(Adapter.commands.elementAt(i));
		commandSelect.setBounds(20, 360, 460, 30);
		this.add(commandSelect);
		
		JButton send=new JButton("send");
		send.setBounds(500, 360, 80, 30);
		this.add(send);
		
		send.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent arg0) {
				String host=(String) clientSelect.getSelectedItem();
				SocketNode node=null;
				for(int i=0;i<Adapter.SocketPack.size();i++) {
					if(host.equals(Adapter.SocketPack.get(i).toString())) {
						node=Adapter.SocketPack.get(i);
					}
				}
				ControlUnit cu=new ControlUnit();
				if(cu.LoadTarget(node)) {
					cu.ExecuteCommand(Adapter.commandSelect.getSelectedItem().toString());
				}
				
			}
			
		});
		
	}
	
	
}
