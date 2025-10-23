            JMP	    INICIO
MULTIPLICA:	LOAD 	A
		    MULT 	B
		    STORE 	R
INICIO:		INPUT	A, B
		    MULTIPLICA
            OUTPUT R
A:          SPACE
B:          SPACE
R:          SPACE