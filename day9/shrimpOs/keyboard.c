#include "bootpack.h"

struct FIFO8 keyfifo;

void inthandler21(int *esp)
/* 处理来自键盘的中断  由naskfunc.nas中的_asm_inthandler21调用 */
{
	unsigned char data;
	io_out8(PIC0_OCW2,0x61); 	//通知PIC"IRQ-1已经受理完毕"
	data=io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo,data);
	return;
}

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