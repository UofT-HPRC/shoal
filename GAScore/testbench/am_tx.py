import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

am_tx = Testbench.default('am_tx')
filepath = os.path.join(os.path.dirname(__file__), 'build/am_tx/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('release_V', 'output')

axis_kernel = AXIS('axis_kernel', 'slave', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_kernel.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_kernel)

axis_net = AXIS('axis_net', 'master', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_net.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_net)

axis_mm2sCommand = AXIS('axis_mm2sCommand_V_data_V', 'master', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_o')
axis_mm2sCommand.port.init_channels('min', 64, True)
axis_mm2sCommand.port.add_channel('TREADY', 'tready')
dut.add_interface(axis_mm2sCommand)

axis_mm2s = AXIS('axis_mm2s', 'slave', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_mm2s.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_mm2s)

axis_mm2sStatus = AXIS('axis_mm2sStatus', 'slave', 'ap_clk', 
    c_struct='axis_word_mm2sStatus', c_stream='uaxis_l')
axis_mm2sStatus.port.init_channels('default', 64, True)
dut.add_interface(axis_mm2sStatus)
am_tx.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('axis_net_tready', 1)
initT.set_signal('axis_mm2sCommand_V_data_V_tready', 1)

#-------------------------------------------------------------------------------
# Short Message A
#
# Short message from 0xAA to 0xCC calling handler 0xE with two handler args
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t1 = short_message_A.add_thread()
smA_t1.add_delay('100ns')
smA_t1.init_timer()
axis_kernel.writes(smA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,32,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x0}"), "callTB": 1},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1, "callTB": 2}
])

smA_t2 = short_message_A.add_thread()
axis_net.reads(smA_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,32,0xE,1,2}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0,0xE,1,2}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1}
])
smA_t2.print_elapsed_time("Short_Message_A")
smA_t2.end_vector()

#-------------------------------------------------------------------------------
# Short Message B
#
# Short message from 0xAA to 0xCC calling handler 0xE with no handler args
#-------------------------------------------------------------------------------

short_message_B = TestVector()
short_message_B.add_thread(initT)

smB_t1 = short_message_B.add_thread()
smB_t1.add_delay('100ns')
smB_t1.init_timer()
axis_kernel.writes(smB_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,16,0xE,1,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0,0xE,1,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x1}"), "callTB": 2}
])

smB_t2 = short_message_B.add_thread()
axis_net.reads(smB_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,16,0xE,1,0}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0,0xE,1,0}")},
    {"tdata": strToInt("{AMToken,0x1}")}
])
smB_t2.print_elapsed_time("short_message_B")
smB_t2.end_vector()

#-------------------------------------------------------------------------------
# Medium Message A
#
# Medium message from 0xAA to 0xCC with 0x1 word of payload from FIFO. It 
# calls handler 0xE with zero handler args.
#-------------------------------------------------------------------------------

medium_message_A = TestVector()
medium_message_A.add_thread(initT)

mmA_t1 = medium_message_A.add_thread()
mmA_t1.add_delay('100ns')
mmA_t1.init_timer()
axis_kernel.writes(mmA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,24,0xE,0x12,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,8,0xE,0x12,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x2}"), "callTB": 1},
    {"tdata": 0xDEADBEEF, "tlast": 1, "callTB": 2}
])
medium_message_A.add_thread(mmA_t1)

mmA_t2 = medium_message_A.add_thread()
axis_net.reads(mmA_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,24,0xE,0x2,0}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,8,0xE,0x2,0}")},
    {"tdata": strToInt("{AMToken,0x2}")},
    {"tdata": 0xDEADBEEF, "tlast": 1}
])
mmA_t2.print_elapsed_time("Medium_Message_A")
mmA_t2.end_vector()
medium_message_A.add_thread(mmA_t2)

#-------------------------------------------------------------------------------
# Medium Message B
#
# Medium message from 0xAA to 0xCC with 0x100 words of payload from FIFO. It 
# calls handler 0xE with six handler args.
#-------------------------------------------------------------------------------

medium_message_B = TestVector()
medium_message_B.add_thread(initT)

mmB_t1 = medium_message_B.add_thread()
mmB_t1.add_delay('100ns')
mmB_t1.init_timer()
axis_kernel.writes(mmB_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x810,0xE,0x12,6}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x800,0xE,0x12,6}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x3}"), "callTB": 1}
])
for i in range(5):
    axis_kernel.write(mmB_t1, 0x98765432)
axis_kernel.write(mmB_t1, 0x98765432, callTB=1)
for i in range(255):
    axis_kernel.write(mmB_t1, 0xDEADBEEF)
axis_kernel.write(mmB_t1, 0xFEEDDEED, tlast=1, callTB=2)

mmB_t2 = medium_message_B.add_thread()
axis_net.reads(mmB_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x810,0xE,0x2,6}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x800,0xE,0x2,6}")},
    {"tdata": strToInt("{AMToken,0x3}")}
])
for i in range(6):
    axis_net.read(mmB_t2, 0x98765432)
for i in range(255):
    axis_net.read(mmB_t2, 0xDEADBEEF)
axis_net.read(mmB_t2, 0xFEEDDEED, tlast=1)
mmB_t2.print_elapsed_time("medium_message_B")
mmB_t2.end_vector()

#-------------------------------------------------------------------------------
# Medium Message C
#
# Medium message from 0xAA to 0xCC with 0x1 word of payload from mem. 
# It calls handler 0xE with zero handler args.
#-------------------------------------------------------------------------------

medium_message_C = TestVector()
medium_message_C.add_thread(initT)

mmC_t1 = medium_message_C.add_thread()
mmC_t1.add_delay('100ns')
mmC_t1.init_timer()
axis_kernel.writes(mmC_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,24,0xE,0x2,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,8,0xE,0x2,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x4}"), "callTB": 1},
    {"tdata": 0x1000, "tlast": 1, "callTB": 1} # address to read
])

mmC_t2 = medium_message_C.add_thread()
axis_mm2sCommand.read(mmC_t2, strToInt("{dataMoverCommand,8,1,0,1,0,0x1000,0}"))
axis_mm2s.write(mmC_t2, 0xFEEDDEED, tlast=1, callTB=2)
#? should mm2S status be used here
axis_mm2sStatus.write(mmC_t2, 0x80, callTB=1)

mmC_t3 = medium_message_C.add_thread()
axis_net.reads(mmC_t3, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,24,0xE,2,0}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,8,0xE,2,0}")},
    {"tdata": strToInt("{AMToken,0x4}")},
    {"tdata": 0xFEEDDEED, "tlast": 1} # address to read
])
mmC_t3.print_elapsed_time("medium_message_C")
mmC_t3.end_vector()

#-------------------------------------------------------------------------------
# Long Message A
#
# Long message from 0xAA to 0xCC with 0x100 words of payload written to 
# dst address 0xAABBCC08. It calls handler 0xE with zero handler args.
#-------------------------------------------------------------------------------

long_message_A = TestVector()
long_message_A.add_thread(initT)

lmA_t1 = long_message_A.add_thread()
lmA_t1.add_delay('100ns')
lmA_t1.init_timer()
axis_kernel.writes(lmA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x818,0xE,0x14,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x800,0xE,0x14,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x5}"), "callTB": 1},
    {"tdata": "0xAABBCCD8", "callTB": 1} # address
])
for i in range(255):
    axis_kernel.write(lmA_t1, i)
axis_kernel.write(lmA_t1, 0xDEADBEEF, tlast=1, callTB=2)

lmA_t2 = long_message_A.add_thread()
# USE_ABS_PAYLOAD axis_net.read(lmA_t2, strToInt("{AMHeader,0xAA,0xCC,0x818,0xE,4,0}"))
axis_net.read(lmA_t2, strToInt("{AMHeader,0xAA,0xCC,0x800,0xE,4,0}"))
axis_net.read(lmA_t2, strToInt("{AMToken,0x5}"))
axis_net.read(lmA_t2, 0xAABBCCD8)
for i in range(255):
    axis_net.read(lmA_t2, i)
axis_net.read(lmA_t2, 0xDEADBEEF, tlast=1)
lmA_t2.print_elapsed_time("Long_Message_A")
lmA_t2.end_vector()

#-------------------------------------------------------------------------------
# Long Message B
#
# Long message A but from memory instead of FIFO
#-------------------------------------------------------------------------------

long_message_B = TestVector()
long_message_B.add_thread(initT)

lmB_t1 = long_message_B.add_thread()
lmB_t1.add_delay('100ns')
lmB_t1.init_timer()
axis_kernel.writes(lmB_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x818,0xE,0x4,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x800,0xE,0x4,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x5}"), "callTB": 1},
    {"tdata": 0xAABBCCD8, "callTB": 1}, # source address
    {"tdata": 8, "callTB": 1} # dst address
])

lmB_t2 = long_message_B.add_thread()
axis_mm2sCommand.read(lmB_t2, strToInt("{dataMoverCommand,0x800,1,0,1,0,0xAABBCCD8,0}"))
for i in range(255):
    axis_mm2s.write(lmB_t2, 0xDEADBEEF)
axis_mm2s.write(lmB_t2, 0xDEADBEEF, tlast=1, callTB=1)
axis_mm2sStatus.write(lmB_t2, 0x80, callTB=2)

lmB_t3 = long_message_B.add_thread()
# USE_ABS_PAYLOAD axis_net.read(lmB_t3, strToInt("{AMHeader,0xAA,0xCC,0x818,0xE,4,0}"))
axis_net.read(lmB_t3, strToInt("{AMHeader,0xAA,0xCC,0x800,0xE,4,0}"))
axis_net.read(lmB_t3, strToInt("{AMToken,0x5}"))
axis_net.read(lmB_t3, 8)
for i in range(255):
    axis_net.read(lmB_t3, 0xDEADBEEF)
axis_net.read(lmB_t3, 0xDEADBEEF, tlast=1)
lmB_t3.print_elapsed_time("long_message_B")
lmB_t3.end_vector()

#-------------------------------------------------------------------------------
# Long Message C
#
# Long strided message from 0xAA to 0xCC with 0xC words of payload
# across two vectors. It calls handler 0xD with two handler args. Each 
# stride is 0x100 words, and it write 4 words each time
#-------------------------------------------------------------------------------

long_message_C = TestVector()
long_message_C.add_thread(initT)

lmC_t1 = long_message_C.add_thread()
lmC_t1.add_delay('100ns')
lmC_t1.init_timer()
axis_kernel.writes(lmC_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x88,0xD,0x5,2}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x60,0xD,0x5,2}"), "callTB": 1},
    {"tdata": strToInt("{AMLongStride,0,0x60,1}")}, # src stride info
    {"tdata": 0x200}, # src address
    {"tdata": strToInt("{AMLongStride,0x100,32,3,0x5}")}, # dst stride info
    {"tdata": 0, "callTB": 1}, # dst address
    {"tdata": 0xAABBCCDD, "callTB": 0}, # handler arg 0
    {"tdata": 0xDDCCBBAA, "callTB": 1}, # handler arg 1
])

lmC_t2 = long_message_C.add_thread()
axis_mm2sCommand.read(lmC_t2, strToInt("{dataMoverCommand,0x60,1,0,1,0,0x200,0}"))
for i in range(2):
    for j in range(3):
        axis_mm2s.write(lmC_t2, 0xBEEFDEAD)
    axis_mm2s.write(lmC_t2, 0xDEADBEEF)
for j in range(3):
    axis_mm2s.write(lmC_t2, 0xBEEFDEAD)
axis_mm2s.write(lmC_t2, 0xDEADBEEF, tlast=1, callTB=1)
axis_mm2sStatus.write(lmC_t2, 0x80, callTB=2)

lmC_t3 = long_message_C.add_thread()
axis_net.reads(lmC_t3, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x88,0xD,0x5,2}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x60,0xD,0x5,2}")},
    {"tdata": strToInt("{AMLongStride,0x100,0x20,3,0x5}")},
    {"tdata": 0}, # initial address
    {"tdata": 0xAABBCCDD}, # handler arg 0
    {"tdata": 0xDDCCBBAA} # handler arg 1
])
for i in range(2):
    for j in range(3):
        axis_net.read(lmC_t3, 0xBEEFDEAD)
    axis_net.read(lmC_t3, 0xDEADBEEF)
for j in range(3):
    axis_net.read(lmC_t3, 0xBEEFDEAD)
axis_net.read(lmC_t3, 0xDEADBEEF, tlast=1)
lmC_t3.print_elapsed_time("long_message_C")
lmC_t3.end_vector()

#-------------------------------------------------------------------------------
# Long Message D
#
# Long vector message from 0xAA to 0xCC with 0x10C words of payload
# across two vectors. It calls handler 0xF with zero handler args. First 
# vector has 0xC words to address 0 and second has has 0x100 words to 
# address 0xFF00
#-------------------------------------------------------------------------------

long_message_D = TestVector()
long_message_D.add_thread(initT)

lmD_t1 = long_message_D.add_thread()
lmD_t1.add_delay('100ns')
lmD_t1.init_timer()
axis_kernel.writes(lmD_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,type:0x6,src:0xAA,dst:0xCC,payload:0x888,handler:0xF,args:0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,type:0x6,src:0xAA,dst:0xCC,payload:0x860,handler:0xF,args:0}"), "callTB": 1},
    {"tdata": strToInt("{AMLongVector,2,2,0x60,0x60,0x8}")},
    {"tdata": 0x1000}, # src address 1
    {"tdata": 0}, # destination address 1
    {"tdata": 0x800}, # src size 1 
    {"tdata": 0xFF00}, # src address 1
    {"tdata": 0x800}, # dst size 1 
    {"tdata": 0xDD00, "callTB": 1} # dst address 1
])

lmD_t2 = long_message_D.add_thread()
axis_mm2sCommand.read(lmD_t2, strToInt("{dataMoverCommand,0x60,1,0,0,0,0x1000,0}"))
axis_mm2sCommand.read(lmD_t2, strToInt("{dataMoverCommand,0x800,1,0,1,0,0xFF00,0}"))
for i in range(2):
    for j in range(3):
        axis_mm2s.write(lmD_t2, 0xBEEFDEAD)
    axis_mm2s.write(lmD_t2, 0xDEADBEEF)
for j in range(3):
    axis_mm2s.write(lmD_t2, 0xBEEFDEAD)
axis_mm2s.write(lmD_t2, 0xDEADBEEF, tlast=1)
for i in range(255):
    axis_mm2s.write(lmD_t2, 0xFFAAFFAA)
axis_mm2s.write(lmD_t2, 0xDDDDDDDD, tlast=1)
axis_mm2sStatus.write(lmD_t2, 0x80)
axis_mm2sStatus.write(lmD_t2, 0x80, callTB=1)

lmD_t3 = long_message_D.add_thread()
axis_net.reads(lmD_t3, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x888,0xF,0x6,0}")},
    {"tdata": strToInt("{AMHeader,0xAA,0xCC,0x860,0xF,0x6,0}")},
    {"tdata": strToInt("{AMLongVector,2,0x60,0x8}")},
    {"tdata": 0}, # dst address 1
    {"tdata": 0x800}, # dst size 2
    {"tdata": 0xDD00}, # dst addr 2
])
for i in range(2):
    for j in range(3):
        axis_net.read(lmD_t3, 0xBEEFDEAD)
    axis_net.read(lmD_t3, 0xDEADBEEF)
for j in range(3):
    axis_net.read(lmD_t3, 0xBEEFDEAD)
axis_net.read(lmD_t3, 0xDEADBEEF, tlast=1)
for i in range(255):
    axis_net.read(lmD_t3, 0xFFAAFFAA)
axis_net.read(lmD_t3, 0xDDDDDDDD, tlast=1)
lmD_t3.print_elapsed_time("long_message_D")
lmD_t3.end_vector()

am_tx.add_test_vector(short_message_A)
am_tx.add_test_vector(short_message_B)
am_tx.add_test_vector(medium_message_A)
am_tx.add_test_vector(medium_message_B)
am_tx.add_test_vector(medium_message_C)
am_tx.add_test_vector(long_message_A)
am_tx.add_test_vector(long_message_B)
am_tx.add_test_vector(long_message_C)
am_tx.add_test_vector(long_message_D)

am_tx.generateTB(filepath, 'all')
