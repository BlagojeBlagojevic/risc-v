#prereq  autoconf automake curl python3 mpc mpfr gmp gawk base-devel bison flex texinfo gperf libtool patchutils bc zlib expat
#riscv tool chain
deps:
	git clone https://github.com/riscv/riscv-gnu-toolchain
	cd ./risc-gnu-toolchain/
	./configure --prefix=/opt/riscv
	make && make linux


CFLAGS = -Og  -ggdb -fsanitize=address -static-libasan 

main: main.c
	gcc $(CFLAGS) main.c  -o main 

test.bin: test.c

	#-Ttext=0x80000000 
	riscv64-unknown-elf-gcc -march=rv64im -mabi=lp64 -ffreestanding -nostdlib -nostartfiles \
		-Wl,-Ttext=0x80000000 -Wl,--entry=main -O3 -o test.elf test.c
	
	#flat bin
	riscv64-unknown-elf-objcopy -O binary test.elf test.bin


all: main test.bin
	./main test.bin


clean:
	rm -f test
	rm -f test.bin
	rm -f test.s
	rm -f main kao organizovati linker za ovaj problem
