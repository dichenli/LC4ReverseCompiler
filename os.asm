;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;   OS - TRAP VECTOR TABLE   ;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.OS
.CODE
.ADDR x8000
	; TRAP vector table
	JMP TRAP_GETC		; x00
	JMP TRAP_PUTC		; x01
	JMP TRAP_DRAW_PIXEL	; x02
	JMP TRAP_VIDEO_COLOR	; x03
	JMP TRAP_VIDEO_BOX		; x04
	JMP TRAP_TIMER			; x05
	JMP TRAP_PUTS			; x06

	OS_KBSR_ADDR .UCONST xFE00  ; ‘alias’ for keyboard status reg
	OS_KBDR_ADDR .UCONST xFE02  ; ‘alias’ for keyboard data reg
	OS_ADSR_ADDR .UCONST xFE04  ; aliases for ASCII display status reg
	OS_ADDR_ADDR .UCONST xFE06  ; aliases for ASCII display data reg
	OS_TSR_ADDR  .UCONST xFE08	; alias for timer status register
	OS_TIR_ADDR  .UCONST xFE0A	; alias for timer interval register
	OS_VIDEO_NUM_COLS .UCONST #128
	OS_VIDEO_NUM_ROWS .UCONST #124


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;; OS VIDEO MEMORY ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	.DATA
	.ADDR xC000	
	OS_VIDEO_MEM .BLKW x3E00

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;; OS VARIABLE MEMORY ;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;






;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   OS - TRAP IMPLEMENTATION   ;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.CODE
.ADDR x8200
.FALIGN
	;; by default, return to usercode: PC=x0000
	CONST R7, #0   ; R7 = 0
	RTI            ; PC = R7 ; PSR[15]=0


;;;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_GETC   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function: Get a single character from keyboard
;;; Inputs           - none
;;; Outputs          - R0 = ASCII character from keyboard

.CODE
TRAP_GETC
   	LC R0, OS_KBSR_ADDR  ; R0 = address of keyboard status reg
   	LDR R0, R0, #0       ; R0 = value of keyboard status reg
   	BRzp TRAP_GETC       ; if R0[15]=1, data is waiting!
                             ; else, loop and check again...

   	; reaching here, means data is waiting in keyboard data reg

   	LC R0, OS_KBDR_ADDR  ; R0 = address of keyboard data reg
   	LDR R0, R0, #0       ; R0 = value of keyboard data reg
	RTI                  ; PC = R7 ; PSR[15]=0


;;;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_PUTC   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function: Put a single character out to ASCII display
;;; Inputs           - R0 = ASCII character
;;; Outputs          - write to OS_ADDR_ADDR

.CODE
TRAP_PUTC
	LC R1, OS_ADSR_ADDR	; R1 = address of display status reg
	LDR R1, R1, #0		; R1 = value of status reg
	BRzp TRAP_PUTC		; If R1[15] = 1, data is ready to output, else continue to loop

	LC R1, OS_ADDR_ADDR	; R1 = address of display data reg
	STR R0, R1, #0		; write R0 to display
	RTI


;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_DRAW_PIXEL   ;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function: Draw point on video display
;;; Inputs           - R0 = row to draw on (y)
;;;                  - R1 = column to draw on (x)
;;;                  - R2 = color to draw with
;;; Outputs          - none

.CODE
TRAP_DRAW_PIXEL
	LEA R3, OS_VIDEO_MEM	  ; R3=start address of video memory
	LC  R4, OS_VIDEO_NUM_COLS ; R4=number of columns

	MUL R4, R0, R4		  ; R4= (row * NUM_COLS)
	ADD R4, R4, R1	 	  ; R4= (row * NUM_COLS) + col
	ADD R4, R4, R3		  ; Add the offset to the start of video memory
	CONST R5, xFF
	HICONST R5, xFD			; R5 = xFDFF, end of video memory
	SUB R3, R4, R5			; R3 = current video memory - max video memory allowed
	BRp END_DRAW_PIXEL		; if R4 too large, jump to RTI, no color shown
	STR R2, R4, #0			; Fill in the pixel with color from user (R2)
	END_DRAW_PIXEL
	RTI						; PC = R7 ; PSR[15]=0
	
	;; question, why is this a poorly written TRAP?  Does it protect video memory?
	
	

;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_VIDEO_COLOR   ;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function: Set all pixels of VIDEO display to a certain color
;;; Inputs           - R0 = color to set all pixels to
;;; Outputs          - none

.DATA
	;Below are memories needed for TRAP_VIDEO_COLOR and TRAP_VIDEO_BOX
	.ADDR xA000 
	COLOR_FILL
	.ADDR xA001
	X_BOX
	.ADDR xA002
	Y_BOX
	.ADDR xA003
	INDEX_I
	.ADDR xA004 
	INDEX_J
	.ADDR xA005
	FRAME_POINTER
	.ADDR xA006
	STACK_POINTER

.CODE
TRAP_VIDEO_COLOR
	LEA R3, FRAME_POINTER
	STR R5, R3, #0
	LEA R3, STACK_POINTER
	STR R6, R3, #0			  ; save R5 and R6 values(FP and SP)
	LEA R1, OS_VIDEO_MEM	  ; R1=start address of video memory
	LC  R2, OS_VIDEO_NUM_COLS ; R2=number of columns

	CONST R4, x00
	HICONST R4, x00				; row = 0
	LOOP_1						; for (row = 0 ; row <=123 ; row++)
		CMPIU R4, #123			; row <= 123?
		BRp END_LOOP_1				; if row > 123, end of loop 1		
		CONST R5, x00				
		HICONST R5, x00				; col = 0	
		LOOP_2						; for (col = 0 ; col <=127 ; col++)
			CMPIU R5, #127			; col <= 127?
			BRp END_LOOP_2				; if col > 127, end of loop 2
			CONST R6, x00
			HICONST R6, x00			; current addr = 0	
			MUL R6, R2, R4			; R6= (row * NUM_COLS)
			ADD R6, R6, R5			; R6= (row * NUM_COLS) + col
			ADD R6, R6, R1			; Add the offset to the start of video memory
			STR R0, R6, #0			; Fill in the pixel R6 with color from user (R0)
			ADD R5, R5, #1			; col++
			BRnzp LOOP_2			; go back to loop 2
		END_LOOP_2
		ADD R4, R4, #1				; row++
		BRnzp LOOP_1				;go back to loop 1
	END_LOOP_1
	LEA R3, FRAME_POINTER
	LDR R5, R3, #0
	LEA R3, STACK_POINTER
	LDR R6, R3, #0
RTI			  ; PC = R7 ; PSR[15]=0


;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_VIDEO_BOX   ;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function: Set 10 * 10 box of pixels of VIDEO display to a certain color
;;; Inputs           - R0 = color to set all pixels to,
;;;					 - R1, R2 = x and y coordinates of box left top corner
;;; Outputs          - none




.CODE
TRAP_VIDEO_BOX
	LEA R3, FRAME_POINTER
	STR R5, R3, #0
	LEA R3, STACK_POINTER
	STR R6, R3, #0
	LEA R3, OS_VIDEO_MEM	  ; R3=start address of video memory
	LC  R4, OS_VIDEO_NUM_COLS ; R4=number of columns
	LEA R5, COLOR_FILL
	STR R0, R5, #0			;save color information
	LEA R5, X_BOX
	STR R1, R5, #0
	LEA R5, Y_BOX
	STR R2, R5, #0
	ADD R5, R2, #0				; R5 = y, start coordinate row
	BOX_LOOP_1						; for (row = y ; row <=y+10 && row <= 123 ; row++)
		CMPIU R5, #123			; row <= 123?
		BRp END_BOX_LOOP_1				; if row > 123, end loop
		SUB R5, R5, R2				; row - y
		CMPIU R5, #10				; row - y < 10?
		BRzp END_BOX_LOOP_1				; if row - y >= 10, end loop
		ADD R5, R5, R2				; recover row by adding R2
		LEA R6, INDEX_I
		STR R5, R6, #0				; save value of row to INDEX_I
		LEA R1, X_BOX
		LDR R1, R1, #0				; load x coord of box
		ADD R5, R1, #0				; R5 = x, start coordinate col
		BOX_LOOP_2						; for (col = x ; row <=x+10 && col <=127 ; col++)
			CMPIU R5, #127			; col <= 127?
			BRp END_BOX_LOOP_2				; if col > 127, end of loop 2
			SUB R5, R5, R1				; col - x
			CMPIU R5, #10				; col - x < 10?
			BRzp END_BOX_LOOP_2				; if col - x >= 10, end loop
			ADD R5, R5, R1				; recover col by adding R1	
			LEA R6, INDEX_J
			STR R5, R6, #0				; save value of row to INDEX_J		
			LEA R6, INDEX_I
			LDR R6, R6, #0		; R6 = row
			CONST R0, x00
			HICONST R0, x00			; current addr = 0	
			MUL R0, R6, R4			; R0= (row * NUM_COLS)
			ADD R0, R0, R5			; R0= (row * NUM_COLS) + col
			ADD R0, R0, R3			; Add the offset to the start of video memory
			LEA R6, COLOR_FILL
			LDR R6, R6, #0			; now R6 = color to fill in
			STR R6, R0, #0			; Fill in the pixel R0 with color from user (R6 or COLOR_FILL)
			ADD R5, R5, #1			; col++
			LEA R6, INDEX_J
			STR R5, R6, #0			; save updated col to INDEX_J
			BRnzp BOX_LOOP_2			; go back to loop 2
		END_BOX_LOOP_2
		; need to load R5 value as INDEX_I
		LEA R6, INDEX_I
		LDR R5, R6, #0				; now R5 = INDEX_I, or R5 =row
		ADD R5, R5, #1				; row++
		STR R5, R6, #0				;  save updated row to INDEX_I
		BRnzp BOX_LOOP_1				;go back to loop 1
	END_BOX_LOOP_1
	;;End of program, but need to restore user input value to regfiles
	LEA R6, X_BOX
	LDR R1, R6, #0
	LEA R6, Y_BOX
	LDR R2, R6, #0
	LEA R6, COLOR_FILL
	LDR R0, R6, #0
	LEA R3, FRAME_POINTER
	LDR R5, R3, #0
	LEA R3, STACK_POINTER
	LDR R6, R3, #0
RTI			  ; PC = R7 ; PSR[15]=0



;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_TIMER   ;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function:		timer count down
;;; Inputs			 - R0: time interval to wait
;;; Outputs          - none

.CODE
LC R1, OS_TSR_ADDR
LDR R1, R1, #0		; R1 = timer status

TRAP_TIMER
	LC R1, OS_TIR_ADDR	; R1 = timer interval
	STR R0, R1, #0		; R1 = input from R0
	WAIT_TIR
		LC R1, OS_TSR_ADDR
		LDR R1, R1, #0		; R1 = timer status
		BRzp WAIT_TIR
RTI


;;;;;;;;;;;;;;;;;;;;;;;;;;;   TRAP_PUTS   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Function: Put a string out to ASCII display
;;; Inputs           - R0 = start address of the string
;;; Outputs          - put the string to ASCII display

.CODE
TRAP_PUTS
	LC R1, OS_ADSR_ADDR	; R1 = address of display status reg
	LDR R1, R1, #0		; R1 = value of status reg
	BRzp TRAP_PUTS		; If R1[15] = 1, data is ready to output, else continue to loop

	LC R1, OS_ADDR_ADDR	; R1 = address of display data reg
	LDR R2, R0, #0		; read char from R0 address to R2
	ADD R0, R0, #1		; R0 += 1
	STR R2, R1, #0		; write R2 to display
	CMPI R2, #0
	BRnp TRAP_PUTS		; if R2 is not '\0', loop to put the next char
	RTI














