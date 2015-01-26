;; Square root program
;; A = R0, B = R1, C = B * B. R1 stores the final result of sqrt(A)
;; A to be assigned a value by script file

.CODE
.ADDR x0000

.FALIGN
CONST R1 #0		;; B = 0
CMPI R0 #0 		;; NZP = A - 0
BRn end			;; if (A >= 0)
while_loop
	MUL R2 R1 R1	;; C = B * B
	CMP R2 R0		;; NZP = B * B - A
	BRp end		;; while (B * B <= A)
	ADD R1 R1 #1	;; B = B + 1
	BRnzp while_loop	;; end while loop
end
ADD R1 R1 #-1		;; B = B - 1
infinite_loop
	BRnzp infinite_loop
;; End sqrt calculator


;; Include a simple OS that just RTIs back to user code from the default PennSim entry point of x8200.
;;.FALIGN
.OS
.CODE
.ADDR x8200
.FALIGN
	CONST R7, #0
	RTI

