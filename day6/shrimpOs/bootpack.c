#include "bootpack.h"
#include<stdio.h>

void HariMain(void)
{
	//char *vram; /*变量vram，用于BYTE型地址*/
	//int xsize,ysize;
	struct BOOTINFO *binfo=(struct BOOTINFO *)0x0ff0;;
	char s[40], mcursor[256];
	int mx, my;
	
	init_gdtidt();		/* 初始化GDT, IDT */
	init_pic();			/* 初始化PIC */
	io_sti();			/* 打开所有可屏蔽中断 */
	
	init_palette(); 	//设定调色板
	init_screen8(binfo->vram,binfo->scrnx,binfo->scrny);
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_00FF00, "SHRIMP OS");
	mx = (binfo->scrnx - 16) / 2; //画面中央坐标计算
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor,COL8_008484);
	putblock8_8(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
	sprintf(s,"(%d,%d)",mx,my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 16, 31, COL8_00FF00, s);

	io_out8(PIC0_IMR, 0xf9); /* PIC0(11111001) (打开IRQ1键盘中断和连接从PIC的IRQ2)*/
	io_out8(PIC1_IMR, 0xef); /* PIC1(11101111) (打开PS2鼠标中断 即IRQ12)*/
	
	for (;;) {
		io_hlt();	/*执行naskfunc.nas里的_io_hlt*/
	}

}






