import enum

class Instruction(enum.Enum):
    load = 0
    send_short = 1
    send_medium = 2
    send_long = 3
    start_timer = 4
    stop_timer = 5
    barrier_send = 6
    recv_medium = 7
    end = 8
    barrier_wait = 9
    send_long_fifo = 10
    send_medium_fifo = 11
    read_local = 12

class Instructions(object):
    def __init__(self):
        self.instructions = []

    def load(self, dst, token, handler, args_num, args, payloadsize, src_addr, dst_addr):
        self.instructions.append(Instruction.load.value)
        self.instructions.append(dst)
        self.instructions.append(token)
        self.instructions.append(handler)
        self.instructions.append(args_num)
        self.instructions.extend(args)
        self.instructions.append(payloadsize)
        self.instructions.append(src_addr)
        self.instructions.append(dst_addr)

    def write_instruction(self, name):
        assert isinstance(name, enum.Enum)
        self.instructions.append(name.value)

    def write_payload(self, value):
        if isinstance(value, list):
            self.instructions.extend(value)
        else:
            self.instructions.append(value)

    def write_coe(self, filename):
        with open(filename, 'w+') as f:
            f.write("memory_initialization_radix=10;\n")
            f.write("memory_initialization_vector=\n")
            instructions = [str(s) + ",\n" for s in self.instructions]
            instructions.append(str(Instruction.end.value) + ";\n")
            f.writelines(instructions)

    @staticmethod
    def byte_reverse(instruction):
        instr_str = "%08d" % instruction
        reverse_str = instr_str[6:8] + instr_str[4:6] + instr_str[2:4] + instr_str[0:2]
        return reverse_str + "\n"


    def write_sw_mem(self, filename):
        with open(filename, "w+") as f:
            f.write("@%08d\n" % 0)
            for instruction in self.instructions:
                f.write("%08d\n" % instruction)
            f.write("%08d\n" % Instruction.end.value)

    def write_mem(self, filename):
        with open(filename, "w+") as f:
            f.write("@%08d\n" % 0)
            for instruction in self.instructions:
                f.write(self.byte_reverse(instruction))
            for i in range(1024 - len(self.instructions)):
                f.write(self.byte_reverse(Instruction.end.value))

    def write_files(self, filename):
        self.write_coe(filename + ".coe")
        self.write_mem(filename + ".mem")
        self.write_sw_mem(filename + "_sw.mem")

if __name__ == "__main__":
    lst = Instructions()
    lst.write_instruction(Instruction.barrier_send)
    lst.write_payload(1)
    lst.load(1, 0, 0, 0, [], 8, 0, 0)
    lst.write_instruction(Instruction.send_medium_fifo)
    lst.write_payload(0xDEAD)
    lst.write_instruction(Instruction.recv_medium)
    lst.write_instruction(Instruction.recv_medium)
    lst.write_instruction(Instruction.recv_medium)
    lst.write_instruction(Instruction.recv_medium)
    lst.write_files("hls_kernel_0")

    lst_1 = Instructions()
    lst_1.write_instruction(Instruction.barrier_wait)
    lst_1.load(0, 1, 0, 0, [], 8, 0, 0)
    lst_1.write_instruction(Instruction.recv_medium)
    lst_1.write_instruction(Instruction.recv_medium)
    lst_1.write_instruction(Instruction.start_timer)
    lst_1.write_instruction(Instruction.send_medium_fifo)
    lst_1.write_payload(0xBEEF)
    lst_1.write_instruction(Instruction.stop_timer)
    lst_1.write_files("hls_kernel_1")
