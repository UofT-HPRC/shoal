import enum
import os

class Instruction(enum.Enum):
    load = 0
    short_latency = 1
    medium_latency = 2
    long_latency = 3
    barrier_send = 4
    recv_medium = 5
    end = 6
    barrier_wait = 7
    long_fifo_latency = 8
    medium_fifo_latency = 9
    read_local = 10
    recv_time = 11
    add_label = 12

class Instructions(object):
    def __init__(self):
        self.instructions = []

    def load(self, handler, args_num, args, payloadsize, src_addr, dst_addr):
        """
        Load values

        Args:
            handler (int): handler
            args_num (int): args_num
            args (int): args
            payloadsize (int): payloadsize
            src_addr (int): src_addr
            dst_addr (int): dst_addr
        """
        self.instructions.append(Instruction.load.value)
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
    def byte_reverse(instruction, reverse=True):
        instr_str = instruction
        if reverse:
            reverse_str = instr_str[6:8] + instr_str[4:6] + instr_str[2:4] + instr_str[0:2]
        else:
            reverse_str = instr_str
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
                f.write(self.byte_reverse("{:08x}".format(instruction)))
            for i in range(1024 - len(self.instructions)):
                f.write(self.byte_reverse("{:08x}".format(Instruction.end.value)))

    def write_files(self, filename):
        path = os.environ["SHOAL_PATH"] + "/tests/build/" + filename
        self.write_coe(path + ".coe")
        self.write_mem(path + ".mem")
        self.write_sw_mem(path + "_sw.mem")

    def add_label(self, label, meta):
        self.write_instruction(Instruction.add_label)
        self.write_payload(label.value)
        self.write_payload(meta)

LATENCY_ITERATIONS = 1000
PAYLOAD_MIN = 0
PAYLOAD_MAX = 10 # 2^(10-1) = 512

def kern0():
    lst = Instructions()
    lst.load(0, 0, [], 0, 0, 0)
    lst.write_instruction(Instruction.barrier_wait)

    lst.add_label(Instruction.short_latency, 0)
    lst.write_instruction(Instruction.recv_time)
    lst.write_payload(LATENCY_ITERATIONS)
    lst.write_instruction(Instruction.barrier_wait)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.add_label(Instruction.medium_latency, i)
        lst.write_instruction(Instruction.recv_time)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_wait)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.add_label(Instruction.medium_fifo_latency, i)
        lst.write_instruction(Instruction.recv_time)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_wait)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.add_label(Instruction.long_latency, i)
        lst.write_instruction(Instruction.recv_time)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_wait)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.add_label(Instruction.long_fifo_latency, i)
        lst.write_instruction(Instruction.recv_time)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_wait)

    lst.write_files("benchmark_0")

def kern1():
    lst = Instructions()
    lst.load(0, 0, [], 0, 0, 0)
    lst.write_instruction(Instruction.barrier_send)

    lst.write_instruction(Instruction.short_latency)
    lst.write_payload(LATENCY_ITERATIONS)
    lst.write_instruction(Instruction.barrier_send)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.medium_latency)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_send)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.medium_fifo_latency)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_send)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.long_latency)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_send)

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.long_fifo_latency)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_send)

    lst.write_files("benchmark_1")

def kern2():
    lst = Instructions()
    lst.load(0, 0, [], 0, 0, 0)
    lst.write_instruction(Instruction.barrier_send)

    # short latency
    lst.write_instruction(Instruction.barrier_send)

    # medium latency
    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.recv_medium)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_send)

    # medium fifo latency
    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.recv_medium)
        lst.write_payload(LATENCY_ITERATIONS)
        lst.write_instruction(Instruction.barrier_send)

    # long latency
    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], 0, 0, 0)
        lst.write_instruction(Instruction.barrier_send)

    # long fifo latency
    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], 0, 0, 0)
        lst.write_instruction(Instruction.barrier_send)

    lst.write_files("benchmark_2")

def test_kern_0():
    lst = Instructions()
    lst.load(0, 0, [], 0, 0, 0)
    lst.write_files("benchmark_test_0")

def test_kern_1():
    lst = Instructions()
    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.medium_latency)
        lst.write_payload(LATENCY_ITERATIONS)

    lst.write_files("benchmark_test_1")

def test_kern_2():
    lst = Instructions()
    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        lst.load(0, 0, [], (2**i)*8, 0, 0)
        lst.write_instruction(Instruction.recv_medium)
        lst.write_payload(LATENCY_ITERATIONS)

    lst.write_files("benchmark_test_2")

if __name__ == "__main__":
    kern0()
    kern1()
    kern2()
    test_kern_0()
    test_kern_1()
    test_kern_2()
