
mysoc: test.c soc.c soc.h curve.c curve.h
	gcc test.c soc.c curve.c  -lm -g -o mysoc