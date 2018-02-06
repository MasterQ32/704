; CALL → JSX PUTC
; RET  → JMP $800
.EQU PUTC $0080

.ORG $0000
START:
	LLB 'H'
	JSX PUTC
	LLB 'I'
	JSX PUTC
	HLT

;.ORG $0080
;PUTC:
;	DOT     $E, $E    ; Ausgabe des Zeichens
;	
;PUTC_LOOP:
;	DIN     $E, $0    ; Status abfragen
;	SRC L   1         ; Bit 7 → Bit 0
;	SAM               ; Skip if Bit 0 set
;	JMP     PUTC_LOOP
;	
;	JMP I   $000

