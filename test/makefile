.PHONY:  all clean 


all: build_dir mysoc

build_dir:
	mkdir -p build_dir
clean:
	rm  -rf build_dir

mysoc: build_dir/test.o build_dir/soc.o build_dir/curve.o build_dir/soh.o build_dir/soe.o build_dir/sop.o build_dir/sox.o build_dir/timebase.o
	gcc  build_dir/test.o build_dir/soc.o build_dir/curve.o build_dir/soh.o build_dir/soe.o build_dir/sop.o build_dir/sox.o build_dir/timebase.o -o mysoc -lm -g



build_dir/test.o: test.c ../src/sox.h
	gcc -I ../src -c  test.c -o build_dir/test.o -lm -g

build_dir/timebase.o: ../src/timebase.c ../src/timebase.h
	gcc -c  ../src/timebase.c -o build_dir/timebase.o -lm -g
build_dir/soc.o: ../src/soc.c ../src/soc.h ../src/sox.h
	gcc -c  ../src/soc.c -o build_dir/soc.o -lm -g
build_dir/curve.o: ../src/curve.c ../src/curve.h ../src/sox.h
	gcc -c  ../src/curve.c -o build_dir/curve.o -lm -g
build_dir/soh.o: ../src/soh.c ../src/soh.h	../src/sox.h
	gcc -c  ../src/soh.c -o build_dir/soh.o -lm -g
build_dir/soe.o: ../src/soe.c ../src/soe.h	../src/sox.h
	gcc -c  ../src/soe.c -o build_dir/soe.o -lm -g
build_dir/sop.o: ../src/sop.c ../src/sop.h	../src/sox.h
	gcc -c  ../src/sop.c -o build_dir/sop.o -lm -g
build_dir/sox.o: ../src/sox.c ../src/sox.h	../src/soc.h ../src/soh.h ../src/soe.h ../src/sop.h
	gcc -c  ../src/sox.c -o build_dir/sox.o -lm -g




