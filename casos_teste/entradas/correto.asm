MODULO:     BEGIN
SOMA_UM:	MACRO ; Definição de macro sem parâmetros (soma um)
		    LOAD X
		    ADD ONE
		    STORE X
		    ENDMACRO
TROCA: 		MACRO &A, &B ; Definição de macro com parâmetros (troca valores)
		    COPY &A, TEMP
		    COPY &B, &A
		    COPY TEMP, &B
		    COPY &A, X
		    SOMA_UM ; Chamada aninhada de macro
		    COPY X, &A
		    COPY &B, X
		    SOMA_UM ; Chamada aninhada de macro
		    COPY X, &B
		    ENDMACRO
		    INPUT VET
		    INPUT VET + 1
		    TROCA VET, VET + 1 ; Chamada de macro
		    OUTPUT VET
		    OUTPUT VET + 1
            STOP
VET:		SPACE 2
TEMP:		SPACE
ONE:		CONST 1
X:		    SPACE
            END