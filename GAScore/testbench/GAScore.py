import os

from sonar.testbench import Testbench, TestVector, Thread, Module
from sonar.interfaces import AXIS, SAXILite

from sonar_strToInt import strToInt

GAScore = Testbench.default('GAScore')
GAScore.set_metadata("Flag_Count", 2)
filepath = os.path.join(os.path.dirname(__file__), 'build/GAScore/')

dut = Module.default("DUT")
dut.add_clock_port('clock', '20ns')
dut.add_reset_port('reset_n')
# dut.add_port('interrupt_0', 'output')
# dut.add_port('interrupt_1', 'output')

axis_kernel_out = AXIS('axis_kernel_out', 'master', 'clock')
axis_kernel_out.port.init_channels('tkeep', 64, False)
axis_kernel_out.port.add_channel('tdest', 'tdest', 16)
dut.add_interface(axis_kernel_out)

axis_kernel_in = AXIS('axis_kernel_in', 'slave', 'clock')
axis_kernel_in.port.init_channels('tkeep', 64, False)
dut.add_interface(axis_kernel_in)

axis_net_in = AXIS('axis_net_in', 'slave', 'clock')
axis_net_in.port.init_channels('tkeep', 64, False)
dut.add_interface(axis_net_in)

axis_net_out = AXIS('axis_net_out', 'master', 'clock')
axis_net_out.port.init_channels('tkeep', 64, False)
dut.add_interface(axis_net_out)

ctrl_bus_0 = SAXILite('s_axi_ctrl_bus_00', 'clock', 'reset_n')
ctrl_bus_0.add_register('config', 0x10)
ctrl_bus_0.add_register('arg', 0x18)
ctrl_bus_0.add_register('counter', 0x20)
ctrl_bus_0.add_register('barrier', 0x28)
ctrl_bus_0.add_register('memory', 0x30)
ctrl_bus_0.set_address('4K', 0)
ctrl_bus_0.port.init_channels(mode='default', dataWidth=32, addrWidth=12)
dut.add_interface(ctrl_bus_0)

ctrl_bus_1 = SAXILite('s_axi_ctrl_bus_01', 'clock', 'reset_n')
ctrl_bus_1.add_register('config', 0x10)
ctrl_bus_1.add_register('arg', 0x18)
ctrl_bus_1.add_register('counter', 0x20)
ctrl_bus_1.add_register('barrier', 0x28)
ctrl_bus_1.add_register('memory', 0x30)
ctrl_bus_1.set_address('4K', 0)
ctrl_bus_1.port.init_channels(mode='default', dataWidth=32, addrWidth=12)
dut.add_interface(ctrl_bus_1)
GAScore.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_timer()
initT.init_signals()
initT.wait_negedge('clock')
initT.add_delay('40ns')
initT.set_signal('reset_n', 1)
initT.set_signal('axis_net_out_tready', 1)
initT.set_signal('axis_kernel_out_tready', 1)

#-------------------------------------------------------------------------------
# Short Message A
#
# Sends short messages from the network that call the adder handler function
# that raises the interrupt once the set threshold is exceeded. There are two
# kernels here. A short message calling no handler is also inserted which does
# nothing and is just absorbed by the GAScore. All transactions result in reply
# messages sent back over the network.
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t2 = Thread()
smA_t2.add_delay('100ns')

smA_t2.print_elapsed_time('STAT_sma_2_0')
# ctrl_bus_1.write(smA_t2, 'counter_threshold', 4)
smA_t2.print_elapsed_time('STAT_sma_2_1')
# ctrl_bus_1.write(smA_t2, 'config_handler', 2)


axis_net_in.write(smA_t2, strToInt("{AMHeader,0x10,0x01,0,2,1,1}"))
axis_net_in.write(smA_t2, strToInt("{AMToken,0x0}"))
axis_net_in.write(smA_t2, 4, tlast=1)


axis_net_in.write(smA_t2, strToInt("{AMHeader,0x10,0x01,0,2,1,1}"))
axis_net_in.write(smA_t2, strToInt("{AMToken,0x1}"))
axis_net_in.write(smA_t2, 2, tlast=1)

# ctrl_bus_0.write(smA_t2, 'counter_threshold', 4)
# ctrl_bus_0.write(smA_t2, 'config_handler', 2)

# smA_t2.enable_timestamps('STAT_sma_2_', 2)
axis_net_in.write(smA_t2, strToInt("{AMHeader,0x10,0x01,0,0,1,0}"))
axis_net_in.write(smA_t2, strToInt("{AMToken,0x2}"), tlast=1)

axis_net_in.write(smA_t2, strToInt("{AMHeader,0x10,0x00,0,2,1,1}"))
axis_net_in.write(smA_t2, strToInt("{AMToken,0x3}"))
axis_net_in.write(smA_t2, 5, tlast=1)

# smA_t2.wait_level('interrupt_0 == $value', 1)
smA_t2.add_delay("980ns")
ctrl_bus_0.read(smA_t2, "counter", 5)
ctrl_bus_1.read(smA_t2, "counter", 6)
ctrl_bus_0.write(smA_t2, "arg", 0x5)
ctrl_bus_0.write(smA_t2, "config", 0x12)
ctrl_bus_0.write(smA_t2, "config", 0x02)
ctrl_bus_1.write(smA_t2, "arg", 0x6)
ctrl_bus_1.write(smA_t2, "config", 0x12)
ctrl_bus_1.write(smA_t2, "config", 0x02)
smA_t2.add_delay("100ns")
ctrl_bus_0.read(smA_t2, "counter", 0)
ctrl_bus_1.read(smA_t2, "counter", 0)
smA_t2.disable_timestamps()
# smA_t2.wait_level('interrupt_1 == $value', 1)

# ctrl_bus_1.write(smA_t2, 'config', 1)
# ctrl_bus_1.write(smA_t2, 'config', 2)
# ctrl_bus_1.write(smA_t2, 'config', 0)
# ctrl_bus_0.write(smA_t2, 'config', 1)
# ctrl_bus_0.write(smA_t2, 'config', 2)
# ctrl_bus_0.write(smA_t2, 'config', 0)

short_message_A.add_thread(smA_t2)

smA_t3 = Thread()
axis_net_out.read(smA_t3, strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}"))
axis_net_out.read(smA_t3, strToInt("{AMToken,0x0}"), tlast=1)
axis_net_out.read(smA_t3, strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}"))
axis_net_out.read(smA_t3, strToInt("{AMToken,0x1}"), tlast=1)
# smA_t3.enable_timestamps('STAT_sma_3_', 0)
axis_net_out.read(smA_t3, strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}"))
axis_net_out.read(smA_t3, strToInt("{AMToken,0x2}"), tlast=1)
axis_net_out.read(smA_t3, strToInt("{AMHeader,0x0,0x10,0,1,0x41,0}"))
axis_net_out.read(smA_t3, strToInt("{AMToken,0x3}"), tlast=1)
smA_t3.disable_timestamps()

# smA_t3.wait_level('interrupt_0 == $value', 0)
# smA_t3.wait_level('interrupt_1 == $value', 0)
smA_t3.end_vector()

short_message_A.add_thread(smA_t3)

#-------------------------------------------------------------------------------
# Short Message B
#
# Sends short messages from local kernels that call the adder handler function
# that raises the interrupt once the set threshold is exceeded. There are two
# kernels here. A short message calling no handler is also inserted which does
# nothing and is just absorbed by the GAScore. The last two transactions are
# routed to the network since the destination is > 0xF.
#-------------------------------------------------------------------------------

short_message_B = TestVector()
short_message_B.add_thread(initT)

smB_t2 = Thread()
smB_t2.add_delay('100ns')
# ctrl_bus_1.write(smB_t2, 'counter_threshold', 4),
# ctrl_bus_1.write(smB_t2, 'config_handler', 2),
axis_kernel_in.write(smB_t2, strToInt("{AMHeader,0x0,0x01,0,2,1,1}"))
# smB_t2.enable_timestamps('STAT_smb_2_', 0)
axis_kernel_in.write(smB_t2, strToInt("{AMToken,0x0}"))
axis_kernel_in.write(smB_t2, 4, tlast=1)
axis_kernel_in.write(smB_t2, strToInt("{AMHeader,0x0,0x01,0,2,1,1}"))
axis_kernel_in.write(smB_t2, strToInt("{AMToken,0x1}"))
axis_kernel_in.write(smB_t2, 1, tlast=1),
# ctrl_bus_0.write(smB_t2, 'counter_threshold', 4)
# ctrl_bus_0.write(smB_t2, 'config_handler', 2)
axis_kernel_in.write(smB_t2, strToInt("{AMHeader,0x0,0x01,0,0,1,0}"))
axis_kernel_in.write(smB_t2, strToInt("{AMToken,0x2}"), tlast=1)
axis_kernel_in.write(smB_t2, strToInt("{AMHeader,0x1,0x00,0,2,1,1}"))
axis_kernel_in.write(smB_t2, strToInt("{AMToken,0x3}"))
axis_kernel_in.write(smB_t2, 5, tlast=1)
axis_kernel_in.write(smB_t2, strToInt("{AMHeader,0x0,0x11,0,0,1,0}"))
axis_kernel_in.write(smB_t2, strToInt("{AMToken,0x4}"), tlast=1)
axis_kernel_in.write(smB_t2, strToInt("{AMHeader,0x1,0x10,0,0,1,0}"))
axis_kernel_in.write(smB_t2, strToInt("{AMToken,0x5}"), tlast=1)
smB_t2.wait_flag(0)
axis_net_in.write(smB_t2, strToInt("{AMHeader,0x11,0x0,0,0,0x41,0}"))
axis_net_in.write(smB_t2, strToInt("{AMToken,0x6}"), tlast=1)
smB_t2.disable_timestamps()
axis_net_in.write(smB_t2, strToInt("{AMHeader,0x10,0x1,0,0,0x41,0}"))
axis_net_in.write(smB_t2, strToInt("{AMToken,0x7}"), tlast=1)
ctrl_bus_1.read(smB_t2, "counter", 5)
ctrl_bus_0.read(smB_t2, "counter", 5)
ctrl_bus_0.write(smB_t2, "arg", 0x5)
ctrl_bus_0.write(smB_t2, "config", 0x12)
ctrl_bus_0.write(smB_t2, "config", 0x02)
ctrl_bus_1.write(smB_t2, "arg", 0x5)
ctrl_bus_1.write(smB_t2, "config", 0x12)
ctrl_bus_1.write(smB_t2, "config", 0x02)
smB_t2.set_flag(1)
short_message_B.add_thread(smB_t2)

# smB_t4 = Thread()
# smB_t4.wait_level('interrupt_1 == $value', 1)
# smB_t4.enable_timestamps('STAT_smb_4_', 0)
# smB_t4.wait_level('interrupt_0 == $value', 1)
# smB_t4.disable_timestamps()
# short_message_B.add_thread(smB_t4)

smB_t3 = Thread()
# smB_t3.enable_timestamps('STAT_smb_3_', 0)
# axis_kernel_out.read(smB_t3, strToInt("{KernelHeader,0x41,0,0,0}"), tdest=0, tlast=1)
# axis_kernel_out.read(smB_t3, strToInt("{KernelHeader,0x41,0,0,1}"), tdest=0, tlast=1)
# axis_kernel_out.read(smB_t3, strToInt("{KernelHeader,0x41,0,0,2}"), tdest=0, tlast=1)
# axis_kernel_out.read(smB_t3, strToInt("{KernelHeader,0x41,1,0,3}"), tdest=1, tlast=1)
axis_net_out.read(smB_t3, strToInt("{AMHeader,0x0,0x11,0,0,1,0}"))
axis_net_out.read(smB_t3, strToInt("{AMToken,0x4}"), tlast=1)
axis_net_out.read(smB_t3, strToInt("{AMHeader,0x1,0x10,0,0,1,0}"))
axis_net_out.read(smB_t3, strToInt("{AMToken,0x5}"), tlast=1)
smB_t3.set_flag(0)
# axis_kernel_out.read(smB_t3, strToInt("{KernelHeader,0x41,0x11,0,6}"), tdest=0, tlast=1)
# axis_kernel_out.read(smB_t3, strToInt("{KernelHeader,0x41,0x10,0,7}"), tdest=1, tlast=1)
smB_t3.wait_flag(1)
smB_t3.disable_timestamps()
smB_t3.end_vector()
short_message_B.add_thread(smB_t3)

#-------------------------------------------------------------------------------
# Medium Message A
#
# This sends a pair of medium messages: one for loopback, one externally. The
# loopback message is  medium FIFO message which passes the token + payload to
# the local kernel and a reply message is sent back. The remote message is
# forwarded in its entirety out (FIFO bit is changed in the type)
#-------------------------------------------------------------------------------

medium_message_A = TestVector()
medium_message_A.add_thread(initT)

mmA_t1 = Thread()
mmA_t1.add_delay('100ns')
# mmA_t1.enable_timestamps('STAT_mma_1_', 0)
axis_kernel_in.write(mmA_t1, strToInt("{AMHeader,0x0,0x01,16,0,0x12,0}")),
axis_kernel_in.write(mmA_t1, strToInt("{AMToken,0x0}")),
axis_kernel_in.write(mmA_t1, 5), # payload argument of 5
axis_kernel_in.write(mmA_t1, 4, tlast=1), # payload argument of 4
axis_kernel_in.writes(mmA_t1, [
    {'tdata': strToInt("{AMHeader,0x0,0x10,16,0,0x12,0}")},
    {'tdata': strToInt("{AMToken,0x0}")},
    {'tdata': 5},
    {'tdata': 4, "tlast": 1}
])
mmA_t1.disable_timestamps()
medium_message_A.add_thread(mmA_t1)

mmA_t2 = Thread()
# mmA_t2.enable_timestamps('STAT_mma_2_', 0)
# axis_kernel_out.read(mmA_t2, strToInt("{KernelHeader,0x12,0,16,0}"), tdest=1)
axis_kernel_out.read(mmA_t2, strToInt("{AMHeader,0x0,0x0,16,0,0x12,0}"), tdest=1),
axis_kernel_out.read(mmA_t2, 5, tdest=1), # payload argument of 5
axis_kernel_out.read(mmA_t2, 4, tdest=1, tlast=1), # payload argument of 4
# axis_kernel_out.read(mmA_t2, strToInt("{KernelHeader,0x41,0,0,0}"), tdest=0, tlast=1)
mmA_t2.disable_timestamps()
medium_message_A.add_thread(mmA_t2)

mmA_t3 = Thread()
# mmA_t3.enable_timestamps('STAT_mma_3_', 0)
axis_net_out.read(mmA_t3, strToInt("{AMHeader,0x0,0x10,16,0,0x02,0}"))
axis_net_out.read(mmA_t3, strToInt("{AMToken,0x0}"))
axis_net_out.read(mmA_t3, 5) # payload argument of 5
axis_net_out.read(mmA_t3, 4, tlast=1) # payload argument of 4
mmA_t3.disable_timestamps()
mmA_t3.end_vector()
medium_message_A.add_thread(mmA_t3)

#-------------------------------------------------------------------------------
# Medium Message B
#
# This sends a medium message from a local kernel externally. It reads 4 words
# from byte address 0x10 and streams it out
#-------------------------------------------------------------------------------

medium_message_B = TestVector()
medium_message_B.add_thread(initT)

mmB_t1 = Thread()
mmB_t1.add_delay('100ns')
# mmB_t1.enable_timestamps('STAT_mmb_1_', 0)
axis_kernel_in.write(mmB_t1, strToInt("{AMHeader,0x0,0x10,32,0,0x02,0}")),
axis_kernel_in.write(mmB_t1, strToInt("{AMToken,0x0}")),
axis_kernel_in.write(mmB_t1, 0x10, tlast=1)
mmB_t1.disable_timestamps()
medium_message_B.add_thread(mmB_t1)

mmB_t2 = Thread()
# mmB_t2.enable_timestamps('STAT_mmb_2_', 0)
axis_net_out.read(mmB_t2, strToInt("{AMHeader,0x0,0x10,32,0,0x02,0}")),
axis_net_out.read(mmB_t2, strToInt("{AMToken,0x0}")),
axis_net_out.read(mmB_t2, 2),
axis_net_out.read(mmB_t2, 3),
axis_net_out.read(mmB_t2, 4),
axis_net_out.read(mmB_t2, 5, tlast=1)
mmB_t2.disable_timestamps()
mmB_t2.print_elapsed_time('Medium_Message_B')
mmB_t2.end_vector()
medium_message_B.add_thread(mmB_t2)

#-------------------------------------------------------------------------------
# Medium Message C
#
# Same as Medium B but the data request comes from a remote device (i.e. a get).
# It sends an async message out back to the requesting kernel
#-------------------------------------------------------------------------------

medium_message_C = TestVector()
medium_message_C.add_thread(initT)

mmC_t1 = Thread()
mmC_t1.add_delay('100ns')
# mmC_t1.enable_timestamps('STAT_mmc_1_', 0)
axis_net_in.write(mmC_t1, strToInt("{AMHeader,0x10,0,32,0,0x42,0}")),
axis_net_in.write(mmC_t1, strToInt("{AMToken,0x0}")),
axis_net_in.write(mmC_t1, 0x10, tlast=1)
mmC_t1.disable_timestamps()
medium_message_C.add_thread(mmC_t1)

mmC_t2 = Thread()
# mmC_t2.enable_timestamps('STAT_mmc_2_', 0)
axis_net_out.read(mmC_t2, strToInt("{AMHeader,0x0,0x10,32,0,0x22,0}")),
axis_net_out.read(mmC_t2, strToInt("{AMToken,0x0}")),
axis_net_out.read(mmC_t2, 2),
axis_net_out.read(mmC_t2, 3),
axis_net_out.read(mmC_t2, 4),
axis_net_out.read(mmC_t2, 5, tlast=1)
mmC_t2.disable_timestamps()
mmC_t2.print_elapsed_time('Medium_Message_C')
mmC_t2.end_vector()
medium_message_C.add_thread(mmC_t2)

#-------------------------------------------------------------------------------
# Long Message A
#
# Sends a long message reading 4 words from byte address 0x10 to be written
# to byte address 0x20. Handler 1 is also called with argument 1
#-------------------------------------------------------------------------------

long_message_A = TestVector()
long_message_A.add_thread(initT)

lmA_t1 = Thread()
lmA_t1.add_delay('100ns')
axis_kernel_in.writes(lmA_t1, [
    {"tdata": strToInt("{AMHeader,0x0,0x10,32,2,0x04,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x10},
    {"tdata": 0x20},
    {"tdata": 0x1, "tlast": 1}
])
long_message_A.add_thread(lmA_t1)

lmA_t2 = Thread()
axis_net_out.reads(lmA_t2, [
    {"tdata": strToInt("{AMHeader,0x0,0x10,32,2,0x04,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x20},
    {"tdata": 1},
    {"tdata": 2},
    {"tdata": 3},
    {"tdata": 4},
    {"tdata": 5, "tlast": 1}
])
lmA_t2.print_elapsed_time('Long_Message_A')
lmA_t2.end_vector()
long_message_A.add_thread(lmA_t2)

#-------------------------------------------------------------------------------
# Long Message B
#
# Sends a long strided message reading 4 words from address 0 with an
# offset of 0x100 (32 words) and sent to address 0x20 remotely.
#-------------------------------------------------------------------------------

long_message_B = TestVector()
long_message_B.add_thread(initT)

lmB_t1 = Thread()
lmB_t1.add_delay('100ns')
axis_kernel_in.writes(lmB_t1, [
    {"tdata": strToInt("{AMHeader,0x0,0x10,32,0,0x05,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4}")},
    {"tdata": 0},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20, "tlast": 1}
])
long_message_B.add_thread(lmB_t1)

lmB_t2 = Thread()
axis_net_out.reads(lmB_t2, [
    {"tdata": strToInt("{AMHeader,0x0,0x10,32,0,0x05,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20},
    {"tdata": 0},
    {"tdata": 0x20},
    {"tdata": 0x40},
    {"tdata": 0x60, "tlast": 1}
])
lmB_t2.print_elapsed_time('long_message_B')
lmB_t2.end_vector()
long_message_B.add_thread(lmB_t2)

#-------------------------------------------------------------------------------
# Long Message C
#
# Sends a Long Vectored message with two vectors from Kernel 0 to 0x10 (remote).
#-------------------------------------------------------------------------------

long_message_C = TestVector()
long_message_C.add_thread(initT)

lmC_t1 = Thread()
lmC_t1.add_delay('100ns')
axis_kernel_in.writes(lmC_t1, [
    {"tdata": strToInt("{AMHeader,0x0,0x10,80,0,0x06,0}")},
    {"tdata": strToInt("{AMLongVector,2,2,32,32,0}")},
    {"tdata": 0},
    {"tdata": 0x10},
    {"tdata": 0x30},
    {"tdata": 0x100},
    {"tdata": 0x30},
    {"tdata": 0x80, "tlast": 1}
])
long_message_C.add_thread(lmC_t1)

lmC_t2 = Thread()
axis_net_out.reads(lmC_t2, [
    {"tdata": strToInt("{AMHeader,0x0,0x10,80,0,0x06,0}")},
    {"tdata": strToInt("{AMLongVector,2,32,0}")},
    {"tdata": 0x10},
    {"tdata": 48},
    {"tdata": 0x80},
    {"tdata": 0},
    {"tdata": 1},
    {"tdata": 2},
    {"tdata": 3},
    {"tdata": 0x20},
    {"tdata": 0x21},
    {"tdata": 0x22},
    {"tdata": 0x23},
    {"tdata": 0x24},
    {"tdata": 0x25, "tlast": 1}
])
lmC_t2.print_elapsed_time('long_message_C')
lmC_t2.end_vector()
long_message_C.add_thread(lmC_t2)

#-------------------------------------------------------------------------------
# Receive
#
# From the above messages, take the data on net_out and send it back to net_in
# (changing src/dst addresses as needed) to make sure the core can handle
# receiving these messages.
#-------------------------------------------------------------------------------

message_recv = TestVector()
message_recv.add_thread(initT)
message_t1 = message_recv.add_thread()
message_t1.add_delay('100ns')
axis_net_in.writes(message_t1, [
    # sends an empty short message doing nothing
    {"tdata": strToInt("{AMHeader,0x11,0x1,0,0,1,0}")},
    {"tdata": strToInt("{AMToken,0x0}"), "tlast": 1},

    # sends a reply that should just be absorbed
    {"tdata": strToInt("{AMHeader,0x11,0x1,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0x0}"), "tlast": 1},

    # sends a medium message with two word payload to kernel 1
    {"tdata": strToInt("{AMHeader,0x10,0x1,16,0,0x02,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 5}, # payload argument of 5
    {"tdata": 4, "tlast": 1}, # payload argument of 4

    # response to a remote get - async medium message
    {"tdata": strToInt("{AMHeader,0x10,0x0,32,0,0x22,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 2},
    {"tdata": 3},
    {"tdata": 4},
    {"tdata": 5, "tlast":1},

    # long message to store 4 words to address 0x20
    {"tdata": strToInt("{AMHeader,0x10,0x1,32,2,0x04,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x20},
    {"tdata": 4}, # handler arg
    {"tdata": 4},
    {"tdata": 5},
    {"tdata": 6},
    {"tdata": 7, "tlast": 1},

    # long strided message
    {"tdata": strToInt("{AMHeader,0x10,0x1,32,0,0x05,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20}, # destination address
    {"tdata": 0x4}, # payload 0
    {"tdata": 0x24},
    {"tdata": 0x44},
    {"tdata": 0x64, "tlast": 1}, # payload 3

    # long vectored message (2 vectors (of size 4 and 6))
    {"tdata": strToInt("{AMHeader,0x10,0x1,80,0,0x06,0}")},
    {"tdata": strToInt("{AMLongVector,2,32,0xE}")},
    {"tdata": 0x10}, # dst addr for Vector 0
    {"tdata": 48}, # size of vector 1
    {"tdata": 0x80}, # dst addr of vector 1
    {"tdata": 2},
    {"tdata": 3},
    {"tdata": 4},
    {"tdata": 5},
    {"tdata": 0x10},
    {"tdata": 0x11},
    {"tdata": 0x12},
    {"tdata": 0x13},
    {"tdata": 0x14},
    {"tdata": 0x15, "tlast": 1}
])


message_t2 = Thread()
axis_net_out.reads(message_t2, [
    # reply to short message
    {"tdata": strToInt("{AMHeader,0x1,0x11,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},

    # reply to medium message
    {"tdata": strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},

    # reply to long message
    {"tdata": strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},

    # reply to long strided message
    {"tdata": strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0xF}")},

    # reply to long vectored message
    {"tdata": strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0xE}")}
])
message_t2.print_elapsed_time('message_recv')
message_t2.end_vector()
message_recv.add_thread(message_t2)

#-------------------------------------------------------------------------------
# Get
#
# Tests receiving get message requests (identified by non-short REPLY messages).
# The requests are handled by passing them off as local-kernel generated SEND
# requests but enforce the ASYNC flag high. This prevents a reply back to the
# local kernel.
#-------------------------------------------------------------------------------

message_get = TestVector()
message_get.add_thread(initT)

message_get_t1 = message_get.add_thread()
message_get_t1.add_delay('100ns')
axis_net_in.writes(message_get_t1, [
    # GET two words from address 0x10 as a medium message
    {"tdata": strToInt("{AMHeader,0x10,0x1,16,0,0x42,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x10, "tlast": 1}, # address

    # GET 4 words from remote and save them locally
    {"tdata": strToInt("{AMHeader,0x10,0x1,64,2,0x44,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x0}, # src address
    {"tdata": 0x20}, # dst address
    {"tdata": 1, "tlast": 1}, # handler arg

    # GET strided data from remote and save locally
    {"tdata": strToInt("{AMHeader,0x10,0x1,32,0,0x45,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4}")},
    {"tdata": 0},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20, "tlast": 1},

    # GET vectored data from remote and save locally
    {"tdata": strToInt("{AMHeader,0x10,0x1,80,0,0x46,0}")},
    {"tdata": strToInt("{AMLongVector,2,2,32,32,0}")},
    {"tdata": 0}, # src addr 0
    {"tdata": 0x10}, # dst addr 0
    {"tdata": 0x30}, # src size 1
    {"tdata": 0x100}, # src addr 1
    {"tdata": 0x30}, # dst size 1
    {"tdata": 0x80, "tlast": 1} # dst addr 1
])

message_get_t2 = message_get.add_thread()
# message_get_t2.add_delay('100ns')
axis_net_out.reads(message_get_t2, [
    # reply to medium message
    {"tdata": strToInt("{AMHeader,0x1,0x10,16,0,0x22,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 2},
    {"tdata": 3},

    # reply to long message
    {"tdata": strToInt("{AMHeader,0x1,0x10,64,2,0x24,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x20},
    {"tdata": 1}, # handler arg
    {"tdata": 0}, 
    {"tdata": 1},
    {"tdata": 2},
    {"tdata": 3},
    {"tdata": 4},
    {"tdata": 5},
    {"tdata": 6},
    {"tdata": 7, "tlast": 1},

    # reply to long strided message
    {"tdata": strToInt("{AMHeader,0x1,0x10,32,0,0x25,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20},
    {"tdata": 0},
    {"tdata": 0x20},
    {"tdata": 0x40},
    {"tdata": 0x60, "tlast": 1},

    # reply to long vectored message
    {"tdata": strToInt("{AMHeader,0x1,0x10,80,0,0x26,0}")},
    {"tdata": strToInt("{AMLongVector,2,32,0}")},
    {"tdata": 0x10},
    {"tdata": 48},
    {"tdata": 0x80},
    {"tdata": 0},
    {"tdata": 1},
    {"tdata": 2},
    {"tdata": 3},
    {"tdata": 0x20},
    {"tdata": 0x21},
    {"tdata": 0x22},
    {"tdata": 0x23},
    {"tdata": 0x24},
    {"tdata": 0x25, "tlast": 1}
])
message_get_t2.print_elapsed_time('message_get')
message_get_t2.end_vector()

#-------------------------------------------------------------------------------
# Kernel Get
#
# Tests sending GET requests from a local kernel. These are non-short REPLY
# messages which are passed through verbatim.
#-------------------------------------------------------------------------------

message_kernel_get = TestVector()
message_kernel_get.add_thread(initT)

message_get_kernel_t1 = message_kernel_get.add_thread()
message_get_kernel_t1.add_delay('100ns')
axis_kernel_in.writes(message_get_kernel_t1, [
    # GET two words from address 0x10 as a medium message
    {"tdata": strToInt("{AMHeader,0x1,0x10,16,0,0x42,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x10, "tlast": 1}, # address

    # GET 4 words from remote and save them locally
    {"tdata": strToInt("{AMHeader,0x1,0x10,32,2,0x44,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x10}, # src address
    {"tdata": 0x20}, # dst address
    {"tdata": 1, "tlast": 1}, # handler arg

    # GET strided data from remote and save locally
    {"tdata": strToInt("{AMHeader,0x1,0x10,32,0,0x45,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4}")},
    {"tdata": 0},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20, "tlast": 1},

    # GET vectored data from remote and save locally
    {"tdata": strToInt("{AMHeader,0x1,0x10,80,0,0x46,0}")},
    {"tdata": strToInt("{AMLongVector,2,2,32,32,0}")},
    {"tdata": 0},
    {"tdata": 0x10},
    {"tdata": 0x30},
    {"tdata": 0x100},
    {"tdata": 0x30},
    {"tdata": 0x80, "tlast": 1}
])

message_get_kernel_t2 = message_kernel_get.add_thread()
axis_net_out.reads(message_get_kernel_t2, [
    # the message is passed through verbatim because it's a reply
    # GET two words from address 0x10 as a medium message
    {"tdata": strToInt("{AMHeader,0x1,0x10,16,0,0x42,0}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x10, "tlast": 1}, # address

    # GET 4 words from remote and save them locally
    {"tdata": strToInt("{AMHeader,0x1,0x10,32,2,0x44,1}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0x10}, # src address
    {"tdata": 0x20}, # dst address
    {"tdata": 1, "tlast": 1}, # handler arg

    # GET strided data from remote and save locally
    {"tdata": strToInt("{AMHeader,0x1,0x10,32,0,0x45,0}")},
    {"tdata": strToInt("{AMLongStride,0x100,8,4}")},
    {"tdata": 0},
    {"tdata": strToInt("{AMLongStride,0x100,8,4,0xF}")},
    {"tdata": 0x20, "tlast": 1},

    # GET vectored data from remote and save locally
    {"tdata": strToInt("{AMHeader,0x1,0x10,80,0,0x46,0}")},
    {"tdata": strToInt("{AMLongVector,2,2,32,32,0}")},
    {"tdata": 0},
    {"tdata": 0x10},
    {"tdata": 0x30},
    {"tdata": 0x100},
    {"tdata": 0x30},
    {"tdata": 0x80, "tlast": 1}
])
message_get_kernel_t2.print_elapsed_time('message_get_kernel')
message_get_kernel_t2.end_vector()

GAScore.add_test_vector(short_message_A)
GAScore.add_test_vector(short_message_B)
GAScore.add_test_vector(medium_message_A)
GAScore.add_test_vector(medium_message_B)
GAScore.add_test_vector(medium_message_C)
GAScore.add_test_vector(long_message_A)
GAScore.add_test_vector(long_message_B)
GAScore.add_test_vector(long_message_C)
GAScore.add_test_vector(message_recv)
GAScore.add_test_vector(message_get)
GAScore.add_test_vector(message_kernel_get)

GAScore.generateTB(filepath, 'sv')
