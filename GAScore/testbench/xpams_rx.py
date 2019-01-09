import os

from sonar import sonar as sonarTB
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

xpams_rx = sonarTB.Sonar.default('xpams_rx')
filepath = os.path.join(os.path.dirname(__file__), 'build/xpams_rx/')

dut = sonarTB.Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')

axis_rx = AXIS('axis_rx', 'slave', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_rx.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_rx)

axis_tx = AXIS('axis_tx', 'master', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_tx.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_tx)

axis_kernel_out = AXIS('axis_kernel_out', 'master', 'ap_clk', c_struct='axis_wordDest', c_stream='uaxis_m')
axis_kernel_out.port.init_channels('tkeep', 64, True)
axis_kernel_out.port.add_channel('TDEST', 'tdest', 16)
dut.add_interface(axis_kernel_out)

axis_handler = AXIS('axis_handler', 'master', 'ap_clk', c_struct='axis_wordNoKeep', c_stream='uaxis_n')
axis_handler.port.init_channels('default', 64, False)
dut.add_interface(axis_handler)
xpams_rx.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = sonarTB.Thread()
initT.init_signals()
initT.wait_edge('negedge', 'clock')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('axis_tx_TREADY', 1)
initT.set_signal('axis_kernel_out_TREADY', 1)
initT.set_signal('axis_handler_TREADY', 1)

#-------------------------------------------------------------------------------
# Short Message A
#
# Empty handler reply message
#-------------------------------------------------------------------------------

short_message_A = sonarTB.TestVector()
short_message_A.add_thread(initT)

smA_t1 = sonarTB.Thread()
smA_t1.add_delay('100ns')
smA_t1.init_timer()
axis_rx.write(smA_t1, strToInt("{AMHeader,0xCC,0x02,0,0,0x41,0}"))
axis_rx.write(smA_t1, strToInt("{AMToken,0x1}"), callTB=2)
short_message_A.add_thread(smA_t1)

smA_t2 = sonarTB.Thread()
axis_kernel_out.read(smA_t2, strToInt("{AMToken,0x1,0x41}"), tdest=2)
smA_t2.print_elapsed_time("Short_Message_A")
smA_t2.end_vector()
short_message_A.add_thread(smA_t2)

#-------------------------------------------------------------------------------
# Medium Message A
#
# This sends a AMheader + payload with no handler args to xpams. There's 
# an empty handler receiving it so it sends a reply message back and passes 
# the payload to the kernel
#-------------------------------------------------------------------------------

medium_message_A = sonarTB.TestVector()
medium_message_A.add_thread(initT)

mmA_t1 = sonarTB.Thread()
mmA_t1.add_delay('100ns')
mmA_t1.init_timer()
axis_rx.writes(mmA_t1, [
    {"tdata": strToInt("{AMHeader,0xAA,0x2,0x100,0x0,0x2,0}")},
    {"tdata": strToInt("{AMToken,0x0}"), "callTB": 1},
])
for i in range(255):
    axis_rx.write(mmA_t1, 0x98765432)
axis_rx.write(mmA_t1, 0x98765432, tlast=1, callTB=3)
medium_message_A.add_thread(mmA_t1)

mmA_t2 = sonarTB.Thread()
for i in range(255):
    axis_kernel_out.read(mmA_t2, 0x98765432, tdest=2)
axis_kernel_out.read(mmA_t2, 0x98765432, tdest=2, tlast=1)
medium_message_A.add_thread(mmA_t2)

mmA_t3 = sonarTB.Thread()
axis_tx.read(mmA_t3, strToInt("{AMHeader,0x2,0xAA,0,0x0,0x41,0}"))
axis_tx.read(mmA_t3, strToInt("{AMToken,0x0}"), tlast=1)
mmA_t3.print_elapsed_time("Medium_Message_A")
mmA_t3.end_vector()
medium_message_A.add_thread(mmA_t3)

#-------------------------------------------------------------------------------
# Medium Message C
#
# This sends a medium get message from kernel 0x10 to 0. GAScore 0 should send 
# a medium message send to the TX channel as if Kernel 0 itself initiated a send
#-------------------------------------------------------------------------------

medium_message_C = sonarTB.TestVector()
medium_message_C.add_thread(initT)

mmC_t1 = sonarTB.Thread()
mmC_t1.add_delay('100ns')
mmC_t1.init_timer()
axis_rx.write(mmC_t1, strToInt("{AMHeader,0x10,0,4,0,0x42,0}"), callTB=1),
axis_rx.write(mmC_t1, strToInt("{AMToken,0x0}")),
axis_rx.write(mmC_t1, 0x10, tlast=1, callTB=1)
medium_message_C.add_thread(mmC_t1)

mmC_t2 = sonarTB.Thread()
axis_tx.read(mmC_t2, strToInt("{AMHeader,0,0x10,4,0,0x22,0}"))
axis_tx.read(mmC_t2, strToInt("{AMToken,0x0}")),
axis_tx.read(mmC_t2, 0x10, tlast=1)
mmC_t2.print_elapsed_time("Medium_Message_C")
mmC_t2.end_vector()
medium_message_C.add_thread(mmC_t2)

#-------------------------------------------------------------------------------
# Long Message A
#
# This sends a AMheader + token with two handler args to xpams. The header + 
# handler args are forwarded to the handler and a reply message is sent back
#-------------------------------------------------------------------------------

long_message_A = sonarTB.TestVector()
long_message_A.add_thread(initT)

lmA_t1 = sonarTB.Thread()
lmA_t1.add_delay('100ns')
lmA_t1.init_timer()
axis_rx.writes(lmA_t1, [
    {"tdata": strToInt("{AMHeader,0xAA,0x2,0xC,1,0x5,2}")},
    {"tdata": strToInt("{AMLongStride,0x100,4,3,2}")},
    {"tdata": 0xAABBCCDD},
    {"tdata": 0xDDCCBBAA, "tlast": 1, "callTB": 3}
])
long_message_A.add_thread(lmA_t1)

lmA_t2 = sonarTB.Thread()
axis_handler.read(lmA_t2, strToInt("{AMHeader,0xAA,0x2,0xC,1,0x5,2}"))
axis_handler.read(lmA_t2, 0xAABBCCDD)
axis_handler.read(lmA_t2, 0xDDCCBBAA, tlast=1)
long_message_A.add_thread(lmA_t2)

lmA_t3 = sonarTB.Thread()
axis_tx.read(lmA_t3, strToInt("{AMHeader,0x2,0xAA,0,0,0x41,0}"))
axis_tx.read(lmA_t3, strToInt("{AMToken,0x2}"), tlast=1)
lmA_t3.print_elapsed_time("Long_Message_A")
lmA_t3.end_vector()
long_message_A.add_thread(lmA_t3)

xpams_rx.add_test_vector(short_message_A)
xpams_rx.add_test_vector(medium_message_A)
xpams_rx.add_test_vector(medium_message_C)
xpams_rx.add_test_vector(long_message_A)

xpams_rx.finalize_waits()
xpams_rx.generateTB(filepath, 'all')