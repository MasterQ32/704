; Ausstiegspunkt des Bootloaders
; nach einem geladenen Byte.
; Sollte irgendwann überschrieben werden
.ORG $007F
EXIT:
        JMP     LOOP    ; Rücksprung in den Bootloader

; Einstiegspunkt des Bootloaders
.ORG $0080
START:
        DOT     $E, $9  ; Lochstreifenleser wählen

; Warte auf "Zeichen empfangen"
LOOP:
        DIN     $E, $0  ; Status abfragen
        SRC L   1       ; Bit 7 → Bit 0
        SAM             ; Skip if Bit 0 set
        JMP     LOOP

        DIN     $E, $D  ; Lese ein Byte vom Lochstreifen
        STB I   $0      ; Speichere das Byte indexiert ab
        DXS     1       ; Prüfe auf "index <> 0", dann
        IXS     2       ; inkrementiere idx, sonst
        CAX             ; setzte "index = akk"
        JMP     EXIT    ; in den einstiegspunkt springen
