MODULO: 
BEGIN
SOMA_UM:	
MACRO       ; Definição de macro sem parâmetros (soma um)
		    LOAD    X
   ADD ONE
		    STORE x
ENDMACRO

TROCA: 		macro   &A,   &B 
; Definição de macro com parâmetros (troca valores)
		    COPY &A,    TEMP
		    COPY &B,&A
		    
			COPY TEMP, &B
       COPY &A, X
		    SOMA_UM ; Chamada aninhada de macro
		    COPY X, &A
		    COPY &B,     X
		    SOMA_UM ; Chamada aninhada de macro
		    COPY X, &B
	     ENDMACRO

		
		    INPUT VET
		    input VET+1
		    troca VET, VET + 1 ; Chamada de macro
		    OUTPUT VET
		    OUTPUT VET + 1
			STOP
vet:     
        SPACE 2
Temp:		SPACE
One:		CONST         1
X:		      
 SPACE
 END