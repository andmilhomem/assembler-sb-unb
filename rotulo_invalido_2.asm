            JMP	    INICIO
MULTIPLICA:	LOAD 	A
		    MULT 	B
		    STORE 	R
INICIO:		INPUT	1A
		    INPUT	B
		    MULTIPLICA
            OUTPUT R
A:          SPACE
B:          SPACE
R:          SPACE