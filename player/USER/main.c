#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch.h"	 	
#include "includes.h"
#include "key.h"  
#include "malloc.h" 
#include "w25qxx.h"    
#include "sdio_sdcard.h"
#include "ff.h"  
#include "exfuns.h"    
#include "fontupd.h"
#include "text.h"	 
#include "wm8978.h"	 
#include "videoplayer.h" 
#include "timer.h" 
#include "piclib.h"	
#include "fattester.h"	 
//ALIENTEK ̽����STM32F407������ ʵ��57
//UCOSII-�ź���������    --�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

 

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   


//VEDIO����
//�����������ȼ�
#define VEDIO_TASK_PRIO       			6 
//���������ջ��С
#define VEDIO_STK_SIZE  		    		512
//�����ջ	
OS_STK VEDIO_TASK_STK[VEDIO_STK_SIZE];
//������
void vedio_task(void *pdata);


//FILE����
//�����������ȼ�
#define FILE_TASK_PRIO       			6 
//���������ջ��С
#define FILE_STK_SIZE  		    		64
//�����ջ	
OS_STK FILE_TASK_STK[FILE_STK_SIZE];
//������
void file_task(void *pdata);



//������
//�����������ȼ�
#define MAIN_TASK_PRIO       			4 
//���������ջ��С
#define MAIN_STK_SIZE  					128
//�����ջ	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);


//////////////////////////////////////////////////////////////////////////////
OS_EVENT * msg_key;			//���������¼���ָ��	 	  
//����������   
void ucos_load_main_ui(void)
{
	LCD_Clear(WHITE);	//����
 	POINT_COLOR=RED;	//��������Ϊ��ɫ 
	LCD_ShowString(30,10,200,16,16,"Explorer STM32");	
	LCD_ShowString(30,30,200,16,16,"UCOSII TEST2");	
	LCD_ShowString(30,50,200,16,16,"ATOM@ALIENTEK");
   	LCD_ShowString(30,75,200,16,16,"KEY0:DS1 KEY_UP:ADJUST");	
   	LCD_ShowString(30,95,200,16,16,"KEY1:BEEP  KEY2:CLEAR"); 
	LCD_ShowString(80,210,200,16,16,"Touch Area");	
	LCD_DrawLine(0,120,lcddev.width,120);
	LCD_DrawLine(0,70,lcddev.width,70);
	LCD_DrawLine(150,0,150,70);
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
  	LCD_ShowString(160,30,200,16,16,"CPU:   %");	
   	LCD_ShowString(160,50,200,16,16,"SEM:000");	
}	 


	
int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	
	LED_Init();					//��ʼ��LED 
	KEY_Init(); 				//������ʼ��  
 	LCD_Init();					//LCD��ʼ�� 
	tp_dev.init();				//��������ʼ��
	//Init mem_tools
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	exfuns_init();			//Ϊfatfs��ر��������ڴ�  
  	f_mount(fs[0],"0:",1); 	//����SD�� 
 	f_mount(fs[1],"1:",1); 	//����FLASH.
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	
	LED_Init();					//��ʼ��LED 
 	LCD_Init();					//LCD��ʼ��  
 	KEY_Init();					//������ʼ��   
	W25QXX_Init();				//��ʼ��W25Q128
	WM8978_Init();				//��ʼ��WM8978	
	
	WM8978_ADDA_Cfg(1,0);		//����DAC
	WM8978_Input_Cfg(0,0,0);	 //�ر�����ͨ��
	WM8978_Output_Cfg(1,0);		//����DAC���  
	WM8978_HPvol_Set(25,25);
	WM8978_SPKvol_Set(60);
	TIM3_Int_Init(10000-1,8400-1);//10Khz����,1�����ж�һ��
	
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);		//��ʼ��CCM�ڴ�� 
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
  	f_mount(fs[0],"0:",1); 		//����SD��  
	POINT_COLOR=RED;      
	while(font_init()) 			//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
		delay_ms(200);	
		LED0=!LED0;
	}  	 
//  update_font(20,110,16,"0:");//�����ֿ�
	//tp_dev.init();				//��������ʼ��
	
//	delay_ms(1500);	
	
	
	//ucos_load_main_ui();		//����������	 
  	OSInit();  	 				//��ʼ��UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	    
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		  
	msg_key=OSMboxCreate((void*)0);	//������Ϣ���� 			  
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
 //	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);	 				   
 //	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						    				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}	  
//LED����
void vedio_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	while(1){
		video_play();
		OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
	  OSTaskResume(MAIN_TASK_PRIO);
		OSTaskDel(VEDIO_TASK_PRIO);
		OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
//		delay_ms(1);
	}
}	   
//�ļ�����
void file_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	while(1){
		file_manage();
		OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
	  OSTaskResume(MAIN_TASK_PRIO);
		OSTaskDel(VEDIO_TASK_PRIO);
		OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
//		delay_ms(1);
	}
}	   




//������
void main_task(void *pdata)
{				
	OS_CPU_SR cpu_sr=0;
	while(1){
		LCD_ShowHomePic(); 
		POINT_COLOR=RED;
		Show_Str(20,20,120,16,"��Ƶ������",16,1);	
		Show_Str(135,20,120,16,"�ļ��鿴��",16,1);	
		while(1)
		{
			if(TP_Scan(0))//
			{
				if(tp_dev.x[0]>20 && tp_dev.x[0]<70 && tp_dev.y[0]>40 && tp_dev.y[0]<100)
				{
						POINT_COLOR=BLUE;
						Show_Str(20,20,120,16,"��Ƶ������",16,1);	
						delay_ms(300);
						OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
					 	OSTaskCreate(vedio_task,(void *)0,(OS_STK*)&VEDIO_TASK_STK[VEDIO_STK_SIZE-1],VEDIO_TASK_PRIO);						    				   
						OSTaskSuspend(MAIN_TASK_PRIO);	//������ʼ����.
						OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
						break;
				}else if(tp_dev.x[0]>135 && tp_dev.x[0]<220 && tp_dev.y[0]>40 && tp_dev.y[0]<70)
				{
						POINT_COLOR=BLUE;
						Show_Str(135,20,120,16,"�ļ��鿴��",16,1);	
						delay_ms(300);
						OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
					 	OSTaskCreate(file_task,(void *)0,(OS_STK*)&FILE_TASK_STK[FILE_STK_SIZE-1],FILE_TASK_PRIO);						    				   
						OSTaskSuspend(MAIN_TASK_PRIO);	//������ʼ����.
						OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
						break;
				}
			}
			delay_ms(1);
		}
	} 
}
