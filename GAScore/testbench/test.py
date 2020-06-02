import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

test = Testbench.default('test')
filepath = os.path.join(os.path.dirname(__file__), 'build/test/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
# dut.add_port('release_V', 'output')

axis_in = AXIS('axis_in', 'slave', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_in.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_in)

axis_out = AXIS('axis_out', 'master', 'ap_clk', c_struct='axis_word', 
    c_stream='uaxis_l')
axis_out.port.init_channels('tkeep', 64, True)
dut.add_interface(axis_out)

test.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('axis_out_tready', 1)

#-------------------------------------------------------------------------------
# Short Message A
#
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t1 = short_message_A.add_thread()
smA_t1.add_delay('300ns')
smA_t1.init_timer()
axis_in.writes(smA_t1, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,48,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,2}"), "callTB": 1},
    {"tdata": strToInt("{AMToken,0x0}"), "callTB": 1},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1, "callTB": 2}
])

smA_t2 = short_message_A.add_thread()
axis_out.reads(smA_t2, [
    # USE_ABS_PAYLOAD {"tdata": strToInt("{AMHeader,0x01,0x02,48,0xE,1,2}")},
    {"tdata": strToInt("{AMHeader,0x01,0x02,0,0xE,1,2}")},
    {"tdata": strToInt("{AMToken,0x0}")},
    {"tdata": 0xDEADBEEF},
    {"tdata": 0x1234, "tlast": 1}
])
smA_t2.print_elapsed_time("Short_Message_A")
smA_t2.end_vector()

test.add_test_vector(short_message_A)
test.generateTB(filepath, 'all')