            JMP	    INICIO
MULTIPLICA:	LOAD 	A
		    MULT 	B
		    STORE 	R
INI$IO:		INPUT	A
		    INPUT	B
		    MULTIPLICA
            OUTPUT R
A:          SPACE
B:          SPACE
R:          SPACE