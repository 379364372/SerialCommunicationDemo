#include <stdio.h>
#include <stdlib.h>

#include "modbus.h"
static u_char *modbus_addr = 0;

/*MODBUS寄存器区 第一个64K段*/
#define REG_ADR_START		((u_short*)modbus_addr)					/*寄存器首地址*/
#define STATE_ADDRESS 		((u_short*)modbus_addr)					/*线圈地址 00001~04096 32000~32255*/
#define INPUT_ADDRESS 		((u_short*)(modbus_addr + 0x200))		/*输入寄存器地址 30001~31999*/
#define COIL_ADDRESS 		((u_short*)(modbus_addr + 0xa00))		/*状态地址 10001~14096  49990~-----*/
#define HOLD_ADDRESS 		((u_short*)(modbus_addr + 0xc00))		/*保持寄存器地址 40001~19989*/

#define COIL_SIZE 			4096	/*线圈寄存器个数*/
#define STATE_SIZE 			4096	/*状态寄存器个数*/
#define INPUT_SIZE 			1024	/*输入寄存器个数*/
#define HOLD_SIZE 			9999	/*保持寄存器个数*/
void modbus_addr_init(u_char *addr)
{
	modbus_addr = addr;
}

u_short read_coil(u_short coil_addr, u_short reg_type)
{
	u_short i, j;

	if(!modbus_addr)
		return 0;
	if(reg_type == REG_MODBUS)
		coil_addr -= 1;

	if(coil_addr >= COIL_SIZE)
		return 0;
	i = coil_addr / 16; 
	j = coil_addr % 16;
	i = *(COIL_ADDRESS + i) & (1 << j);
	return (i != 0);
}

//写Coil状态 写0区
void write_coil(u_short coil_addr, u_short on_off, u_short reg_type)
{
	u_short i, j, *addr;

	if(!modbus_addr)
		return ;
	if(reg_type == REG_MODBUS)
		coil_addr -= 1;
	if(coil_addr >= COIL_SIZE)
		return;	
	i = coil_addr / 16;
	j = coil_addr % 16;
	j = 1 << j;
	addr = COIL_ADDRESS + i;
	if(on_off == COIL_ON) 
		*addr = ( *addr | j); 
	else if(on_off == COIL_OFF) 
		*addr = (*addr & (~j));
	else
		;
}

/*读输入状态 读1区*/
u_short read_state(u_short status_addr, u_short reg_type)
{
	u_short i, j;

	if(!modbus_addr)
		return 0;

	if(reg_type==REG_MODBUS) 
		status_addr -= 10001;
	if(status_addr >= STATE_SIZE)
		return 0;		
	i = status_addr / 16; 
	j = status_addr % 16;
	i = *(STATE_ADDRESS + i) & (1<<j);
	return (i != 0);
}

/*写输入状态 写1区*/
void write_state(u_short state_addr,u_short on_off,u_short reg_type)
{
	u_short i, j, *addr;

	if(!modbus_addr)
		return ;
	if(reg_type == REG_MODBUS)
		state_addr -= 10001;
	if(state_addr >= STATE_SIZE)
		return;	
	i = state_addr / 16; 
	j = state_addr % 16; 
	j = 1 << j;
	addr = STATE_ADDRESS + i;
	if(on_off == STATE_ON)
		*addr = (*addr | j);
	else if(on_off == STATE_OFF)
		*addr = (*addr & (~j));
	else
		;
}

/*读输出寄存器 读3区*/
u_short read_input(u_short input_addr,u_short reg_type)
{
	if(!modbus_addr)
		return 0;

	if(reg_type == REG_MODBUS)
		input_addr -= 30001;
	if(input_addr >= INPUT_SIZE)
		return 0;
	return *(INPUT_ADDRESS + input_addr);
}

/*写输入寄存器 写3区*/
void write_input(u_short input_addr, u_short data, u_short reg_type)
{
	if(!modbus_addr)
		return ;

	if(reg_type==REG_MODBUS)
		input_addr -= 30001;
	if(input_addr >= INPUT_SIZE)
		return;	
	*(INPUT_ADDRESS + input_addr) = data;
}

/*读保持寄存器 读4区*/
u_short read_hold(u_short hold_addr, u_short reg_type)
{
	if(!modbus_addr)
		return 0;
	if(reg_type == REG_MODBUS)
		hold_addr -= 40001;
	if(hold_addr >= HOLD_SIZE)
		return 0;
	return *(HOLD_ADDRESS + hold_addr);
}

/*写保持寄存器 读4区*/
void write_hold(u_short hold_addr, u_short data, u_short reg_type)
{
	if(!modbus_addr)
		return ;
	if(reg_type==REG_MODBUS)
		hold_addr -= 40001;
	if(hold_addr>=HOLD_SIZE)
		return;	
	*(HOLD_ADDRESS + hold_addr) = data;
}

/*读寄存器整型数*/
u_short read_reg_n(u_short reg_addr)
{
	if(!modbus_addr)
		return 0;

	if((reg_addr >= 1) && (reg_addr <= COIL_SIZE)) 
		return read_coil(reg_addr, REG_MODBUS);
	if((reg_addr >= 10001) && (reg_addr <= (10000 + STATE_SIZE)))
		return read_state(reg_addr, REG_MODBUS);
	if((reg_addr >= 30001) && (reg_addr <= (30000 + INPUT_SIZE)))
		return read_input(reg_addr, REG_MODBUS);
	if((reg_addr >= 40001) && (reg_addr <= (40000 + HOLD_SIZE)))
		return read_hold(reg_addr, REG_MODBUS);
	return 0;
}

/*写寄存器整型数*/
void write_reg_n(u_short reg_addr, u_short data)
{
	if(!modbus_addr)
		return ;
	if((reg_addr >= 1) && (reg_addr <= COIL_SIZE))
		write_coil(reg_addr, data, REG_MODBUS);
	else if((reg_addr >= 10001) && (reg_addr <= (10000 + STATE_SIZE)))
		write_state(reg_addr, data, REG_MODBUS);
	else if((reg_addr >= 30001) && (reg_addr <= (30000 + INPUT_SIZE)))
		write_input(reg_addr, data, REG_MODBUS);
	else if((reg_addr >= 40001) && (reg_addr <= (40000 + HOLD_SIZE)))
		write_hold(reg_addr, data, REG_MODBUS);
	else;
}

/*读寄存器浮点数 swap=0-低位在前 1-高位在前*/
float read_reg_f(u_short reg_addr,u_char swap)
{
	union { u_int i;
			float f;
		  } w;			/*浮点数与整型转换  注意:组态王与ADP3高地位相反!!!*/

	if(!modbus_addr)
		return 0.0;

	if((reg_addr >= 30001) && (reg_addr <= (30000 + INPUT_SIZE)))
	{
		if(swap==1)
			w.i = (read_input(reg_addr, REG_MODBUS) << 16) + read_input(reg_addr + 1, REG_MODBUS);
		else
			w.i = read_input(reg_addr, REG_MODBUS)+(read_input(reg_addr + 1 ,REG_MODBUS) << 16);
		return w.f;
	}
	else if((reg_addr >= 40001) && (reg_addr <= (40000 + HOLD_SIZE)))
	{
		if(swap == 1)
			w.i = (read_hold(reg_addr, REG_MODBUS) << 16)+read_hold(reg_addr + 1, REG_MODBUS);
		else
			w.i = read_hold(reg_addr, REG_MODBUS) + (read_hold(reg_addr + 1, REG_MODBUS) << 16);
		return w.f;
	}
	else
		;
	return 0;
}

/*写寄存器浮点数 swap=0-低位在前 1-高位在前*/
void write_reg_f(u_short reg_addr,float data,u_char swap)
{
	union { u_int i; float f; } w;		/*浮点数与整型转换  注意:组态王与ADP3高地位相反!!!*/

	if(!modbus_addr)
		return ;

	w.f = data;
	if((reg_addr >= 30001) && (reg_addr <= (30000 + INPUT_SIZE)))
	{
		if(swap == 1)
		{
			write_input(reg_addr, w.i >> 16, REG_MODBUS);
			write_input(reg_addr + 1, w.i, REG_MODBUS);
		}
		else
		{
			write_input(reg_addr, w.i, REG_MODBUS);
			write_input(reg_addr + 1, w.i >> 16, REG_MODBUS);
		}
	}
	else if((reg_addr >= 40001) && (reg_addr <= (40000 + HOLD_SIZE)))
	{
		if(swap == 1)
		{
			write_hold(reg_addr, w.i >> 16, REG_MODBUS);
			write_hold(reg_addr + 1, w.i, REG_MODBUS);
		}
		else
		{
			write_hold(reg_addr, w.i, REG_MODBUS);
			write_hold(reg_addr + 1, w.i >> 16, REG_MODBUS);
		}
	}
	else
		;
}

/*读寄存器长整型数 swap=0-低位在前 1-高位在前*/
u_int read_reg_ln(u_short reg_addr, u_char swap)
{
	u_int i;

	if(!modbus_addr)
		return 0;

	if((reg_addr >= 30001) && (reg_addr <= (30000 + INPUT_SIZE)))
	{
		if(swap == 1)
			i = (read_input(reg_addr, REG_MODBUS) << 16) + read_input(reg_addr + 1, REG_MODBUS);
		else
			i = read_input(reg_addr, REG_MODBUS) + (read_input(reg_addr + 1, REG_MODBUS) << 16);
		return i;
	}
	else if((reg_addr >= 40001) && (reg_addr <= (40000+HOLD_SIZE)))
	{
		if(swap == 1)
			i = (read_hold(reg_addr, REG_MODBUS) << 16)+read_hold(reg_addr + 1, REG_MODBUS);
		else
			i = read_hold(reg_addr, REG_MODBUS) + (read_hold(reg_addr + 1, REG_MODBUS) << 16);
		return i;
	}
	else;
	return 0;
}

/*写寄存器长整型数 swap=0-低位在前 1-高位在前*/
void write_reg_ln(u_short reg_addr,u_int data,u_char swap)
{
	u_int i;

	if(!modbus_addr)
		return ;

	i = data;
	if((reg_addr >= 30001) && (reg_addr <= (30000 + INPUT_SIZE)))
	{
		if(swap == 1)
		{
			write_input(reg_addr, i >> 16, REG_MODBUS);
			write_input(reg_addr + 1, i, REG_MODBUS);
		}
		else
		{
			write_input(reg_addr, i, REG_MODBUS);
			write_input(reg_addr + 1, i >> 16, REG_MODBUS);
		}
	}
	else if((reg_addr >= 40001) && (reg_addr <= (40000 + HOLD_SIZE)))
	{
		if(swap == 1)
		{
			write_hold(reg_addr, i >> 16, REG_MODBUS);
			write_hold(reg_addr + 1, i, REG_MODBUS);
		}
		else
		{
			write_hold(reg_addr, i, REG_MODBUS);
			write_hold(reg_addr + 1, i >> 16, REG_MODBUS);
		}
	}
	else;
}

/*寄存器搬移 swap=0-低位在前 1-高位在前*/
void move_reg(u_short d_reg_adr, u_short s_reg_adr, u_short reg_num, u_char swap)
{
	u_short i, n;
	u_short value0, value1;
	if(swap == 1)
	{
		n=reg_num / 2;
		for(i = 0;i < n; i++)
		{
			value0 = read_reg_n(s_reg_adr + i * 2);
			value1 = read_reg_n(s_reg_adr + i * 2 + 1);
			write_reg_n(d_reg_adr + i * 2, value1);
			write_reg_n(d_reg_adr + i * 2 + 1, value0);
		}
	}
	else
	{
		for(i = 0; i < reg_num; i++)
		{
			value0 = read_reg_n(s_reg_adr + i);
			write_reg_n(d_reg_adr + i, value0);
		}
	}
}

u_short sx2toi(u_char *str)
{
	u_short i=0;
	
	if((str[0]>='0') && (str[0]<='9'))
		i=(str[0]-'0')<<4;
	else if((str[0]>='A') && (str[0]<='F'))
		i=(str[0]-'A'+10)<<4;  
	else if((str[0]>='a') && (str[0]<='f'))
		i=(str[0]-'a'+10)<<4;  
	else;

	if((str[1]>='0') && (str[1]<='9'))
		i=i+str[1]-'0';
	else if((str[1]>='A') && (str[1]<='F'))
		i=i+str[1]-'A'+10;  
	else if((str[1]>='a') && (str[1]<='f'))
		i=i+str[1]-'a'+10;  
	else;
	
	return i;
}

/*整型数转2位16进制ASCII码数据*/
void itosx2(u_short data,u_char *str)
{
	u_short i,j;
	
	i=(data & 0xf0)>>4;
	j=data & 0x0f;
	

	if(i<=9)
		str[0]='0'+i;
	else
		str[0]='A'+i-10;  

	if(j<=9)
		str[1]='0'+j;
	else
		str[1]='A'+j-10;  
}

/*1字节ASCII码转2位16进制整型数*/
u_short modbus_asc_hex(u_char *asc_buf, u_short num)
{
	u_short i;
	num = (num - 3) / 2; 
	for(i = 0; i < num; i++) 
	 	asc_buf[i] = (u_char)sx2toi(&asc_buf[i * 2 + 1]);
	return i;
}

/*2位16进制数据转ASCII码数据*/
u_short modbus_hex_asc(u_char *hex_buf, u_char *asc_buf, u_short num)
{
	u_short i;
	asc_buf[0] = ':';
	for(i = 0; i < num; i++)
		itosx2((u_short)hex_buf[i], &asc_buf[i * 2 + 1]); 
	num = num * 2 + 1;
	asc_buf[num] = '\r';
	num++;
	asc_buf[num] = '\n';
	num++;
	return num;
}

u_short modbus_data(u_char *command_buf, u_char *ack_buf)
{
	u_short i,data_start,data_num,data,ack_num;

	switch(command_buf[1])
	{
	case READ_COIL:					//读多Coil状态 读0区
		ack_buf[0] = command_buf[0];
		ack_buf[1] = command_buf[1];
		data_start = command_buf[3] + (command_buf[2] << 8);
		data_num = command_buf[5]+(command_buf[4]<<8);

		if(data_num >= 2048)		//防止数据超界
			data_num = 2048;	
		ack_num = data_num / 8;

		if(data_num % 8)
			ack_num++;
		ack_buf[2] = ack_num;
		for(i = 0; i < ack_num; i++)
			ack_buf[i + 3] = 0;

		ack_num = 2;			
		for(i = 0; i < data_num; i++)
		{
			data = read_coil(data_start, REG_LINE);
			data_start++;

			if(i % 8 == 0)
				ack_num++;

			ack_buf[ack_num] = ack_buf[ack_num] | (data << (i % 8));
		}
		ack_num++;
		break;
	case READ_STATE:					//读输入状态 读1区
		ack_buf[0] = command_buf[0];
		ack_buf[1] = command_buf[1];
		data_start = command_buf[3] + (command_buf[2] << 8);
		data_num = command_buf[5] + (command_buf[4] << 8);
		if(data_num >= 2048)			//防止数据超界
			data_num = 2040;	
		ack_num=data_num / 8;

		if(data_num % 8)
			ack_num++;

		ack_buf[2] = ack_num;

		for(i = 0; i < ack_num; i++)
			ack_buf[i + 3] = 0;

		ack_num = 2;			
		for(i = 0; i < data_num; i++)
		{
			data = read_state(data_start, REG_LINE);
			data_start++;
			if(i % 8==0)
				ack_num++;
			ack_buf[ack_num] = ack_buf[ack_num] | (data << (i % 8));
		}
		ack_num++;
		break;
	case READ_HOLD:				//读保持寄存器值 读4区
		ack_buf[0] = command_buf[0];
		ack_buf[1] = command_buf[1];
		data_start = command_buf[3] + (command_buf[2] << 8);
		data_num = command_buf[5] + (command_buf[4] << 8);

		if(data_num >= 128)		//防止数据超界
			data_num=127;

		ack_buf[2] = data_num * 2;
		ack_num = 3;
		for(i = 0; i < data_num; i++)
		{
			data = read_hold(data_start, REG_LINE);
			data_start++;
			ack_buf[ack_num] = data >> 8;
 			ack_num++; 
			ack_buf[ack_num] = data;		
			ack_num++;	
		}
		break;
	case READ_INPUT:			//读输入寄存器值 读3区
		ack_buf[0] = command_buf[0];
		ack_buf[1] = command_buf[1];
		data_start = command_buf[3] + (command_buf[2] << 8);
		data_num = command_buf[5] + (command_buf[4] << 8);
		if(data_num >= 128)		//防止数据超界
			data_num = 127;	
		ack_buf[2] = data_num * 2;
		ack_num = 3;
		for(i = 0;i < data_num; i++)
		{
			data = read_input(data_start, REG_LINE);
			data_start++;
			ack_buf[ack_num] = data >> 8;
			ack_num++; 
			ack_buf[ack_num] = data;		
			ack_num++;	
		}
		break;
	case WRITE_1_COIL:	/*强制单个Coil输出 写0区*/
		ack_buf[0] = command_buf[0];
		ack_buf[1] = command_buf[1];
		ack_buf[2] = command_buf[2];
		ack_buf[3] = command_buf[3];
		ack_buf[4] = command_buf[4];
		ack_buf[5] = command_buf[5];
		data_start = command_buf[3] + (command_buf[2] << 8);
		data = command_buf[5] + (command_buf[4] << 8);
		ack_num = 6;
		if(data == 0xff00)
			write_coil(data_start, COIL_ON, REG_LINE);
		else if(data == 0)
			write_coil(data_start, COIL_OFF, REG_LINE);
		else
			ack_num = 0;
		break;
	case WRITE_1_HOLD:	/*写单个保持输出 写4区*/
		ack_buf[0]=command_buf[0];
		ack_buf[1]=command_buf[1];
		ack_buf[2]=command_buf[2];
		ack_buf[3]=command_buf[3];
		ack_buf[4]=command_buf[4];
		ack_buf[5]=command_buf[5];
		data_start=command_buf[3]+(command_buf[2]<<8);
		data=command_buf[5]+(command_buf[4]<<8);
		ack_num=6;
		write_hold(data_start,data,REG_LINE);
		break;
		case WRITE_N_COIL:	/*强制多个Coil输出 写0区*/
			ack_buf[0]=command_buf[0];
			ack_buf[1]=command_buf[1];
			ack_buf[2]=command_buf[2];
			ack_buf[3]=command_buf[3];
			ack_buf[4]=command_buf[4];
			ack_buf[5]=command_buf[5];
			data_start=command_buf[3]+(command_buf[2]<<8);
			data_num=command_buf[5]+(command_buf[4]<<8);
			ack_num=6;
			for(i=0;i<data_num;i++)
			{
				if(i%8==0) ack_num++;
				data=command_buf[ack_num] & (1<<(i%8));
				data=(u_int)(data != COIL_OFF);
				write_coil(data_start,data,REG_LINE);	data_start++;
			}
			ack_num=6;
			break;
		case WRITE_N_HOLD:	/*写多个保持输出 写4区*/
			ack_buf[0]=command_buf[0];
			ack_buf[1]=command_buf[1];
			ack_buf[2]=command_buf[2];
			ack_buf[3]=command_buf[3];
			ack_buf[4]=command_buf[4];
			ack_buf[5]=command_buf[5];
			data_start=command_buf[3]+(command_buf[2]<<8);
			data_num=command_buf[5]+(command_buf[4]<<8);
			ack_num=7;
			for(i=0;i<data_num;i++)
			{
				data=(command_buf[ack_num]<<8)+command_buf[ack_num+1];
				ack_num+=2; 
				write_hold(data_start,data,REG_LINE);	data_start++;
			}
			ack_num=6;
			break;
		default: ack_num=0;
			break;
	}
	return ack_num;
}
u_char LRC(u_char *auchMsg, u_short usDataLen)
{
	u_char uchLRC = 0 ;
	while (usDataLen--) { uchLRC += *auchMsg++ ; }
	return ((u_char)(~uchLRC+1));
}

/*16位CRC校验表*/
const u_char auchCRCHi[]={
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,

0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40
};
const u_char auchCRCLo[]={
0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,0x07,0xc7,0x05,0xc5,0xc4,0x04,
0xcc,0x0c,0x0d,0xcd,0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,0x08,0xc8,
0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,
0x14,0xd4,0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,0x11,0xd1,0xd0,0x10,
0xf0,0x30,0x31,0xf1,0x33,0xf3,0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,0x3b,0xfb,0x39,0xf9,0xf8,0x38,
0x28,0xe8,0xe9,0x29,0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,0xec,0x2c,
0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,

0xa0,0x60,0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,0xa5,0x65,0x64,0xa4,
0x6c,0xac,0xad,0x6d,0xaf,0x6f,0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,
0xb4,0x74,0x75,0xb5,0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,0x70,0xb0,
0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,
0x9c,0x5c,0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,0x99,0x59,0x58,0x98,
0x88,0x48,0x49,0x89,0x4b,0x8b,0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,0x40
};

/*16位CRC校验子程序*/
u_short CRC16(u_char* puchMsg, u_short usDataLen)
{
	u_char uchCRCHi=0xff;
	u_char uchCRCLo=0xff;
	u_short uIndex;
	while(usDataLen--)
	{
		uIndex=uchCRCHi^*(puchMsg++);
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo=auchCRCLo[uIndex];
	}
	return uchCRCHi<<8|uchCRCLo;
}
