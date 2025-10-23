            JMP	    INICIO
MULTIPLICA:	LOAD 	A
		    MULT 	B
		    STORE 	R
INICIO:		INPUT	A
		    INPUT	B
		    MULTIPLICA
            OUTPUT R
A:  B:      SPACE
R:          SPACE