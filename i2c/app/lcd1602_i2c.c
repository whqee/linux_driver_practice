#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#define IO_RD 0
#define IO_WR 1

// pcf8574--lcd1602: p7 p6 p5 p4 p3 p2 p1 p0
//                -- D7 D6 D5 D4 K  E  RW RS
//  [7-4]:data bit, send a 8bit,H then L.
//  K:blacklight, E(CS):1->0 info-lcd-rx, RW: R(1)/W(0), RS:1(data)/0(cmd)

// The status read of bits[3-0] is useless.


// commands
#define LCD_CMD_CLEAR_DISPLAY 0x01
#define LCD_CMD_RETURN_HOME 0x02
#define LCD_CMD_ENTRY_MODE_SET 0x04
#define LCD_CMD_DISPLAY_CONTROL 0x08
#define LCD_CMD_CURSOR_SHIFT 0x10
#define LCD_CMD_FUNCTION_SET 0x20
#define LCD_CMD_SETCGRAMADDR 0x40
#define LCD_CMD_SET_DDRAM_ADDR 0x80

// flags for display entry mode
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAY_MOVE 0x08
#define LCD_CURSOR_MOVE 0x00
#define LCD_MOVE_RIGHT 0x04
#define LCD_MOVE_LEFT 0x00

// flags for function set
#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 1<<3
#define LCD_NOBACKLIGHT 0<<3

#define En 1<<2  // Enable bit

#define Rw_R 1<<1  // Read/Write bit
#define Rw_W 0<<1  // Read/Write bit

#define Rs_DR 1<<0  // Register select bit
#define Rs_CMD 0<<0  // Register select bit



#define __Set_En(SByte); \
        SByte |= En;   \
        __lcd1602_i2c_write_byte(FD,SByte);
#define __UnSet_En(SByte); \
        SByte &= (~En);  \
        __lcd1602_i2c_write_byte(FD,SByte);


static __u8 Whether_LCD_Backlight = LCD_BACKLIGHT; // Global flag
// static __u8 Whether_LCD_Backlight = LCD_BACKLIGHT; // Global flag

// The status read from bits[3-0] is useless
static int __lcd1602_i2c_read_byte(int FD, unsigned char* RByte)
{
    int ret = ioctl(FD, IO_RD, *RByte);
    if(ret < 0) {
        perror("ioctl error1");
        return ret;
    }
    *RByte = ret;
    return ret;
}
static int __lcd1602_i2c_write_byte(int FD, unsigned char WByte)
{
    int ret = ioctl(FD, IO_WR, WByte);
    if(ret < 0) {
        perror("ioctl error1");
        return ret;
    }
}

static __u8 set_backlight(int FD, __u8 set)
{
    __u8 status;
    __lcd1602_i2c_read_byte(FD,&status);
    if(set)
        Whether_LCD_Backlight = LCD_BACKLIGHT;
    else
        Whether_LCD_Backlight = LCD_NOBACKLIGHT;

    return set?__lcd1602_i2c_write_byte(FD, status | LCD_BACKLIGHT)
                :__lcd1602_i2c_write_byte(FD, status & (~LCD_BACKLIGHT));
}

static void __lcd1602_i2c_wait_no_busy(int FD)
{
    __u8 sta, SByte = 0xf0;
    SByte = SByte | Rs_CMD | Rw_R | Whether_LCD_Backlight;
    __lcd1602_i2c_write_byte(FD,SByte);
    do
    {
        __Set_En(SByte);
        usleep(1000);
        __lcd1602_i2c_read_byte(FD,&sta);
        __UnSet_En(SByte);
    } while (sta & 0x80);
}

static void __lcd1602_i2c_flush_byte(int FD, __u8 RS, __u8 RW,__u8 cmd_or_data)
{
    __lcd1602_i2c_wait_no_busy(FD);
    __u8 SByte = RS | RW | Whether_LCD_Backlight;
    // send H 4 bits, then L 4 bits
    SByte |= (cmd_or_data & 0xf0);
    __lcd1602_i2c_write_byte(FD,SByte);
    __Set_En(SByte);
    __UnSet_En(SByte);
    SByte &= 0x0f;
    SByte |= (cmd_or_data << 4);
    __lcd1602_i2c_write_byte(FD,SByte);
    __Set_En(SByte);
    __UnSet_En(SByte);
}

static void  lcd1602_i2c_write_cmd(int FD, __u8 cmd)
{
    __lcd1602_i2c_flush_byte(FD,Rs_CMD,Rw_W, cmd);
}

static void  lcd1602_i2c_write_data(int FD, __u8 dat)
{
    __lcd1602_i2c_flush_byte(FD,Rs_DR,Rw_W, dat);
}

static void lcd1602_i2c_set_cursor(int FD, __u8 x, __u8 y)  
{
    __u8 addr = 0;

   	switch (y)
	{
		case 0:	 					
            addr = 0x00 + x;	break;	
		case 1:						
			addr = 0x40 + x; 	break;
		default:
            break;
	}
    lcd1602_i2c_write_cmd(FD, addr | LCD_CMD_SET_DDRAM_ADDR);
}

static void lcd1602_i2c_show_str(int FD, __u8 x, __u8 y, __u8 *pStr, __u8 size)     
{
    lcd1602_i2c_set_cursor(FD, x, y);      
    while (size--)
    {
        lcd1602_i2c_write_data(FD, *pStr++);
    }
}

static void lcd1602_init(int FD)
{
    // set_backlight(FD,0);
    lcd1602_i2c_write_cmd(FD, LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE |
        LCD_1LINE | LCD_5x8DOTS); // Set FUNC mode first
    usleep(5000);
    lcd1602_i2c_write_cmd(FD, LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE |
        LCD_2LINE | LCD_5x8DOTS);
    usleep(5000);
    lcd1602_i2c_write_cmd(FD, LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON);
    lcd1602_i2c_write_cmd(FD, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT);
    lcd1602_i2c_write_cmd(FD, LCD_CMD_CLEAR_DISPLAY);
}

int main(int argc, char ** argv)
{

	int FD = open(argv[1], O_RDWR);

	if (FD < 0) {
		perror("Open error.");
        return 1;
	}
    lcd1602_init(FD);
    // pcf8574--lcd1602: p7 p6 p5 p4 p3 p2 p1 p0
    //                -- D7 D6 D5 D4 K  E  RW RS
    // ioctl(FD, IO_WR, 0x00);
    // ioctl(FD, IO_WR, 0x20);
    // ioctl(FD, IO_WR, 0x24);
    // ioctl(FD, IO_WR, 0x20);
    // ioctl(FD, IO_WR, 0x00);
    // ioctl(FD, IO_WR, 0x04);
    // ioctl(FD, IO_WR, 0x00);
    // ioctl(FD, IO_WR, 0xc0);
    // ioctl(FD, IO_WR, 0xc4);
    // ioctl(FD, IO_WR, 0xc0);
	lcd1602_i2c_show_str(FD,0,0,"test",4);
	// lcd1602_i2c_show_str(FD,0,1,"1234234",8);
    // ioctl(FD, IO_WR, 0x00);
	return 0;
}
