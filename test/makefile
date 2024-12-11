
mysoc: test.o soc.o curve.o soh.o soe.o sop.o sox.o
	gcc 	test.o soc.o curve.o soh.o soe.o sop.o sox.o -o mysoc -lm -g

test.o: test.c ../src/sox.h
	gcc -I ../src -c test.c -o test.o -lm -g
soc.o: ../src/soc.c ../src/soc.h ../src/sox.h
	gcc -c ../src/soc.c -o soc.o -lm -g
curve.o: ../src/curve.c ../src/curve.h ../src/sox.h
	gcc -c ../src/curve.c -o curve.o -lm -g
soh.o: ../src/soh.c ../src/soh.h	../src/sox.h
	gcc -c ../src/soh.c -o soh.o -lm -g
soe.o: ../src/soe.c ../src/soe.h	../src/sox.h
	gcc -c ../src/soe.c -o soe.o -lm -g
sop.o: ../src/sop.c ../src/sop.h	../src/sox.h
	gcc -c ../src/sop.c -o sop.o -lm -g
sox.o: ../src/sox.c ../src/sox.h	../src/soc.h ../src/soh.h ../src/soe.h ../src/sop.h
	gcc -c ../src/sox.c -o sox.o -lm -g

