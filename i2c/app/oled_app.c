#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "codetable.h"

static int OLED_Write_Cmd(int fd, unsigned char cmd);
static int OLED_Write_Data(int fd, unsigned char data);
static int OLED_Init(int fd);
static void OLED_SetPos(int fd, unsigned char x, unsigned char y);
static void OLED_Fill(int fd,unsigned char fill_Data);
static void OLED_CLS(int fd);
static void OLED_ON(int fd);
static void OLED_OFF(int fd);
static void OLED_ShowStr(int fd, unsigned char x, 
			unsigned char y, unsigned char ch[], unsigned char TextSize);
static void OLED_ShowCN(int fd, unsigned char x, unsigned char y, 
													unsigned char N);
static void OLED_DrawBMP(int fd, unsigned char x0,unsigned char y0,
				unsigned char x1,unsigned char y1,unsigned char BMP[]);




int main(int argc, char const *argv[])
{
    int fd = open(argv[1], O_RDWR);
    if(fd < 0) {
        perror("Open error.");
        return 1;
    }
    if (OLED_Init(fd) < 0)
        perror("OLED Init error.");
    
	while(1)
	{
		OLED_Fill(fd, 0xFF);//全屏点亮
		sleep(2);
		OLED_Fill(fd,0x00);//全屏灭
		sleep(2);
		for(int i=0;i<6;i++)
		{
			OLED_ShowCN(fd,22+i*16,0,i);//测试显示中文
		}
		sleep(2);
		OLED_ShowStr(fd,0,3,"Hello whqee",1);//测试6*8字符
		OLED_ShowStr(fd,0,4,"Hello Tech",2);				//测试8*16字符
		sleep(2);
		OLED_CLS(fd);//清屏
		OLED_OFF(fd);//测试OLED休眠
		sleep(2);
		OLED_ON(fd);//测试OLED休眠后唤醒
		OLED_DrawBMP(fd, 0,0,128,8,(unsigned char *)BMP1);//测试BMP位图显示
		sleep(2);
	}

    return 0;
}


static int OLED_Write_Cmd(int fd, unsigned char cmd)
{
    return ioctl(fd, 1, cmd);
}

static int OLED_Write_Data(int fd, unsigned char data)
{
    return ioctl(fd, 0, data);
}

static int OLED_Init(int fd)
{
	usleep(100*1000); //这里的延时很重要
	
	OLED_Write_Cmd(fd,0xAE); //display off
	OLED_Write_Cmd(fd,0x20);	//Set Memory Addressing Mode	
	OLED_Write_Cmd(fd,0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_Write_Cmd(fd,0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	OLED_Write_Cmd(fd,0xc8);	//Set COM Output Scan Direction
	OLED_Write_Cmd(fd,0x00); //---set low column address
	OLED_Write_Cmd(fd,0x10); //---set high column address
	OLED_Write_Cmd(fd,0x40); //--set start line address
	OLED_Write_Cmd(fd,0x81); //--set contrast control register
	OLED_Write_Cmd(fd,0xff); //亮度调节 0x00~0xff
	OLED_Write_Cmd(fd,0xa1); //--set segment re-map 0 to 127
	OLED_Write_Cmd(fd,0xa6); //--set normal display
	OLED_Write_Cmd(fd,0xa8); //--set multiplex ratio(1 to 64)
	OLED_Write_Cmd(fd,0x3F); //
	OLED_Write_Cmd(fd,0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	OLED_Write_Cmd(fd,0xd3); //-set display offset
	OLED_Write_Cmd(fd,0x00); //-not offset
	OLED_Write_Cmd(fd,0xd5); //--set display clock divide ratio/oscillator frequency
	OLED_Write_Cmd(fd,0xf0); //--set divide ratio
	OLED_Write_Cmd(fd,0xd9); //--set pre-charge period
	OLED_Write_Cmd(fd,0x22); //
	OLED_Write_Cmd(fd,0xda); //--set com pins hardware configuration
	OLED_Write_Cmd(fd,0x12);
	OLED_Write_Cmd(fd,0xdb); //--set vcomh
	OLED_Write_Cmd(fd,0x20); //0x20,0.77xVcc
	OLED_Write_Cmd(fd,0x8d); //--set DC-DC enable
	OLED_Write_Cmd(fd,0x14); //
	OLED_Write_Cmd(fd,0xaf); //--turn on oled panel
}

static void OLED_SetPos(int fd, unsigned char x, unsigned char y) //设置起始点坐标
{ 
	OLED_Write_Cmd(fd, 0xb0+y);
	OLED_Write_Cmd(fd, ((x&0xf0)>>4)|0x10);
	OLED_Write_Cmd(fd, (x&0x0f)|0x01);
}

static void OLED_Fill(int fd,unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		OLED_Write_Cmd(fd,0xb0+m);		//page0-page1
		OLED_Write_Cmd(fd,0x00);		//low column start address
		OLED_Write_Cmd(fd,0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				OLED_Write_Data(fd,fill_Data);
			}
	}
}

static void OLED_CLS(int fd)//清屏
{
	OLED_Fill(fd, 0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
static void OLED_ON(int fd)
{
	OLED_Write_Cmd(fd, 0X8D);  //设置电荷泵
	OLED_Write_Cmd(fd, 0X14);  //开启电荷泵
	OLED_Write_Cmd(fd, 0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
static void OLED_OFF(int fd)
{
	OLED_Write_Cmd(fd, 0X8D);  //设置电荷泵
	OLED_Write_Cmd(fd, 0X10);  //关闭电荷泵
	OLED_Write_Cmd(fd, 0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
static void OLED_ShowStr(int fd, unsigned char x, 
			unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(fd, x,y);
				for(i=0;i<6;i++)
					OLED_Write_Data(fd, F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(fd, x,y);
				for(i=0;i<8;i++)
					OLED_Write_Data(fd, F8X16[c*16+i]);
				OLED_SetPos(fd, x,y+1);
				for(i=0;i<8;i++)
					OLED_Write_Data(fd, F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
static void OLED_ShowCN(int fd, unsigned char x, unsigned char y, 
													unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(fd, x , y);
	for(wm = 0;wm < 16;wm++)
	{
		OLED_Write_Data(fd, F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(fd, x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		OLED_Write_Data(fd, F16x16[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
static void OLED_DrawBMP(int fd, unsigned char x0,unsigned char y0,
				unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(fd, x0,y);
    for(x=x0;x<x1;x++)
		{
			OLED_Write_Data(fd, BMP[j++]);
		}
	}
}
