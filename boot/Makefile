BIN = ../bin
ASM = nasm -f bin

SRCS = stage1.asm stage2.asm
OUTPUT = $(addprefix $(BIN)/, $(SRCS:%.asm=%.bin))

all: $(OUTPUT)

$(BIN)/%.bin: %.asm
	$(ASM) $< -o $@

clean:
	-rm $(OUTPUT)
