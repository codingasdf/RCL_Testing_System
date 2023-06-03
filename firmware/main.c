/*包含头文件*/
#include "reg52.h"
#include "math.h"



/*预定义通用标识符*/
#define uint8 unsigned char
#define uint16 unsigned int
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long




/*定义特殊功能寄存器*/
#define LCD_Data P0//LCD的数据口
sbit LCD_BF=LCD_Data^7;//LCD忙信号位
sbit LCD_RS=P1^0;
sbit LCD_RW=P1^1;
sbit LCD_EN=P1^2;

sbit SET_4052_A=P1^6;
sbit SET_4052_B=P1^7;

sbit KEY_0=P2^0;
sbit KEY_1=P2^1;
sbit KEY_2=P2^2;
sbit KEY_3=P2^3;

sbit LED_0=P2^4;
sbit LED_1=P2^5;
sbit LED_2=P2^6;
sbit LED_3=P2^7;



/*预定义LCD专用标识符*/

#define LCD_GO_HOME               0x02         //AC=0，光标、画面回HOME位

//输入方式设置
#define LCD_AC_AUTO_INCREMENT     0x06         //数据读、写操作后，AC自动增一
#define LCD_AC_AUTO_DECREASE      0x04         //数据读、写操作后，AC自动减一
#define LCD_MOVE_ENABLE           0x05         //数据读、写操作，画面平移
#define LCD_MOVE_DISENABLE        0x04         //数据读、写操作，画面不动

//设置显示、光标及闪烁开、关
#define LCD_DISPLAY_ON            0x0C         //显示开
#define LCD_DISPLAY_OFF           0x08         //显示关
#define LCD_CURSOR_ON             0x0A         //光标显示
#define LCD_CURSOR_OFF            0x08         //光标不显示
#define LCD_CURSOR_BLINK_ON       0x09         //光标闪烁
#define LCD_CURSOR_BLINK_OFF      0x08         //光标不闪烁

//光标、画面移动，不影响DDRAM
#define LCD_LEFT_MOVE             0x18         //LCD显示左移一位
#define LCD_RIGHT_MOVE            0x1C         //LCD显示右移一位
#define LCD_CURSOR_LEFT_MOVE      0x10         //光标左移一位
#define LCD_CURSOR_RIGHT_MOVE     0x14         //光标右移一位

//工作方式设置
#define LCD_DISPLAY_DOUBLE_LINE   0x38         //两行显示
#define LCD_DISPLAY_SINGLE_LINE   0x30         //单行显示







/*设置全局变量*/

//频率输入及定时器相关变量
unsigned long input_pulse=0,input_frequency=0,input_true_value=0;
unsigned long input_average_temp[5]=0;
unsigned long timer_0_count_50ms=0;

//按键键值以及模式标记
unsigned char key_value=0;
unsigned char temp_mode=0;







/*函数声明*/

//LCD相关
void Judge_LCD_busy(void);//检测是否忙碌
void LCD_ClrAll(void);//清屏
void LCD_Write(unsigned char WriteData);//写控制字
void LCD_write_data(unsigned char LCD_data);//写数据显示
void LCD_cursor(unsigned char x);//光标起始地址
void LCD_printc(unsigned char lcd_data);//输出一个字符
void LCD_prints(unsigned char *lcd_string);//输出字符串
void LCD_initial(void);//初始化


//延时
void Delay_1ms(unsigned int x);
//定时器初始化
void Timer_Config();
//值输出
void Value_to_ASCII_and_Print(unsigned long value_8byte,unsigned char cursor_address);
//求累计值平均
unsigned long Average_Output(unsigned long value_8byte);
//设置LCD显示模式
void Set_Mode_R();
void Set_Mode_C();
void Set_Mode_L();
void Set_Mode_F();




/*LCD忙碌判断子程序*/
void Judge_LCD_busy(void)   //判断LCD1602是否忙状态
{
	while(1)
	{
		LCD_EN=0;
		LCD_RS=0;
		LCD_RW=1;
		LCD_Data=0xff;
		LCD_EN=1;   //EN 是 1—0 使能
		if(!LCD_BF)break; //LCD_BF=1表示忙碌，需要等待。
	}
	LCD_EN=0;
}

/*LCD清屏*/
void LCD_ClrAll(void)
{
	Judge_LCD_busy();   //判断是否忙碌
	LCD_RS=0;
	LCD_RW=0;
	LCD_Data=0x01;
	LCD_EN=1;
	LCD_EN=0;
}

/*LCD写数据定义各种模式*/
void LCD_Write(unsigned char WriteData)   //写指令到LCD
{
	Judge_LCD_busy();
	LCD_RS=0;
	LCD_RW=0;

	LCD_Data=WriteData; //把WriteData的数据送到数据口
	LCD_EN=1;
	LCD_EN=0;
}

/*LCD显示数据*/
void LCD_write_data(unsigned char LCD_data)      //输出一个字节数据到LCD
{
	Judge_LCD_busy();
	LCD_RS=1;
	LCD_RW=0;

	LCD_Data=LCD_data;
	LCD_EN=1;
	LCD_EN=0;
}

/*LCD光标位置的确定*/
void LCD_cursor(unsigned char x)            //LCD光标定位到x处
{
	LCD_Write(0x80+x);                       //第一行地址是0x80
}

/*LCD输出一个字符*//*
void LCD_printc(unsigned char lcd_data)
{
	LCD_write_data(lcd_data);
}*/
/*
	LCD_cursor(0x04);
	LCD_printc('A');
*/

/*LCD输出字符串*/
void LCD_prints(unsigned char *lcd_string)
{
	unsigned char i=0;
	while(lcd_string[i]!=0x00)
	{
		LCD_write_data(lcd_string[i]);
		i++;
	}
}

/*LCD初始化程序*/
void LCD_initial(void)
{
	LCD_Write(LCD_AC_AUTO_INCREMENT|LCD_MOVE_DISENABLE);
	LCD_Write(LCD_DISPLAY_ON|LCD_CURSOR_OFF);
	LCD_Write(LCD_DISPLAY_DOUBLE_LINE);
	LCD_ClrAll();
}





void KEY_Scan()
{
	unsigned char temp_key=0;
//	P2=P2|0x0f;
	temp_key=P2&0x0f;
	if(temp_key!=0x0f)
	{
		switch(temp_key)
		{
		case(0x0e):
			key_value=3;
			break;
		case(0x0d):
			key_value=2;
			break;
		case(0x0b):
			key_value=1;
			break;
		case(0x07):
			key_value=0;
			break;
		}
	}
}




void Delay_1ms(unsigned int x)
{
	unsigned int j;
	unsigned int i;
	for(j=0; j<x; j++)
	{
		for(i=0; i<120; i++);
	}
}



void Value_to_ASCII_and_Print(unsigned long value_8byte,unsigned char cursor_address)
{
	unsigned char temp_print[]="00 000 000";
	if(value_8byte>99999999)
	{
		value_8byte=99999999;
	}
	temp_print[0]=value_8byte/10000000+'0';
	value_8byte=value_8byte%10000000;
	temp_print[1]=value_8byte/1000000+'0';
	value_8byte=value_8byte%1000000;
	temp_print[3]=value_8byte/100000+'0';
	value_8byte=value_8byte%100000;
	temp_print[4]=value_8byte/10000+'0';
	value_8byte=value_8byte%10000;
	temp_print[5]=value_8byte/1000+'0';
	value_8byte=value_8byte%1000;
	temp_print[7]=value_8byte/100+'0';
	value_8byte=value_8byte%100;
	temp_print[8]=value_8byte/10+'0';
	value_8byte=value_8byte%10;
	temp_print[9]=value_8byte+'0';

	LCD_cursor(cursor_address);
	LCD_prints(temp_print);
}




unsigned long Average_Output(unsigned long value_8byte)
{
	if(((abs(value_8byte-input_average_temp[0])+1.0)/(input_average_temp[0]+1.0))>0.2)
	{
		input_average_temp[0]=value_8byte;
		input_average_temp[1]=value_8byte;
		input_average_temp[2]=value_8byte;
		input_average_temp[3]=value_8byte;
		input_average_temp[4]=value_8byte;
		LED_0=0;
		return value_8byte;
	}
	else
	{
		input_average_temp[4]=input_average_temp[3];
		input_average_temp[3]=input_average_temp[2];
		input_average_temp[2]=input_average_temp[1];
		input_average_temp[1]=input_average_temp[0];
		input_average_temp[0]=value_8byte;
		LED_0=1;
		return (input_average_temp[0]+input_average_temp[1]+input_average_temp[2]+input_average_temp[3]+input_average_temp[4])/5;
	}
}

/*
unsigned long Average_Output(unsigned long value_8byte)
{
	input_average_temp[4]=input_average_temp[3];
	input_average_temp[3]=input_average_temp[2];
	input_average_temp[2]=input_average_temp[1];
	input_average_temp[1]=input_average_temp[0];
	input_average_temp[0]=value_8byte;
	return (input_average_temp[0]+input_average_temp[1]+input_average_temp[2]+input_average_temp[3]+input_average_temp[4])/5;
}
*/




void Timer_Config()
{
	//定时器T1做计数器 工作方式1(16位定时器) 只由TRx打开计数器
	//定时器T0做定时器 工作方式1(16位定时器) 只由TRx打开定时器
	TMOD=0x51;
	//设置定时器晶振为12MHz时的定时50ms
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	TH1=0;
	TL1=0;
	//打开中断
	EA=1;
	ET0=1;
	ET1=1;
	//打开定时器
	TR0=1;
	TR1=1;
}




void Set_Mode_R()
{
	TR0=0;
	TR1=0;
	LED_2=0;
	SET_4052_A=0;
	SET_4052_B=0;
	LCD_cursor(0x00);
	LCD_prints("Testing RES     ");
	LCD_cursor(0x40);
	LCD_prints("             ohm");
	LED_2=1;
	TR0=1;
	TR1=1;
}
void Set_Mode_C()
{
	TR0=0;
	TR1=0;
	LED_2=0;
	SET_4052_A=0;
	SET_4052_B=1;
	LCD_cursor(0x00);
	LCD_prints("Testing CAP     ");
	LCD_cursor(0x40);
	LCD_prints("             p F");
	LED_2=1;
	TR0=1;
	TR1=1;
}
void Set_Mode_L()
{
	TR0=0;
	TR1=0;
	LED_2=0;
	SET_4052_A=1;
	SET_4052_B=0;
	LCD_cursor(0x00);
	LCD_prints("Testing INDUCTOR");
	LCD_cursor(0x40);
	LCD_prints("             u H");
	LED_2=1;
	TR0=1;
	TR1=1;
}
void Set_Mode_F()
{
	TR0=0;
	TR1=0;
	LED_2=0;
	SET_4052_A=1;
	SET_4052_B=1;
	LCD_cursor(0x00);
	LCD_prints("Testing FREQ    ");
	LCD_cursor(0x40);
	LCD_prints("              Hz");
	LED_2=1;
	TR0=1;
	TR1=1;
}






void main()
{

	LED_0=1;
	LED_1=1;
	LED_2=1;
	LED_3=1;

	LCD_initial();
	Delay_1ms(100);

	Timer_Config();

	Set_Mode_R();


	while(1)
	{
		KEY_Scan();

		if(temp_mode!=key_value)
		{
			temp_mode=key_value;

			if(temp_mode==0)
			{
				Set_Mode_R();
			}
			else if(temp_mode==1)
			{
				Set_Mode_C();
			}
			else if(temp_mode==2)
			{
				Set_Mode_L();
			}
			else if(temp_mode==3)
			{
				Set_Mode_F();
			}
		}

		if(TR1==0)
		{
			LED_3=0;

			input_frequency=input_pulse+TL1+(TH1*256);
			if(temp_mode==0)
			{
//				input_true_value=(144269504.1/input_frequency-1000)/2-500;//pure
//				input_true_value=(144269504.1/input_frequency-1000)/2.08-500-141;//w
				input_true_value=(144269504.1/input_frequency-1000)/2.0-500;//g
				input_true_value=Average_Output(input_true_value);
			}
			else if(temp_mode==1)
			{
//				input_true_value=707203.4/input_frequency;//pure
//				input_true_value=700000.0/input_frequency-30;//w
				input_true_value=700000.0/input_frequency-30;//g
				input_true_value=Average_Output(input_true_value);
			}
			else if(temp_mode==2)
			{
//				input_true_value=506605.9/input_frequency*1000000/input_frequency;//pure
//				input_true_value=1000000.0/input_frequency*1000000/input_frequency/1.66;//w
				input_true_value=1000000.0/input_frequency*1000000/input_frequency/1.70;//g
				input_true_value=Average_Output(input_true_value);
			}
			else if(temp_mode==3)
			{
				input_true_value=input_frequency;
			}


			Value_to_ASCII_and_Print(input_true_value,0x40);

			LED_3=1;

			input_pulse=0;
			TL1=0;
			TH1=0;

			TR0=1;
			TR1=1;
		}
	}
}



/*定时器0的中断函数*/
void Timer_0() interrupt 1
{
	//12MHZ设置定时50ms的初值
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;

	timer_0_count_50ms++;
	if(timer_0_count_50ms==20)//计时到1S
	{
		TR0=0;
		TR1=0;
		timer_0_count_50ms=0;
	}
}



/*定时器1的中断函数*/
void Timer_1() interrupt 3
{
	//计数到了65536进入一次中断
	input_pulse=input_pulse+65536;
}




