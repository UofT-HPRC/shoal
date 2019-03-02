# Dummy testbench just for Vivado project scripts

import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

am_rx = Testbench.default('memory')
filepath = os.path.join(os.path.dirname(__file__), 'build/memory/')

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
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x0}"), "callTB": 1},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1, "callTB": 2}
])

smA_t2 = short_message_A.add_thread()
axis_xpams_rx.reads(smA_t2, [
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,2}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1}
])
smA_t2.print_elapsed_time("Short_Message_A")
smA_t2.end_vector()

am_rx.add_test_vector(short_message_A)

am_rx.generateTB(filepath, 'sv')
