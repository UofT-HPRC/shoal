import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

am_rx = Testbench.default('am_rx')
filepath = os.path.join(os.path.dirname(__file__), 'build/am_rx/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('release_V', 'output')

axis_net = AXIS('axis_net', 'slave', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_net.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_net)

axis_xpams_rx = AXIS('axis_xpams_rx', 'master', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_xpams_rx.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_xpams_rx)

axis_s2mmCommand = AXIS('axis_s2mmCommand_V_data_V', 'master', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_o')
axis_s2mmCommand.port.init_channels('min', 64, True)
axis_s2mmCommand.port.add_channel('TREADY', 'tready')
dut.add_interface(axis_s2mmCommand)

axis_s2mm = AXIS('axis_s2mm', 'master', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_s2mm.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_s2mm)

axis_s2mmStatus = AXIS('axis_s2mmStatus', 'slave', 'ap_clk', 
    c_struct='axis_word_mm2sStatus', c_stream='uaxis_l')
axis_s2mmStatus.port.init_channels('default', 64, True)
dut.add_interface(axis_s2mmStatus)
am_rx.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('axis_xpams_rx_tready', 1)
initT.set_signal('axis_s2mmCommand_V_data_V_tready', 1)
initT.set_signal('axis_s2mm_tready', 1)

#-------------------------------------------------------------------------------
# Short Message A
#
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t1 = short_message_A.add_thread()
smA_t1.add_delay('100ns')
smA_t1.init_timer()
axis_net.writes(smA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,48,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x0}"), "callTB": 1},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1, "callTB": 2}
])

smA_t2 = short_message_A.add_thread()
axis_xpams_rx.reads(smA_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,48,0xE,1,2}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,2}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1}
])
smA_t2.print_elapsed_time("Short_Message_A")
smA_t2.end_vector()

#-------------------------------------------------------------------------------
# Short Message B
#
#-------------------------------------------------------------------------------

short_message_B = TestVector()
short_message_B.add_thread(initT)

smB_t1 = short_message_B.add_thread()
smB_t1.add_delay('100ns')
smB_t1.init_timer()
axis_net.writes(smB_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,24,0xE,1,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x1}"), "callTB": 2}
])

smB_t2 = short_message_B.add_thread()
axis_xpams_rx.reads(smB_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,24,0xE,1,0}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,0}")},
    {"tdata": strToInt("{AMToken,0x1}")}
])
smB_t2.print_elapsed_time("short_message_B")
smB_t2.end_vector()

#-------------------------------------------------------------------------------
# Medium Message A
#
#-------------------------------------------------------------------------------

medium_message_A = TestVector()
medium_message_A.add_thread(initT)

mmA_t1 = medium_message_A.add_thread()
mmA_t1.add_delay('100ns')
mmA_t1.init_timer()
axis_net.writes(mmA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,36,0xE,2,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,8,0xE,2,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x2}"), "callTB": 1},
    {"tdata": 0xDEADBEEF, "tlast": 1, "callTB": 2}
])
medium_message_A.add_thread(mmA_t1)

mmA_t2 = medium_message_A.add_thread()
axis_xpams_rx.reads(mmA_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,36,0xE,2,0}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,8,0xE,2,0}")},
    {"tdata": strToInt("{AMToken,0x2}")},
    {"tdata": 0xDEADBEEF, "tlast": 1}
])
mmA_t2.print_elapsed_time("Medium_Message_A")
mmA_t2.end_vector()
medium_message_A.add_thread(mmA_t2)

#-------------------------------------------------------------------------------
# Medium Message B
#
#-------------------------------------------------------------------------------

medium_message_B = TestVector()
medium_message_B.add_thread(initT)

mmB_t1 = medium_message_B.add_thread()
mmB_t1.add_delay('100ns')
mmB_t1.init_timer()
axis_net.writes(mmB_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,0x818,0xE,2,6}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0x800,0xE,2,6}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x3}"), "callTB": 1}
])
for i in range(5):
    axis_net.write(mmB_t1, 0x98765432)
axis_net.write(mmB_t1, 0x98765432, callTB=1)
for i in range(255):
    axis_net.write(mmB_t1, 0xDEADBEEF)
axis_net.write(mmB_t1, 0xFEEDDEED, tlast=1, callTB=2)

mmB_t2 = medium_message_B.add_thread()
axis_xpams_rx.reads(mmB_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,0x818,0xE,2,6}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0x800,0xE,2,6}")},
    {"tdata": strToInt("{AMToken,0x3}")}
])
for i in range(6):
    axis_xpams_rx.read(mmB_t2, 0x98765432)
for i in range(255):
    axis_xpams_rx.read(mmB_t2, 0xDEADBEEF)
axis_xpams_rx.read(mmB_t2, 0xFEEDDEED, tlast=1)
mmB_t2.print_elapsed_time("medium_message_B")
mmB_t2.end_vector()

#-------------------------------------------------------------------------------
# Long Message A
#
#-------------------------------------------------------------------------------

long_message_A = TestVector()
long_message_A.add_thread(initT)

lmA_t1 = long_message_A.add_thread()
lmA_t1.add_delay('100ns')
lmA_t1.init_timer()
axis_net.writes(lmA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,0x818,0xE,4,0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0x800,0xE,4,0}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x4}"), "callTB": 1},
    {"tdata": "0xAABBCCD8", "callTB": 1} # address
])
for i in range(255):
    axis_net.write(lmA_t1, 0xDEADBEEF)
axis_net.write(lmA_t1, 0xDEADBEEF, tlast=1, callTB=2)

lmA_t2 = long_message_A.add_thread()
# USE_ABS_PAYLOAD axis_xpams_rx.read(lmA_t2, strToInt("{AMHeader,0x01,0x02,0x818,0xE,4,0}"))
axis_xpams_rx.read(lmA_t2, strToInt("{AMHeader,0x01,0x02,0x800,0xE,4,0}"))
axis_xpams_rx.read(lmA_t2, strToInt("{AMToken,0x4}"))
for i in range(255):
    axis_s2mm.read(lmA_t2, 0xDEADBEEF)
axis_s2mm.read(lmA_t2, 0xDEADBEEF, tlast=1)

lmA_t3 = long_message_A.add_thread()
axis_s2mmCommand.read(lmA_t3, strToInt("{dataMoverCommand,0x800,1,0,1,0,0xAABBCCD8,0}"))

lmA_t4 = long_message_A.add_thread()
lmA_t4.wait_level('axis_s2mm_tlast == $value', value=1)
lmA_t4.add_delay('50ns')
axis_s2mmStatus.write(lmA_t4, 0x80, callTB=1)
lmA_t4.print_elapsed_time("Long_Message_A")
lmA_t4.end_vector()

#-------------------------------------------------------------------------------
# Long Message B
#
# Long strided message from 0xAA to 0xCC with 0xC words of payload
# across two vectors. It calls handler 0xD with two handler args. Each 
# stride is 0x100 words, and it write 4 words each time
#-------------------------------------------------------------------------------

long_message_B = TestVector()
long_message_B.add_thread(initT)

lmB_t1 = long_message_B.add_thread()
lmB_t1.add_delay('100ns')
lmB_t1.init_timer()
axis_net.writes(lmB_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,0x88,0xD,0x5,2}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0x60,0xD,0x5,2}"), "callTB": 1},
    {"tdata": strToInt("{AMLongStride,0x100,32,3,0x5}"), "callTB": 1},
    {"tdata": 0, "callTB": 1}, # initial address
    {"tdata": 0xAABBCCDD}, # handler arg 0
    {"tdata": 0xDDCCBBAA, "callTB": 1}, # handler arg 1
])
for i in range(2):
    for j in range(3):
        axis_net.write(lmB_t1, 0xBEEFDEAD)
    axis_net.write(lmB_t1, 0xDEADBEEF)
for j in range(3):
    axis_net.write(lmB_t1, 0xBEEFDEAD)
axis_net.write(lmB_t1, 0xDEADBEEF, tlast=1, callTB=1)

lmB_t2 = long_message_B.add_thread()
axis_s2mmCommand.read(lmB_t2, strToInt("{dataMoverCommand,0x20,1,0,1,0,0,0}"))
axis_s2mmCommand.read(lmB_t2, strToInt("{dataMoverCommand,0x20,1,0,1,0,0x100,0}"))
axis_s2mmCommand.read(lmB_t2, strToInt("{dataMoverCommand,0x20,1,0,1,0,0x200,0}"))

lmB_t3 = long_message_B.add_thread()
# USE_ABS_PAYLOAD axis_xpams_rx.read(lmB_t3, strToInt("{AMHeader,0x01,0x02,0x88,0xD,0x5,2}"))
axis_xpams_rx.read(lmB_t3, strToInt("{AMHeader,0x01,0x02,0x60,0xD,0x5,2}"))
axis_xpams_rx.read(lmB_t3, strToInt("{AMLongStride,0x100,32,3,0x5}"))
axis_xpams_rx.read(lmB_t3, 0xAABBCCDD) # handler arg 0
axis_xpams_rx.read(lmB_t3, 0xDDCCBBAA) # handler arg 1
for i in range(3):
    for j in range(3):
        axis_s2mm.read(lmB_t3, 0xBEEFDEAD)
    axis_s2mm.read(lmB_t3, 0xDEADBEEF, tlast=1)
lmB_t3.add_delay('50ns')
axis_s2mmStatus.write(lmB_t3, 0x80)
axis_s2mmStatus.write(lmB_t3, 0x80)
axis_s2mmStatus.write(lmB_t3, 0x80, callTB=1)
lmB_t3.print_elapsed_time("long_message_B")
lmB_t3.end_vector()

#-------------------------------------------------------------------------------
# Long Message C
#
# Long vector message from 0xAA to 0xCC with 0x10C words of payload
# across two vectors. It calls handler 0xF with zero handler args. First 
# vector has 0xC words to address 0 and second has has 0x100 words to 
# address 0xFF00
#-------------------------------------------------------------------------------

long_message_C = TestVector()
long_message_C.add_thread(initT)

lmC_t1 = long_message_C.add_thread()
lmC_t1.add_delay('100ns')
lmC_t1.init_timer()
axis_net.writes(lmC_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,type:0x6,src:0x01,dst:0x02,payload:0x888,handler:0xF,args:0}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,type:0x6,src:0x01,dst:0x02,payload:0x860,handler:0xF,args:0}"), "callTB": 1},
    {"tdata": strToInt("{AMLongVector,dst:2,dstSize:0x60,token:0x6}")},
    {"tdata": 0}, # destination 0
    {"tdata": 0x800}, # size 1 
    {"tdata": 0xFF00, "callTB": 1}, # destination 1
])
for i in range(3):
    for j in range(3):
        axis_net.write(lmC_t1, 0xBEEFDEAD)
    axis_net.write(lmC_t1, 0xDEADBEEF)
for i in range(255):
    axis_net.write(lmC_t1, 0xFFAAFFAA)
axis_net.write(lmC_t1, 0xDDDDDDDD, tlast=1, callTB=2)

lmC_t2 = long_message_C.add_thread()
axis_s2mmCommand.read(lmC_t2, strToInt("{dataMoverCommand,0x60,1,0,1,0,0,0}"))
axis_s2mmCommand.read(lmC_t2, strToInt("{dataMoverCommand,0x800,1,0,1,0,0xFF00,0}"))

lmC_t3 = long_message_C.add_thread()
axis_xpams_rx.reads(lmC_t3, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,0x888,0xF,0x6,0}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0x860,0xF,0x6,0}")},
    {"tdata": strToInt("{AMLongVector,2,0x60,0x6}")}
])
for i in range(2):
    for j in range(3):
        axis_s2mm.read(lmC_t2, 0xBEEFDEAD)
    axis_s2mm.read(lmC_t2, 0xDEADBEEF)
for j in range(3):
    axis_s2mm.read(lmC_t2, 0xBEEFDEAD)
axis_s2mm.read(lmC_t2, 0xDEADBEEF, tlast=1)
for i in range(255):
    axis_s2mm.read(lmC_t2, 0xFFAAFFAA)
axis_s2mm.read(lmC_t2, 0xDDDDDDDD, tlast=1)

lmC_t4 = long_message_C.add_thread()
lmC_t4.wait_level('axis_s2mm_tlast == $value', value=1)
lmC_t4.wait_level('axis_s2mm_tlast == $value', value=1)
lmC_t4.add_delay('50ns')
axis_s2mmStatus.write(lmC_t4, 0x80)
axis_s2mmStatus.write(lmC_t4, 0x80, callTB=1)
lmC_t4.print_elapsed_time("long_message_C")
lmC_t4.end_vector()

am_rx.add_test_vector(short_message_A)
am_rx.add_test_vector(short_message_B)
am_rx.add_test_vector(medium_message_A)
am_rx.add_test_vector(medium_message_B)
am_rx.add_test_vector(long_message_A)
am_rx.add_test_vector(long_message_B)
am_rx.add_test_vector(long_message_C)

am_rx.generateTB(filepath, 'all')
