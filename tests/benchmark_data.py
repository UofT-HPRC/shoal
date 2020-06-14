import enum
import os
import argparse

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
    short_throughput = 13
    medium_throughput = 14
    long_throughput = 15
    medium_fifo_throughput = 16
    long_fifo_throughput = 17
    strided_latency = 18
    strided_throughput = 19
    vector_latency = 20
    vector_throughput = 21
    load_stride = 22
    load_vector = 23
    wait_counter = 24
    busy_loop = 25
    send_pilot = 26
    recv_pilot = 27

class Instructions(object):
    def __init__(self):
        self.instructions = []

    def load(self, handler, args_num, args, payloadsize, src_addr, dst_addr):
        """
        Load values

        Args:
            handler (int): handler
            args_num (int): args_num
            args (list): args
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

    def load_strided(self, src_stride, src_blk_size, src_blk_num, dst_stride, dst_blk_size, dst_blk_num):
        """
        Load strided values

        Args:
            src_stride (int): src stride
            src_blk_size (int): src block size
            src_blk_num (int): src block num
            dst_stride (int): dst stride
            dst_blk_size (int): dst block size
            dst_blk_num (int): dst block num
        """
        self.instructions.append(Instruction.load_stride.value)
        self.instructions.append(src_stride)
        self.instructions.append(src_blk_size)
        self.instructions.append(src_blk_num)
        self.instructions.append(dst_stride)
        self.instructions.append(dst_blk_size)
        self.instructions.append(dst_blk_num)

    def load_vectored(self, srcVectorCount, dstVectorCount, srcSize, src_addrs, dstSize, dst_addrs):
        """
        Load vectored values

        Args:
            srcVectorCount (int): src vector count
            dstVectorCount (int): dst vector count
            srcSize (list): src vector sizes
            src_addrs (list): src vector addresses
            dstSize (list): dst vector sizes
            dst_addrs (list): dst vector addresses
        """
        self.instructions.append(Instruction.load_vector.value)
        self.instructions.append(srcVectorCount)
        self.instructions.append(dstVectorCount)
        self.instructions.extend(srcSize)
        self.instructions.extend(src_addrs)
        self.instructions.extend(dstSize)
        self.instructions.extend(dst_addrs)

    def send_pilot(self, dest):
        self.load(0, 0, [], 8, 0, 0)
        self.write_instruction(Instruction.send_pilot)
        self.write_payload(dest)

    def recv_pilot(self):
        self.load(0, 0, [], 8, 0, 0)
        self.write_instruction(Instruction.recv_pilot)

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

def prologue(kern0, kern1, kern2):
    kern0.load(0, 0, [], 0, 0, 0)
    kern0.write_instruction(Instruction.barrier_wait)

    kern1.load(0, 0, [], 0, 0, 0)
    kern1.write_instruction(Instruction.barrier_send)

    kern2.load(0, 0, [], 0, 0, 0)
    kern2.write_instruction(Instruction.barrier_send)


def short_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.short_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.short_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS
    
    kern0.add_label(instruction, 0)
    kern0.write_instruction(Instruction.recv_time)
    kern0.write_payload(time_iterations)
    kern0.write_instruction(Instruction.barrier_wait)
    
    kern1.recv_pilot()
    kern1.write_instruction(instruction)
    kern1.write_payload(iterations)
    kern1.write_instruction(Instruction.barrier_send)

    kern2.send_pilot(1)
    kern2.write_instruction(Instruction.barrier_send)

def medium_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.medium_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.medium_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        kern0.add_label(instruction, i)
        kern0.write_instruction(Instruction.recv_time)
        kern0.write_payload(time_iterations)
        kern0.write_instruction(Instruction.barrier_wait)

        kern1.recv_pilot()
        kern1.load(0, 0, [], (2**i)*8, 0, 0)
        kern1.write_instruction(instruction)
        kern1.write_payload(iterations)
        kern1.write_instruction(Instruction.barrier_send)

        kern2.send_pilot(1)
        kern2.load(0, 0, [], (2**i)*8, 0, 0)
        kern2.write_instruction(Instruction.recv_medium)
        if throughput:
            kern2.write_payload(iterations * THROUGHPUT_TIMES)
        else:
            kern2.write_payload(iterations)
        kern2.write_instruction(Instruction.barrier_send)

def medium_fifo_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.medium_fifo_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.medium_fifo_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        kern0.add_label(instruction, i)
        kern0.write_instruction(Instruction.recv_time)
        kern0.write_payload(time_iterations)
        kern0.write_instruction(Instruction.barrier_wait)

        kern1.recv_pilot()
        kern1.load(0, 0, [], (2**i)*8, 0, 0)
        kern1.write_instruction(instruction)
        kern1.write_payload(iterations)
        kern1.write_instruction(Instruction.barrier_send)

        kern2.send_pilot(1)
        kern2.load(0, 0, [], (2**i)*8, 0, 0)
        kern2.write_instruction(Instruction.recv_medium)
        if throughput:
            kern2.write_payload(iterations * THROUGHPUT_TIMES)
        else:
            kern2.write_payload(iterations)
        kern2.write_instruction(Instruction.barrier_send)

def long_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.long_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.long_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        kern0.add_label(instruction, i)
        kern0.write_instruction(Instruction.recv_time)
        kern0.write_payload(time_iterations)
        kern0.write_instruction(Instruction.barrier_wait)

        kern1.recv_pilot()
        kern1.load(0, 0, [], (2**i)*8, 0, 0)
        kern1.write_instruction(instruction)
        kern1.write_payload(iterations)
        kern1.write_instruction(Instruction.barrier_send)

        kern2.send_pilot(1)
        kern2.load(0, 0, [], 0, 0, 0)
        kern2.write_instruction(Instruction.barrier_send)

def long_fifo_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.long_fifo_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.long_fifo_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS

    for i in range(PAYLOAD_MIN,PAYLOAD_MAX):
        kern0.add_label(instruction, i)
        kern0.write_instruction(Instruction.recv_time)
        kern0.write_payload(time_iterations)
        kern0.write_instruction(Instruction.barrier_wait)

        kern1.recv_pilot()
        kern1.load(0, 0, [], (2**i)*8, 0, 0)
        kern1.write_instruction(instruction)
        kern1.write_payload(iterations)
        kern1.write_instruction(Instruction.barrier_send)

        kern2.send_pilot(1)
        kern2.load(0, 0, [], 0, 0, 0)
        kern2.write_instruction(Instruction.barrier_send)

def strided_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.strided_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.strided_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS

    # src_stride, src_blk_size, src_blk_num, dst_stride, dst_blk_size, dst_blk_num
    stride_combos = [
        # (16, 8, 4, 8, 32, 1) # 0, 2, 4, 6, 0, 0, 0, 0, 0, 0
        (8, 32, 1, 16, 8, 4) # 0, 0, 1, 0, 2, 0, 3, 0, 0, 0
    ]
    for i, stride_combo in enumerate(stride_combos):
        kern0.add_label(instruction, i)
        kern0.write_instruction(Instruction.recv_time)
        kern0.write_payload(time_iterations)
        kern0.write_instruction(Instruction.barrier_wait)

        payload = stride_combo[1] * stride_combo[2]
        assert stride_combo[1] * stride_combo[2] == stride_combo[4] * stride_combo[5]
        kern1.recv_pilot()
        kern1.load(0, 0, [], payload, 0, 0)
        kern1.load_strided(*stride_combo)
        kern1.write_instruction(instruction)
        kern1.write_payload(iterations)
        kern1.write_instruction(Instruction.barrier_send)

        kern2.send_pilot(1)
        kern2.load(0, 0, [], 0, 0, 0)
        kern2.write_instruction(Instruction.barrier_send)

def vectored_latency(kern0, kern1, kern2, throughput=False):
    if throughput:
        instruction = Instruction.vector_throughput
        iterations = THROUGHPUT_ITERATIONS
        time_iterations = THROUGHPUT_TIMES
    else:
        instruction = Instruction.vector_latency
        iterations = LATENCY_ITERATIONS
        time_iterations = LATENCY_ITERATIONS

    # srcVectorCount, dstVectorCount, srcSize, src_addrs, dstSize, dst_addrs
    vector_combos = [
        (2, 2, [16, 8], [0, 32], [16, 8], [0, 32]), # 0, 1, 0, 0, 4, 0, 0, 0, 0, 0
        (2, 1, [16, 8], [0, 32], [24], [0]), # 0, 1, 4, 0, 0, 0, 0, 0, 0, 0
        (1, 2, [24], [8], [8, 16], [0, 32]), # 1, 0, 0, 0, 2, 3, 0, 0, 0, 0
    ]
    for i, vector_combo in enumerate(vector_combos):
        kern0.add_label(instruction, i)
        kern0.write_instruction(Instruction.recv_time)
        kern0.write_payload(time_iterations)
        kern0.write_instruction(Instruction.barrier_wait)

        payload = sum(vector_combo[2])
        assert sum(vector_combo[2]) == sum(vector_combo[4])
        kern1.recv_pilot()
        kern1.load(0, 0, [], payload, 0, 0)
        kern1.load_vectored(*vector_combo)
        kern1.write_instruction(instruction)
        kern1.write_payload(iterations)
        kern1.write_instruction(Instruction.barrier_send)

        kern2.send_pilot(1)
        kern2.load(0, 0, [], 0, 0, 0)
        kern2.write_instruction(Instruction.barrier_send)


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

THROUGHPUT_TIMES = 2
LATENCY_ITERATIONS = 100000
THROUGHPUT_ITERATIONS = 100000
PAYLOAD_MIN = 0
PAYLOAD_MAX = 10 # 2^(10-1) = 512

if __name__ == "__main__":
    test_kern_0()
    test_kern_1()
    test_kern_2()

    kern0 = Instructions()
    kern1 = Instructions()
    kern2 = Instructions()

    parser = argparse.ArgumentParser(description="Generate data for benchmark")
    parser.add_argument("mode", type=str, help="latency, throughput, or all", default="latency")
    parser.add_argument("--test", type=str, help="type of tests to run", default=["all"], nargs="+")
    parser.add_argument("--iterations", type=int, help="number of times to repeat", default=100000)
    parser.add_argument("--payload_min", type=int, help="2^x min", default=0)
    parser.add_argument("--payload_max", type=int, help="2^x max (non-inclusive)", default=10)

    args = parser.parse_args()

    LATENCY_ITERATIONS = args.iterations
    THROUGHPUT_ITERATIONS = args.iterations
    PAYLOAD_MIN = args.payload_min
    PAYLOAD_MAX = args.payload_max

    prologue(kern0, kern1, kern2)

    if(args.mode in ["all", "latency"]):
        if bool(set(args.test) & set(["all", "short"])):
            short_latency(kern0, kern1, kern2)
        if bool(set(args.test) & set(["all", "medium"])):
            medium_latency(kern0, kern1, kern2)
        if bool(set(args.test) & set(["all", "medium_fifo"])):
            medium_fifo_latency(kern0, kern1, kern2)
        if bool(set(args.test) & set(["all", "long"])):
            long_latency(kern0, kern1, kern2)
        if bool(set(args.test) & set(["all", "long_fifo"])):
            long_fifo_latency(kern0, kern1, kern2)
        if bool(set(args.test) & set(["all", "strided"])):
            strided_latency(kern0, kern1, kern2)
        if bool(set(args.test) & set(["all", "vector"])):
            vectored_latency(kern0, kern1, kern2)
    if(args.mode in ["all", "throughput"]):
        if bool(set(args.test) & set(["all", "short"])):
            short_latency(kern0, kern1, kern2, True)
        if bool(set(args.test) & set(["all", "medium"])):
            medium_latency(kern0, kern1, kern2, True)
        if bool(set(args.test) & set(["all", "medium_fifo"])):
            medium_fifo_latency(kern0, kern1, kern2, True)
        if bool(set(args.test) & set(["all", "long"])):
            long_latency(kern0, kern1, kern2, True)
        if bool(set(args.test) & set(["all", "long_fifo"])):
            long_fifo_latency(kern0, kern1, kern2, True)
        if bool(set(args.test) & set(["all", "strided"])):
            strided_latency(kern0, kern1, kern2, True)
        if bool(set(args.test) & set(["all", "vector"])):
            vectored_latency(kern0, kern1, kern2, True)

    kern0.write_files("benchmark_0")
    kern1.write_files("benchmark_1")
    kern2.write_files("benchmark_2")
