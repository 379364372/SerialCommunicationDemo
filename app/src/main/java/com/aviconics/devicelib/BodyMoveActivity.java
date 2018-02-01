package com.aviconics.devicelib;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;

public class BodyMoveActivity extends Activity {
	private StringBuffer mStringBuffer = new StringBuffer();
	private TextView mTextView;
	private BodyControlCom bodyControl;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_body_move);
        mTextView = (TextView) this.findViewById(R.id.test);
        
        bodyControl =  new BodyControlCom();
		System.out.println("---------0----------");
		int ret = bodyControl.bodyCom_hardware_init();
		int ret2 = bodyControl.native_flexible_init();
		System.out.println("---------1----------");
        if (ret == 0)
        	mStringBuffer  = mStringBuffer.append("OK\n");
        else
        	mStringBuffer  = mStringBuffer.append("FALSE\n");
        mTextView.setText("subBoard_hardware_init" + mStringBuffer.toString());
    }
    
    public void onClick(View view) {
		int id = view.getId();
		switch (id) {

			case R.id.fix:
				int a,c;
				Log.e("demo", "APP FIX state");
				c = bodyControl.native_flexible(1);
				Log.e("demo", "APP FIX state : "+c);
				a = bodyControl.native_flexible_state();
				Log.e("demo", "APP FIX state : "+a);
				break;
			case R.id.power_off:
				int b,d;
				Log.e("demo", "APP FIX state");
				b = bodyControl.native_flexible(0);
				Log.e("demo", "APP FIX state : "+b);
				d = bodyControl.native_flexible_state();
				Log.e("demo", "APP FIX state : "+d);
				break;
			case R.id.left_rotate:
				byte []param = {0x02, (byte)0x2d}; //45
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_LEFT_ARM_ANGLE, param, 2);
				Log.e("demo", "APP CONTROL_SET_LEFT_ARM_ANGLE");
				break;

			case R.id.left_rotate2:
				byte []param1 = {0x02, (byte)0x87}; //135
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_LEFT_ARM_ANGLE, param1, 2);
				Log.e("demo", "APP CONTROL_SET_LEFT_ARM_ANGLE");
				break;
			case R.id.right_rotate:
				byte []param2 ={0x02, (byte)0x2d};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_RIGHT_ARM_ANGLE, param2, 2);
				Log.e("demo", "APP CONTROL_SET_RIGHT_ARM_ANGLE");
				break;
			case R.id.right_rotate2:
				byte []param3 ={0x02, (byte)0x87};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_RIGHT_ARM_ANGLE, param3, 2);
				Log.e("demo", "APP CONTROL_SET_RIGHT_ARM_ANGLE");
				break;
			case R.id.nod_rotate://-10
				//Log.e("demo", "device_sn:" + controlboard.fromHardGetSn("get_sn"));
				byte []param4 ={0x02, (byte)0x50};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_NOD_ANGLE, param4, 2);
				Log.e("demo", "APP CONTROL_SET_NOD_ANGLE");
				//Toast.makeText(getApplicationContext(), controlboard.fromHardGetSn("get_sn"), 1).show();
				 break;
			case R.id.nod_rotate2://0
				//Log.e("demo", "device_sn:" + controlboard.fromHardGetSn("get_sn"));
				byte []param5 ={0x02, (byte)0x5A};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_NOD_ANGLE, param5, 2);
				Log.e("demo", "APP CONTROL_SET_NOD_ANGLE");
				//Toast.makeText(getApplicationContext(), controlboard.fromHardGetSn("get_sn"), 1).show();
				break;
			case R.id.nod_rotate3://+10
				//Log.e("demo", "device_sn:" + controlboard.fromHardGetSn("get_sn"));
				byte []param6 ={0x02, (byte)0x64};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_NOD_ANGLE, param6, 2);
				Log.e("demo", "APP CONTROL_SET_NOD_ANGLE");
				//Toast.makeText(getApplicationContext(), controlboard.fromHardGetSn("get_sn"), 1).show();
				break;
				 
			case R.id.shake_rotate://1
				//bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_RIGHT_ARM_ANGLE, param, 2);
				byte []param7 ={0x02, (byte)0x01};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_SHAKE_ANGLE, param7, 2);
				Log.e("demo", "APP CONTROL_SET_SHAKE_ANGLE");
				break;
			case R.id.shake_rotate2://0
				//bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_RIGHT_ARM_ANGLE, param, 2);
				byte []param8 ={0x02, (byte)0x5A};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_SHAKE_ANGLE, param8, 2);
				Log.e("demo", "APP CONTROL_SET_SHAKE_ANGLE");
				break;
			case R.id.shake_rotate3://180
				//bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_RIGHT_ARM_ANGLE, param, 2);
				byte []param9 ={0x02, (byte)0xB4};
				bodyControl.bodyCom_msg_send(bodyControl.CONTROL_SET_SHAKE_ANGLE, param9, 2);
				Log.e("demo", "APP CONTROL_SET_SHAKE_ANGLE");
				break;
				
			default:
				break;
		}
	}
    
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.sub_board, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

	@Override
	protected void onDestroy() {
		super.onDestroy();
		bodyControl.bodyCom_hardware_exit();

	}
}
