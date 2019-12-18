TARGET ?= mipsel-linux-gnu

# disable builtin rules
MAKEFLAGS += --no-builtin-rules
.SECONDARY:

.PHONY: all
all: testbench.bin testbench.gen.s

%.c.o: %.c *.h
	$(TARGET)-gcc -c $< -EL -o $@ -ffreestanding -nostdlib -mips32r2 -mxgot -g -DROUTER_BACKEND_XILINX -I.

testbench.o: testbench.c.o io.c.o lib.c.o xil_printf.c.o router_hal.c.o boot.lds
	$(TARGET)-ld -EL $^ -T boot.lds -o $@

%.bin: %.o
	$(TARGET)-objcopy -O binary $^ -j .text -j .rodata -j .data -j .got $@

%.gen.s: %.o
	$(TARGET)-objdump -D $^ -M no-aliases,gpr-names=numeric > $@

.PHONY: clean
clean:
	rm -f *.bin *.o *.gen.s *.s.o Router-Lab/HAL/src/xilinx/router_hal.c.o

