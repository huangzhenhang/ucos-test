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
//ALIENTEK 探索者STM32F407开发板 实验57
//UCOSII-信号量和邮箱    --库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

 

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   


//VEDIO任务
//设置任务优先级
#define VEDIO_TASK_PRIO       			6 
//设置任务堆栈大小
#define VEDIO_STK_SIZE  		    		512
//任务堆栈	
OS_STK VEDIO_TASK_STK[VEDIO_STK_SIZE];
//任务函数
void vedio_task(void *pdata);


//FILE任务
//设置任务优先级
#define FILE_TASK_PRIO       			6 
//设置任务堆栈大小
#define FILE_STK_SIZE  		    		64
//任务堆栈	
OS_STK FILE_TASK_STK[FILE_STK_SIZE];
//任务函数
void file_task(void *pdata);



//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			4 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					128
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);


//////////////////////////////////////////////////////////////////////////////
OS_EVENT * msg_key;			//按键邮箱事件块指针	 	  
//加载主界面   
void ucos_load_main_ui(void)
{
	LCD_Clear(WHITE);	//清屏
 	POINT_COLOR=RED;	//设置字体为红色 
	LCD_ShowString(30,10,200,16,16,"Explorer STM32");	
	LCD_ShowString(30,30,200,16,16,"UCOSII TEST2");	
	LCD_ShowString(30,50,200,16,16,"ATOM@ALIENTEK");
   	LCD_ShowString(30,75,200,16,16,"KEY0:DS1 KEY_UP:ADJUST");	
   	LCD_ShowString(30,95,200,16,16,"KEY1:BEEP  KEY2:CLEAR"); 
	LCD_ShowString(80,210,200,16,16,"Touch Area");	
	LCD_DrawLine(0,120,lcddev.width,120);
	LCD_DrawLine(0,70,lcddev.width,70);
	LCD_DrawLine(150,0,150,70);
 	POINT_COLOR=BLUE;//设置字体为蓝色 
  	LCD_ShowString(160,30,200,16,16,"CPU:   %");	
   	LCD_ShowString(160,50,200,16,16,"SEM:000");	
}	 


	
int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	
	LED_Init();					//初始化LED 
	KEY_Init(); 				//按键初始化  
 	LCD_Init();					//LCD初始化 
	tp_dev.init();				//触摸屏初始化
	//Init mem_tools
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池 
	exfuns_init();			//为fatfs相关变量申请内存  
  	f_mount(fs[0],"0:",1); 	//挂载SD卡 
 	f_mount(fs[1],"1:",1); 	//挂载FLASH.
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	
	LED_Init();					//初始化LED 
 	LCD_Init();					//LCD初始化  
 	KEY_Init();					//按键初始化   
	W25QXX_Init();				//初始化W25Q128
	WM8978_Init();				//初始化WM8978	
	
	WM8978_ADDA_Cfg(1,0);		//开启DAC
	WM8978_Input_Cfg(0,0,0);	 //关闭输入通道
	WM8978_Output_Cfg(1,0);		//开启DAC输出  
	WM8978_HPvol_Set(25,25);
	WM8978_SPKvol_Set(60);
	TIM3_Int_Init(10000-1,8400-1);//10Khz计数,1秒钟中断一次
	
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM);		//初始化CCM内存池 
	exfuns_init();				//为fatfs相关变量申请内存  
  	f_mount(fs[0],"0:",1); 		//挂载SD卡  
	POINT_COLOR=RED;      
	while(font_init()) 			//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
		delay_ms(200);	
		LED0=!LED0;
	}  	 
//  update_font(20,110,16,"0:");//更新字库
	//tp_dev.init();				//触摸屏初始化
	
//	delay_ms(1500);	
	
	
	//ucos_load_main_ui();		//加载主界面	 
  	OSInit();  	 				//初始化UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	    
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
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
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
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

//开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	pdata = pdata; 		  
	msg_key=OSMboxCreate((void*)0);	//创建消息邮箱 			  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
 //	OSTaskCreate(touch_task,(void *)0,(OS_STK*)&TOUCH_TASK_STK[TOUCH_STK_SIZE-1],TOUCH_TASK_PRIO);	 				   
 //	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						    				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}	  
//LED任务
void vedio_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	while(1){
		video_play();
		OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
	  OSTaskResume(MAIN_TASK_PRIO);
		OSTaskDel(VEDIO_TASK_PRIO);
		OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
//		delay_ms(1);
	}
}	   
//文件任务
void file_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	while(1){
		file_manage();
		OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
	  OSTaskResume(MAIN_TASK_PRIO);
		OSTaskDel(VEDIO_TASK_PRIO);
		OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
//		delay_ms(1);
	}
}	   




//主任务
void main_task(void *pdata)
{				
	OS_CPU_SR cpu_sr=0;
	while(1){
		LCD_ShowHomePic(); 
		POINT_COLOR=RED;
		Show_Str(20,20,120,16,"视频播放器",16,1);	
		Show_Str(135,20,120,16,"文件查看器",16,1);	
		while(1)
		{
			if(TP_Scan(0))//
			{
				if(tp_dev.x[0]>20 && tp_dev.x[0]<70 && tp_dev.y[0]>40 && tp_dev.y[0]<100)
				{
						POINT_COLOR=BLUE;
						Show_Str(20,20,120,16,"视频播放器",16,1);	
						delay_ms(300);
						OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
					 	OSTaskCreate(vedio_task,(void *)0,(OS_STK*)&VEDIO_TASK_STK[VEDIO_STK_SIZE-1],VEDIO_TASK_PRIO);						    				   
						OSTaskSuspend(MAIN_TASK_PRIO);	//挂起起始任务.
						OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
						break;
				}else if(tp_dev.x[0]>135 && tp_dev.x[0]<220 && tp_dev.y[0]>40 && tp_dev.y[0]<70)
				{
						POINT_COLOR=BLUE;
						Show_Str(135,20,120,16,"文件查看器",16,1);	
						delay_ms(300);
						OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
					 	OSTaskCreate(file_task,(void *)0,(OS_STK*)&FILE_TASK_STK[FILE_STK_SIZE-1],FILE_TASK_PRIO);						    				   
						OSTaskSuspend(MAIN_TASK_PRIO);	//挂起起始任务.
						OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
						break;
				}
			}
			delay_ms(1);
		}
	} 
}
