package winTest;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTable;




public class ControlWindow extends JFrame {

	String dllServer="47.106.20.56:6666";
	String conServer="47.106.20.56:5547";
	
	public void init() {
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setTitle("Console");
		this.setSize(600, 862);
		this.setLayout(new FlowLayout());
		
		initServerPanel();
		initControlPanel();
		
		
		
		this.setResizable(false);
		this.setLocationRelativeTo(null);
		this.setVisible(true);
	}
	
	private void initServerPanel() {
		JPanel sPanel=new JPanel();
		sPanel.setPreferredSize(new Dimension(600,400));
		sPanel.setBackground(Color.GRAY);
		sPanel.setLayout(null);
		//DLL Server
		JLabel dSPanel=new JLabel("DLL Load Server:"+dllServer); Adapter.dSPanel=dSPanel;//传给适配器
		dSPanel.setBounds(20, 2, 580, 20);
		sPanel.add(dSPanel);	
		DllConnectNotice acn=new DllConnectNotice();
		sPanel.add(acn);
		
		//Control Server
		JLabel cSPanel=new JLabel("Control Server:"+conServer); Adapter.cSPanel=cSPanel;//传给适配器
		cSPanel.setBounds(20, 190, 580, 20);
		sPanel.add(cSPanel);
		ControlConnectNotice ccn=new ControlConnectNotice();
		sPanel.add(ccn);
		
		
		this.add(sPanel);
	}
	private void initControlPanel() {
	
		InterCommand ic = new InterCommand();
		
		this.add(ic);
		
	}
	
	
	
	
	
	public static void main(String[] args) {
		ControlWindow cw=new ControlWindow();
		cw.init();
		
		 //鏈嶅姟閰嶇疆(涓や釜绔彛鍙疯鍕夸慨鏀癸紝鏈ㄩ┈绔凡鍐欐)
        int DllLoadServerPort = 6666;
        String dll32Path = "C:\\Users\\Administrator\\Desktop\\VIR\\spcv32.dll";
        String dll64Path = "C:\\Users\\Administrator\\Desktop\\VIR\\spcv64.dll";
        int ControlServerPort = 5547;


        //鍚姩Dll涓嬭浇鏈嶅姟鍣�
        DllLoadServer dllLoadServer = new DllLoadServer(DllLoadServerPort, dll32Path, dll64Path);
        dllLoadServer.start();

        //鍚姩鎺у埗鏈嶅姟鍣�
        ControlServer controlServer = new ControlServer(ControlServerPort);
//        controlServer.RefreshSocketPack(false);
        controlServer.start();
		
		Adapter.refreshAllDllNotice();
		Adapter.refreshAllConNotice();
		Adapter.refreshSelectClient();
		Adapter.refrehInterMsg();
	}

}
