import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

xpams_tx = Testbench.default('xpams_tx')
filepath = os.path.join(os.path.dirname(__file__), 'build/xpams_tx/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('address_offset_low_V', 'input', 16)
dut.add_port('address_offset_high_V', 'input', 16)

axis_kernel_in = AXIS('axis_kernel_in', 'slave', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_kernel_in.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_kernel_in)

axis_tx = AXIS('axis_tx', 'master', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_tx.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_tx)

axis_kernel_out = AXIS('axis_kernel_out', 'master', 'ap_clk', c_struct='axis_wordDest', c_stream='uaxis_m')
axis_kernel_out.port.init_channels('tkeep', 64, True)
axis_kernel_out.port.add_channel('TDEST', 'tdest', 16)
dut.add_interface(axis_kernel_out)

axis_handler = AXIS('axis_handler', 'master', 'ap_clk', c_struct='axis_wordNoKeep', c_stream='uaxis_n')
axis_handler.port.init_channels('default', 64, True)
dut.add_interface(axis_handler)
xpams_tx.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('axis_tx_tready', 1)
initT.set_signal('axis_kernel_out_tready', 1)
initT.set_signal('axis_handler_tready', 1)

#-------------------------------------------------------------------------------
# Short Message A
#
# Empty handler reply message
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t1 = Thread()
smA_t1.add_delay('100ns')
smA_t1.init_timer()
smA_t1.set_signal('address_offset_high_V', 15)
axis_kernel_in.writes(smA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,16,0,0x1,0}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0,0x1,0}")},
    {"tdata": strToInt("{AMToken,0x1}"), "callTB": 2},
])
short_message_A.add_thread(smA_t1)

smA_t2 = Thread()
axis_kernel_out.read(smA_t2, strToInt("{KernelHeader,0x41,0x1,0,1}"), tdest=1)
smA_t2.print_elapsed_time("Short_Message_A")
smA_t2.end_vector()
short_message_A.add_thread(smA_t2)

#-------------------------------------------------------------------------------
# Medium Message A
#
# 
#-------------------------------------------------------------------------------

medium_message_A = TestVector()
medium_message_A.add_thread(initT)

mmA_t1 = Thread()
mmA_t1.add_delay('100ns')
mmA_t1.set_signal('address_offset_high_V', 15)
mmA_t1.init_timer()
axis_kernel_in.writes(mmA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x2,0x808,0x0,0x2,0}")},
    {"tdata": strToInt("{AMHeader,0x01,0x2,0x800,0x0,0x2,0}")},
    {"tdata": strToInt("{AMToken,0x0}"), "callTB": 1},
])
for i in range(255):
    axis_kernel_in.write(mmA_t1, 0x98765432)
axis_kernel_in.write(mmA_t1, 0x98765432, tlast=1, callTB=3)
medium_message_A.add_thread(mmA_t1)

mmA_t2 = Thread()
axis_kernel_out.read(mmA_t2, strToInt("{KernelHeader,0x2,0x01,0x800,0}"), tdest=2)
# axis_kernel_out.read(mmA_t2, strToInt("{AMToken,0x0}"), tdest=2)
for i in range(255):
    axis_kernel_out.read(mmA_t2, 0x98765432, tdest=2)
axis_kernel_out.read(mmA_t2, 0x98765432, tdest=2, tlast=1)
axis_kernel_out.read(mmA_t2, strToInt("{KernelHeader,0x41,0x1,0,0}"), tdest=1)
mmA_t2.print_elapsed_time("Medium_Message_A")
mmA_t2.end_vector()
medium_message_A.add_thread(mmA_t2)

#-------------------------------------------------------------------------------
# Long Message A
#
# 
#-------------------------------------------------------------------------------

long_message_A = TestVector()
long_message_A.add_thread(initT)

lmA_t1 = Thread()
lmA_t1.add_delay('100ns')
lmA_t1.init_timer()
lmA_t1.set_signal('address_offset_high_V', 15)
axis_kernel_in.writes(lmA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x11,0x78,1,0x5,2}")},
    {"tdata": strToInt("{AMHeader,0x01,0x11,0x60,1,0x5,2}")},
    {"tdata": strToInt("{AMLongStride,0x100,4,3,2}")},
    {"tdata": 0xAABBCCDD},
    {"tdata": 0xDDCCBBAA, "tlast": 1, "callTB": 3}
])
long_message_A.add_thread(lmA_t1)

lmA_t2 = Thread()
# USE_ABS_PAYLOAD axis_tx.read(lmA_t2, strToInt("{AMHeader,0x01,0x11,0x78,1,0x5,2}"))
axis_tx.read(lmA_t2, strToInt("{AMHeader,0x01,0x11,0x60,1,0x5,2}"))
axis_tx.read(lmA_t2, strToInt("{AMLongStride,0x100,4,3,2}"))
axis_tx.read(lmA_t2, 0xAABBCCDD)
axis_tx.read(lmA_t2, 0xDDCCBBAA, tlast=1)
lmA_t2.print_elapsed_time("Long_Message_A")
lmA_t2.end_vector()
long_message_A.add_thread(lmA_t2)

xpams_tx.add_test_vector(short_message_A)
xpams_tx.add_test_vector(medium_message_A)
xpams_tx.add_test_vector(long_message_A)

xpams_tx.generateTB(filepath, 'all')
