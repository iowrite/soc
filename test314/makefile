.PHONY:  clean

all: mysoc 

build_dir:
	mkdir -p build_dir
clean:
	rm -rf build_dir
	rm -rf ../src/build_dir 



USER_CONFIG='"/home/hm/Desktop/mysoc/test314/sox_user_config.h"'

export USER_CONFIG


mysoc: 	build_dir/test.o \
		build_dir/port_linux.o \
		../src/build_dir/libmyekfsoc.a

	clang -Wl,--gc-sections,-Map=output.map \
			build_dir/test.o \
			build_dir/port_linux.o \
			-L ../src/build_dir -lmyekfsoc -lm -lxlsxio_read -o mysoc




CFLAGS 		= -xc -std=gnu99 -Wall -Wextra -ffunction-sections -fdata-sections -O0 -g
INCLUDE     = ../src

build_dir/test.o: test.c build_dir/test.d
	clang -DSOX_USER_CONFIG_FILE=$(USER_CONFIG) $(CFLAGS) -I $(INCLUDE) -c  $<  -o $@
	
build_dir/port_linux.o: ./port/port_linux.c build_dir/port_linux.d
	clang -DSOX_USER_CONFIG_FILE=$(USER_CONFIG) $(CFLAGS) -I $(INCLUDE) -c  $<  -o $@


build_dir/test.d: test.c | build_dir
	clang -DSOX_USER_CONFIG_FILE=$(USER_CONFIG)  -I $(INCLUDE) -M $< -MF $@ -MT build_dir/test.o
build_dir/port_linux.d: ./port/port_linux.c | build_dir
	clang -DSOX_USER_CONFIG_FILE=$(USER_CONFIG) -I $(INCLUDE) -M $< -MF $@ -MT build_dir/port_linux.o

../src/build_dir/libmyekfsoc.a:
	make -C ../src 


-include build_dir/test.d
-include build_dir/port_linux.d



