; CALL → JSX PUTC
; RET  → JMP $800
.EQU PUTC $0080

START:
	LDX IDX_INIT
LOOP:
	CLR		; Lösche Akku
	LDB I	0	; Lade Byte von IDX (Lädt wirklich NUR das byte)
	CLB	0	; Vgl. mit 0
	SNE		; Skip, wenn acc(r) != 0
	JMP	TERMINATE	; Halte
	IXS	1	; Index++, Skip if positive
	NOP		; Nur inkrementieren
	
	STX	TEMP	; Indexregister sichern
	JSX 	PUTC	; Gebe Byte aus (macht index kaputt!)
	LDX	TEMP	; Indexregister wiederherstellen
	
	JMP 	LOOP	; Loop!
TEMP:
	.WORD 0

TERMINATE:
	JMP TERMINATE

.ORG $0040
IDX_INIT:
	.WORD TEXT
TEXT:
	.BYTE $D, $A, "Hallo Welt!", $00

.ORG $0080
PUTC:
	DOT     $E, $E    ; Ausgabe des Zeichens
	
PUTC_LOOP:
	DIN     $E, $0    ; Status abfragen
	SRC L   1         ; Bit 7 → Bit 0
	SAM               ; Skip if Bit 0 set
	JMP     PUTC_LOOP
	
	JMP I   $000

