CC = g++
CFLAGS  = -g -Wall -Iinclude \
	-I/media/sharm294/HDD_1TB/Xilinx/Vivado_HLS/2017.2/include \
	-Wno-unknown-pragmas -Wno-comment

OBJ_DIR = obj
BIN_DIR = bin
SRC_DIR = src
TEST_DIR = testbench

HELPER_OBJS = globals.o packet_defs.o
HELPER_OBJS_PATH = $(OBJ_DIR)/globals.o $(OBJ_DIR)/packet_defs.o

init:
	@mkdir -p obj
	@mkdir -p bin

test: am_rx_tb
	python $(TEST_DIR)/am_rx_generate.py
	./$(BIN_DIR)/am_rx_tb

test-v1: am_rx_tb
	python $(TEST_DIR)/am_rx_generate.py
	./$(BIN_DIR)/am_rx_tb -v

am_rx_tb: am_rx_tb.o am_rx.o $(HELPER_OBJS)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/am_rx_tb $(OBJ_DIR)/am_rx_tb.o $(OBJ_DIR)/am_rx.o $(HELPER_OBJS_PATH)

#Object Files

am_rx_tb.o: $(TEST_DIR)/am_rx_tb.cpp
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/am_rx_tb.o -c $(TEST_DIR)/am_rx_tb.cpp

am_rx.o: $(SRC_DIR)/am_rx.cpp
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/am_rx.o -c $(SRC_DIR)/am_rx.cpp

globals.o: $(SRC_DIR)/globals.cpp
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/globals.o -c $(SRC_DIR)/globals.cpp

packet_defs.o: $(SRC_DIR)/packet_defs.cpp
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/packet_defs.o -c $(SRC_DIR)/packet_defs.cpp

clean: 
	@$(RM) $(BIN_DIR)/* $(OBJ_DIR)/*.o