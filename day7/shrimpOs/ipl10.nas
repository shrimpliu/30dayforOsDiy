; hello-os
; TAB=4
;==================================================================
; ����FAT12�ĸ���ϸ�����ݿɲ������ϻ�ο���Orange`s һ������ϵͳ��ʵ�֡�
; ��4���й��ڴ˲��ֵĽ���
; ע�ͣ�լ
; ʱ�䣺2013��1��21��
;==================================================================
CYLS 	EQU 	10 				; ��Ҫ�����������

		ORG		0x7c00			; ָ�������װ�ص�ַ

; ���µļ������ڱ�׼FAT12��ʽ������
; ���һ�������ûʲô���𣬺ðɣ�Ψһ��������������jmpָ��������ǻ����룬��Ȼ������ֱ�ۡ��׶�
		JMP		entry			; ��ת��entry��ִ�� ע�⣺����תָ��ֻռ��2���ֽڣ�����׼�涨����3���ֽ� ���ԲŻ��������DB 0x90
		DB		0x90			; �����룬��ʵ�ϣ����൱��һ��"nop"ָ�����ָ��
		DB		"HELLOIPL"		; ������������
		DW		512				; ÿ�����ֽ���
		DB		1				; ÿ�ص������� �˴�Ϊ1 ����һ���ؾ͵���һ������
		DW		1				; ���ֵӦ�ñ�ʾΪ��һ��FAT�ļ������֮ǰ������������һ�����ֻ����һ������
		DB		2				; FAT��ĸ���
		DW		224				; ��Ŀ¼���ļ��������ֵ
		DW		2880			; �߼���������
		DB		0xf0			; ���̵�����
		DW		9				; ÿ��FATռ�ö��ٸ�����
		DW		18				; ÿ�ŵ�������
		DW		2				; ��ͷ��
		DD		0				; ����������
		DD		2880			; ����߼���������Ϊ0�����������¼��������
		DB		0,0,0x29		; �ж�13���������š�δʹ�á���չ������־
		DD		0xffffffff		; �����к�
		DB		"SHRIMPOS   "	; ��꣬������11���ַ��������Կո����
		DB		"FAT12   "		; �ļ�ϵͳ���ͣ�������8���ַ����������ո�
		RESB	18				; �ȿճ�18�ֽ�

; ��������

entry:
		MOV		AX,0			; ��ʼ���Ĵ���
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		
		MOV 	AX,0x0820
		MOV		ES,AX
		MOV 	CH,0			; ����0
		MOV 	DH,0 			; ��ͷ0
		MOV		CL,2 			; ����2
readloop:
		MOV 	SI,0 			; ��¼ʧ�ܴ���
retry:
		MOV 	AH,0x02			; AH=0x02 : ����
		MOV 	AL,1			; 1������
		MOV 	BX,0
		MOV 	DL,0x00 		; A������
		INT 	0x13			; ���ô���BIOS
		JNC 	next 			; û��Ļ���ת��next
		ADD 	SI,1 			; ��SI��1
		CMP 	SI,5			; �Ƚ�SI��5
		JAE		error			; SI>=5ʱ��ת��error
		MOV 	AH,0x00
		MOV 	DL,0x00
		INT 	0x13 			; ����������
		JMP 	retry
next:
		MOV 	AX,ES 			; ���ڴ��ַ����0x20
		ADD 	AX,0x0020
		MOV 	ES,AX			; ��Ϊû��ADD ES,0x020ָ�������΢�Ƹ���
		ADD 	CL,1
		CMP 	CL,18			; �Ƚ�CL��18
		JBE 	readloop 		; ���CL<=18��ת��readloop
		MOV 	CL,1
		ADD 	DH,1
		CMP 	DH,2
		JB 		readloop 		; ���DH<2����ת��readloop
		MOV 	DH,0
		ADD 	CH,1
		CMP 	CH,CYLS
		JB 		readloop 		; ���CH<CYLS����ת��readloop
		
		MOV		[0x0ff0],CH 	; ��CYLS��ֵд�뵽�ڴ��ַ0x0ff0��
		JMP		0xc200	
		
error:
		MOV SI,msg
		
putloop:
		MOV		AL,[SI]			; AL�д��Ҫ��ʾ���ַ�
		ADD		SI,1			; SI��1
		CMP		AL,0
		JE		fin				; ����Ѿ���Ҫ������ַ�ȫ��������ˣ�����ת��fin��
		MOV		AH,0x0e			
		MOV		BX,15			; bh = 0   blΪ��ɫ����
		INT		0x10			; �����Կ�BIOS
		JMP		putloop			; �������

fin:
		HLT						; ��CPUֹͣ �ȴ�ָ��
		JMP		fin				; ��ѭ��			

msg:
		DB		0x0a, 0x0a		; ����2��
		DB		"load error"	; Ҫ������ַ�
		DB		0x0a			; ����
		DB		0				; ��������ı�ʶ

		RESB	0x7dfe-$		; ����ַ�

		DB		0x55, 0xaa