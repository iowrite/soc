.PHONY:  clean build_dir

all: mysoc 

build_dir:
	mkdir -p build_dir
clean:
	rm  -rf build_dir


mysoc: 	build_dir/test.o \
		build_dir/port_linux.o \
		libmyekfsoc.a

	clang -Wl,--gc-sections,-Map=output.map \
			build_dir/test.o \
			build_dir/port_linux.o \
			-L . -lmyekfsoc -lm -lxlsxio_read -o mysoc

libmyekfsoc.a:	build_dir/soc.o \
				build_dir/curve.o \
				build_dir/soh.o \
				build_dir/soe.o \
				build_dir/sop.o \
				build_dir/sox.o \
				build_dir/port.o \
				build_dir/debug.o \
				build_dir/common.o
	llvm-ar rcs libmyekfsoc.a \
			build_dir/soc.o \
			build_dir/curve.o \
			build_dir/soh.o \
			build_dir/soe.o \
			build_dir/sop.o \
			build_dir/sox.o \
			build_dir/port.o \
			build_dir/debug.o \
			build_dir/common.o 


CFLAGS 		= -xc -std=gnu99 -Wall -Wextra -ffunction-sections -fdata-sections -O0 -g
CLIBFLAGS 	= -xc -std=c99 -Wall -Wextra -ffunction-sections -fdata-sections -O0 -g -fPIC

build_dir/test.o: test.c build_dir/test.d
	clang $(CFLAGS) -I ../src -c  $<  -o $@
	
build_dir/port_linux.o: ./port/port_linux.c build_dir/port_linux.d
	clang $(CFLAGS) -I ../src -c  $<  -o $@

build_dir/port.o: ../src/port.c build_dir/port.d
	clang $(CLIBFLAGS) -c  $<  -o $@  

build_dir/debug.o: ../src/debug.c
	clang $(CLIBFLAGS) -c  $<  -o $@

build_dir/soc.o: ../src/soc.c build_dir/soc.d
	clang $(CLIBFLAGS) -c  $<  -o $@   

build_dir/curve.o: ../src/curve.c build_dir/curve.d
	clang $(CLIBFLAGS) -c  $<  -o $@  

build_dir/soh.o: ../src/soh.c build_dir/soh.d
	clang $(CLIBFLAGS) -c  $<  -o $@  

build_dir/soe.o: ../src/soe.c build_dir/soe.d
	clang $(CLIBFLAGS) -c  $< -o $@ 

build_dir/sop.o: ../src/sop.c build_dir/sop.d
	clang $(CLIBFLAGS) -c  $<  -o $@  

build_dir/sox.o: ../src/sox.c build_dir/sox.d
	clang $(CLIBFLAGS) -c  $<  -o $@  

build_dir/common.o: ../src/common.c build_dir/common.d
	clang $(CLIBFLAGS) -c  $< -o $@   




build_dir/test.d: test.c | build_dir
	clang -I ../src -M test.c -MF build_dir/test.d -MT build_dir/test.o
build_dir/port_linux.d: ./port/port_linux.c | build_dir
	clang -I ../src -M ./port/port_linux.c -MF build_dir/port_linux.d -MT build_dir/port_linux.o
build_dir/port.d: ../src/port.c | build_dir
	clang -M ../src/port.c -MF build_dir/port.d -MT build_dir/port.o
build_dir/soc.d: ../src/soc.c | build_dir
	clang -M ../src/soc.c -MF build_dir/soc.d -MT build_dir/soc.o
build_dir/curve.d: ../src/curve.c | build_dir
	clang -M ../src/curve.c -MF build_dir/curve.d -MT build_dir/curve.o
build_dir/soh.d: ../src/soh.c | build_dir
	clang -M ../src/soh.c -MF build_dir/soh.d -MT build_dir/soh.o
build_dir/soe.d: ../src/soe.c | build_dir
	clang -M ../src/soe.c -MF build_dir/soe.d -MT build_dir/soe.o
build_dir/sop.d: ../src/sop.c | build_dir
	clang -M ../src/sop.c -MF build_dir/sop.d -MT build_dir/sop.o
build_dir/sox.d: ../src/sox.c | build_dir	
	clang -M ../src/sox.c -MF build_dir/sox.d -MT build_dir/sox.o
build_dir/common.d: ../src/common.c | build_dir
	clang -M ../src/common.c -MF build_dir/common.d -MT build_dir/common.o



-include build_dir/test.d
-include build_dir/port_linux.d
-include build_dir/port.d
-include build_dir/soc.d	
-include build_dir/curve.d
-include build_dir/soh.d
-include build_dir/soe.d
-include build_dir/sop.d
-include build_dir/sox.d
-include build_dir/common.d


