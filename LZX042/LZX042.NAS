;nasm -f win32 lzx042.nas
;   KUMAamp project(âºèÃ)
;   Copyright (C) Mamiya 2000.
;   License:GPL
;   ---------------------------------------------------------------------------
;   LZX decoder
;   lzx042.nas
[SECTION .text]

%macro GETBYTE 1
		MOV	%1,BYTE [EDX]
		INC	EDX
%endmacro

%macro GETBIT 0
		DEC	BH
		JNS	SHORT %%NLD
		GETBYTE	BL
		MOV	BH,7
%%NLD:		ADD	BL,BL
%endmacro

		align	4
		global	_lzx042
_lzx042:
		CLD
		PUSH	EBX
		PUSH	EDI
		PUSH	ESI
		MOV	EDX,[4+ESP+12]
		MOV	EDI,[8+ESP+12]
		ADD	EDX,24H
.SLP:		ADD	EDX,2
		CMP	DWORD [EDX],4CFFFF7FH
		JNE	.SLP
		ADD	EDX,4
		GETBYTE	BL
		MOV	BH,8
		JMP	SHORT .L26

		align	4
.L00:		GETBIT
		ADC	CL,CL
		GETBIT
		ADC	CL,CL
		GETBYTE	AL
.L1A:		INC	CL
.L1C:		LEA	ESI,[EDI+EAX]
		INC	ECX
		REP	MOVSB
.L26:		GETBIT
		JNC	SHORT .L36
		GETBYTE	AL
		STOSB
		JMP	SHORT .L26

		align	4
.L36:		MOV	EAX,-1
		XOR	ECX,ECX
		GETBIT
		JNC	SHORT .L00
		GETBYTE	AH
		GETBYTE	AL
		MOV	CL,AL
		SAR	EAX,3
		AND	CL,7
		JNZ	SHORT .L1A
		GETBYTE	CL
		OR	CL,CL
		JNZ	SHORT .L1C
		POP	ESI
		POP	EDI
		POP	EBX
		XOR	EAX,EAX
		RET
