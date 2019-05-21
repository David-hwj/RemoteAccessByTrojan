package winTest;

import java.awt.Dimension;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;

public class DllConnectNotice extends JScrollPane{

	public DllConnectNotice() {
		init();
	}
	
	private void init() {
		
		this.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
		this.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
		this.setBounds(20,25,560,160);
		
		JTextArea msg=new JTextArea();  Adapter.dllNotice=msg;//传给适配器
		msg.setEditable(false);
		msg.setLineWrap(true);
		msg.setText("No Message");

		
		this.setViewportView(msg);
	}
	
}
