; hello-os
; TAB=4
;==================================================================
; 关于FAT12的跟详细的内容可查阅资料或参考《Orange`s 一个操作系统的实现》
; 第4章中关于此部分的讲解
; 注释：宅
; 时间：2013年1月21日
;==================================================================
CYLS 	EQU 	10 				; 需要读入的柱面数

		ORG		0x7c00			; 指明程序的装载地址

; 以下的记述用于标准FAT12格式的软盘
; 与第一天的内容没什么区别，好吧，唯一的区别就是下面的jmp指令，它不再是机器码，显然这样更直观、易懂
		JMP		entry			; 跳转到entry处执行 注意：该跳转指令只占用2个字节，而标准规定的是3个字节 所以才会有下面的DB 0x90
		DB		0x90			; 机器码，事实上，这相当于一个"nop"指令，即空指令
		DB		"HELLOIPL"		; 启动区的名称
		DW		512				; 每扇区字节数
		DB		1				; 每簇的扇区数 此处为1 表面一个簇就等于一个扇区
		DW		1				; 这个值应该表示为第一个FAT文件分配表之前的引导扇区，一般情况只保留一个扇区
		DB		2				; FAT表的个数
		DW		224				; 根目录中文件数的最大值
		DW		2880			; 逻辑扇区总数
		DB		0xf0			; 磁盘的种类
		DW		9				; 每个FAT占用多少个扇区
		DW		18				; 每磁道扇区数
		DW		2				; 磁头数
		DD		0				; 隐藏扇区数
		DD		2880			; 如果逻辑扇区总数为0，则在这里记录扇区总数
		DB		0,0,0x29		; 中断13的驱动器号、未使用、扩展引导标志
		DD		0xffffffff		; 卷序列号
		DB		"SHRIMPOS   "	; 卷标，必须是11个字符，不足以空格填充
		DB		"FAT12   "		; 文件系统类型，必须是8个字符，不足填充空格
		RESB	18				; 先空出18字节

; 程序主体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		
		MOV 	AX,0x0820
		MOV		ES,AX
		MOV 	CH,0			; 柱面0
		MOV 	DH,0 			; 磁头0
		MOV		CL,2 			; 扇区2
readloop:
		MOV 	SI,0 			; 记录失败次数
retry:
		MOV 	AH,0x02			; AH=0x02 : 读盘
		MOV 	AL,1			; 1个扇区
		MOV 	BX,0
		MOV 	DL,0x00 		; A驱动器
		INT 	0x13			; 调用磁盘BIOS
		JNC 	next 			; 没错的话跳转到next
		ADD 	SI,1 			; 往SI加1
		CMP 	SI,5			; 比较SI与5
		JAE		error			; SI>=5时跳转到error
		MOV 	AH,0x00
		MOV 	DL,0x00
		INT 	0x13 			; 重置驱动器
		JMP 	retry
next:
		MOV 	AX,ES 			; 把内存地址后移0x20
		ADD 	AX,0x0020
		MOV 	ES,AX			; 因为没有ADD ES,0x020指令，这里稍微绕个弯
		ADD 	CL,1
		CMP 	CL,18			; 比较CL与18
		JBE 	readloop 		; 如果CL<=18跳转到readloop
		MOV 	CL,1
		ADD 	DH,1
		CMP 	DH,2
		JB 		readloop 		; 如果DH<2则跳转到readloop
		MOV 	DH,0
		ADD 	CH,1
		CMP 	CH,CYLS
		JB 		readloop 		; 如果CH<CYLS则跳转到readloop
		
		MOV		[0x0ff0],CH 	; 讲CYLS的值写入到内存地址0x0ff0中
		JMP		0xc200	
		
error:
		MOV SI,msg
		
putloop:
		MOV		AL,[SI]			; AL中存放要显示的字符
		ADD		SI,1			; SI加1
		CMP		AL,0
		JE		fin				; 如果已经将要输出的字符全部输出完了，则跳转到fin处
		MOV		AH,0x0e			
		MOV		BX,15			; bh = 0   bl为配色方案
		INT		0x10			; 调用显卡BIOS
		JMP		putloop			; 继续输出

fin:
		HLT						; 让CPU停止 等待指令
		JMP		fin				; 死循环			

msg:
		DB		0x0a, 0x0a		; 换行2次
		DB		"load error"	; 要输出的字符
		DB		0x0a			; 换行
		DB		0				; 结束输出的标识

		RESB	0x7dfe-$		; 填充字符

		DB		0x55, 0xaa