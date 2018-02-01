#ifndef _rtu_modbus_h_
#define _rtu_modbus_h_

#include <stdlib.h>

#define REG_LINE 			0		//���Ե�ַ��־ ��һ0Ϊ
#define REG_MODBUS 			1		//MODBUD��ַ��־ ��һΪ0��00001��1��10001��3��30001��4��40001

#define COIL_ON  			1		//��Ȧ����־
#define COIL_OFF 			0		//��Ȧ�ر�־
#define STATE_ON  			1		//״̬����־
#define STATE_OFF	 		0		//״̬�ر�־

//��׼MODBUS������
#define READ_COIL			1		//����Ȧ�Ĵ���
#define READ_STATE			2		//��״̬�Ĵ���
#define READ_HOLD			3		//�����ּĴ���
#define READ_INPUT			4		//������Ĵ���
#define WRITE_1_COIL		5		//д����Ȧ�Ĵ���
#define WRITE_1_HOLD		6		//д�����ּĴ���
#define WRITE_N_COIL		15		//д����Ȧ�Ĵ���
#define WRITE_N_HOLD		16		//д�ౣ�ּĴ���

//��չMODBUS������
#define READ_COMMAND		0x80	//����� �ɼ�ģ��������
#define WRITE_COMMAND		0x90	//����д �ɼ�ģ��������

//��չЭ��������
#define OPCS_COMMAND		0x88	//OPCS����
#define CFG_COMMAND			0x89	//��������

//*--------------------------------------------------------------------------------------
//*�ⲿ���ö���
//*--------------------------------------------------------------------------------------
void modbus_addr_init(u_char *addr);
extern void set_modbus_addr(u_char *addr);
extern u_short read_coil(u_short coil_addr,u_short reg_type);					//��Coil״̬ ��0��
extern void write_coil(u_short coil_addr,u_short on_off,u_short reg_type);		//дCoil״̬ д0��
extern u_short read_state(u_short status_addr,u_short reg_type);				//������״̬ ��1��
extern void write_state(u_short state_addr,u_short on_off,u_short reg_type);	//дCoil״̬ д0��
extern u_short read_input(u_short input_addr,u_short reg_type);					//������Ĵ��� ��3��
extern void write_input(u_short input_addr,u_short data,u_short reg_type);		//д���ּĴ��� ��4��
extern u_short read_hold(u_short hold_addr,u_short reg_type);					//�����ּĴ��� ��4��
extern void write_hold(u_short hold_addr,u_short data,u_short reg_type);		//д���ּĴ��� ��4��
extern u_short sx2toi(u_char *str);												//2λ16����ASCII������ת������
extern void itosx2(u_short data,u_char *str);									//������ת2λ16����ASCII������
extern u_short modbus_asc_hex(u_char *asc_buf,u_short num);						//1�ֽ�ASCII��ת2λ16����������
extern u_short modbus_hex_asc(u_char *hex_buf,u_char *asc_buf,u_short num);		//2λ16��������תASCII������
extern u_short modbus_data(u_char *command_buf,u_char *ack_buf);				//MODBUS Э��Ӧ�����������ӳ���
extern u_char LRC(u_char *auchMsg, u_short usDataLen);							//LRCУ���ӳ���
extern u_short CRC16(u_char* puchMsg, u_short usDataLen);						//16λCRCУ���ӳ���
extern u_short read_reg_n(u_short reg_addr);
extern void write_reg_n(u_short reg_addr,u_short data);
extern float read_reg_f(u_short reg_addr,u_char swap);
extern void write_reg_f(u_short reg_addr,float data,u_char swap);
extern u_int read_reg_ln(u_short reg_addr,u_char swap);
extern void write_reg_ln(u_short reg_addr,u_int data,u_char swap);
extern void move_reg(u_short d_reg_adr,u_short s_reg_adr,u_short reg_num,u_char swap);
#endif
