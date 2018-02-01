package com.aviconics.devicelib;

import android.util.Log;

public class BodyControlCom {
	public static final int CONTROL_SET_LEFT_ARM_ANGLE =  1;
	public static final int CONTROL_SET_RIGHT_ARM_ANGLE=  2;
	public static final int CONTROL_SET_NOD_ANGLE =  3;
	public static final int CONTROL_SET_SHAKE_ANGLE =  4;
	public static final int CONTROL_CMD_MAX  = CONTROL_SET_SHAKE_ANGLE  + 1;
	
	public static final int CMD_PARAM_POS = 5;
	
	static byte [ ][ ] control_cmd_code = 
		{
				{0x11, 0x06 , 0x00 , 0x0a, 0x01, 0x28, 0x28, 0x55, 0x44}, //CONTROL_SET_LEFT_ARM_ANGLE
				{0x11, 0x06 , 0x00 , 0x0a, 0x02, 0x28, 0x28, 0x55, 0x44}, //CONTROL_SET_RIGHT_ARM_ANGLE
				{0x11, 0x06 , 0x00 , 0x0a, 0x03, 0x28, 0x28, 0x55, 0x44}, //CONTROL_SET_NOD_ANGLE
				{0x11, 0x06 , 0x00 , 0x0a, 0x04, 0x28, 0x28, 0x55, 0x44}
		};//CONTROL_SET_SHAKE_ANGLE

	public native int native_bodyCom_msg_send(int command, byte[] buffer, int len);
	public native int native_bodyCom_init (int port, int baud);
	public native int native_bodyCom_exit ();
	public native int native_flexible_init ();   //1  = down  0 =up
	public native int native_flexible (int flexible);   //1  = down  0 =up
	public native int native_flexible_state ();                      // 1 down_end  0 up_end -1 don't know
	public int  bodyCom_msg_send(int command, byte [] param, int param_size)
	{
		int ret = 0;
		byte [ ] cmd_code;
	
		cmd_code  = control_cmd_code[command - 1];
		
		for (int i =0; i < param_size; i++)
		{
			cmd_code[CMD_PARAM_POS + i] = param[i];
		}
		
		ret  =  native_bodyCom_msg_send(command, cmd_code, cmd_code.length);
		//System.out.println (cmd_code);
		
		return ret;
	}

	public static int  bodyCom_callback_ack_process (int command, byte[] ack_code, int len)
	{
		int ret = 0;
		int cmd;
		byte [ ] cmd_code;

		//System.out.println("subBoard_callback_ack_process");
		
		switch (command)
		{
			case CONTROL_SET_LEFT_ARM_ANGLE:
			case CONTROL_SET_RIGHT_ARM_ANGLE:
			case CONTROL_SET_NOD_ANGLE:
			case CONTROL_SET_SHAKE_ANGLE:
				cmd_code  = control_cmd_code[command - 1];
				//System.out.println("len = " + len + " i = " + i);
				if (ack_code[5] != cmd_code[4])
				{
					System.out.println("1 no  equals");
					ret = -1;
				}
				else
				{
					System.out.println("1 equals");
					ret = 0;
				}
				break;
				//fu tao hui diao
		}
		return ret;
}

	public int bodyCom_hardware_init()
	{
		return native_bodyCom_init (3, 9600);
	}

	public int bodyCom_hardware_exit ()
	{
		return native_bodyCom_exit();
	}
	static{

		System.loadLibrary("bodyControlCom");
	}

}
