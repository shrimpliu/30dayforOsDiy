#include "bootpack.h"
#include<stdio.h>

extern struct FIFO8 keyfifo,mousefifo;
struct MOUSE_DEC {
	unsigned char buf[3],phase;
	int x,y,btn;
};
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec,unsigned char dat);
void init_keyboard(void);

void HariMain(void)
{
	//char *vram; /*变量vram，用于BYTE型地址*/
	//int xsize,ysize;
	struct BOOTINFO *binfo=(struct BOOTINFO *) ADR_BOOTINFO;;
	char s[40], mcursor[256],keybuf[32],mousebuf[128];
	int mx, my,	i;
	unsigned char mouse_dbuf[3],mouse_phase;
	struct MOUSE_DEC mdec;
	
	init_gdtidt();		/* 初始化GDT, IDT */
	init_pic();			/* 初始化PIC */
	io_sti();			/* 打开所有可屏蔽中断 */
	fifo8_init(&keyfifo,32,keybuf);		//初始化FIFO缓冲区
	fifo8_init(&mousefifo,128,mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (打开IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/
	
	init_keyboard();
	
	init_palette(); 	//设定调色板
	init_screen8(binfo->vram,binfo->scrnx,binfo->scrny);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_00FF00, "SHRIMP OS");
	mx = (binfo->scrnx - 16) / 2; //画面中央坐标计算
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor,COL8_008484);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
	sprintf(s,"(%d,%d)",mx,my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, 31, COL8_FFFFFF, s);
	
	enable_mouse(&mdec);
	mouse_phase=0;		//进入等待鼠标的0xfa状态
	for (;;) {
		//io_hlt();	/*执行naskfunc.nas里的_io_hlt*/
		io_cli();
		if(fifo8_status(&keyfifo)+fifo8_status(&mousefifo)==0){
			io_stihlt();
		} else {
			if(fifo8_status(&keyfifo)!=0){
				i=fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,16,15,31);
				putfonts8_asc(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s);
			} else if(fifo8_status(&mousefifo)!=0){
				i=fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,i)!=0){
					//三个字节齐了，显示出来
					sprintf(s,"[lcr %4d %4d]",mdec.x,mdec.y);
					if((mdec.btn&0x01)!=0) {
						s[1]='L';
					}
					if((mdec.btn&0x02)!=0) {
						s[3]='R';
					}
					if((mdec.btn&0x04)!=0) {
						s[2]='C';
					}
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,32+15*8-1,31);
					putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);
					//鼠标指针的移动
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,mx,my,mx+15,my+15);
					mx+=mdec.x;
					my+=mdec.y;
					if(mx<0){
						mx=0;
					}
					if(my<0){
						my=0;
					}
					if(mx>binfo->scrnx-16){
						mx=binfo->scrnx-16;
					}
					sprintf(s,"(%3d %3d)",mx,my);
					boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,0,79,15);	//掩藏坐标
					putfonts8_asc(binfo->vram,binfo->scrnx,0,0,COL8_FFFFFF,s);	//显示坐标
					putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);	//描绘鼠标
				}
			}
		}
	}
}

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
/*等待键盘控制电路准备完毕*/
{
	for(;;){
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
/*初始化键盘控制电路*/
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);	//鼠标模式的模式号码为0x47
	return;
}

#define KEYCMD_SENDTO_MOUSE 		0xd4
#define MOUSECMD_ENABLE				0xf4

void enable_mouse(struct MOUSE_DEC *mdec)
/*激活鼠标*/
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase=0;	//等待0xfa的阶段
	return;		//顺利的话，键盘控制其会返送回ACK(0xfa)
}

int mouse_decode(struct MOUSE_DEC *mdec,unsigned char dat)
{
		if(mdec->phase==0){
			//等待鼠标的0xfa状态
			if (dat==0xfa) {
				mdec->phase=1;
			}
			return 0;
		} else if (mdec->phase==1) {
			//等待鼠标的第1字节
			if((dat&0xc8)==0x08) {
				//如果第一字节正确
				mdec->buf[0]=dat;
				mdec->phase=2;
			}
			return 0;
		} else if (mdec->phase==2) {
			//等待鼠标的第2字节
			mdec->buf[1]=dat;
			mdec->phase=3;
			return 0;
		} else if (mdec->phase==3) {
			//等待鼠标的第3字节
			mdec->buf[2]=dat;
			mdec->phase=1;
			mdec->btn=mdec->buf[0]&0x07;
			mdec->x=mdec->buf[1];
			mdec->y=mdec->buf[2];
			if((mdec->buf[0]&0x10)!=0) {
				mdec->x|=0xffffff00;
			}
			if((mdec->buf[0]&0x20)!=0) {
				mdec->y|=0xffffff00;
			}
			mdec->y=-mdec->y;	//鼠标的y方向与画面符号相反
			return 1;
		}
	return -1;
}


