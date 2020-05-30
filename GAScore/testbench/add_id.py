import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

add_id = Testbench.default('add_id')
filepath = os.path.join(os.path.dirname(__file__), 'build/add_id/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')

axis_input = AXIS('in', 'slave', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_input.port.init_channels('tkeep', 64, True)
axis_input.port.add_channel("TDEST", "tdest", 16)
dut.add_interface(axis_input)

axis_output = AXIS('out', 'master', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_output.port.init_channels('tkeep', 64, True)
axis_output.port.add_channel("TDEST", "tdest", 16)
# axis_output.port.add_channel("TID", "tid", 16)
# axis_output.port.add_channel("TUSER", "tuser", 16)
dut.add_interface(axis_output)

add_id.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('out_tready', 1)

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------

Release_A = TestVector()
Release_A.add_thread(initT)

rA_t1 = Thread()
rA_t1.wait_negedge('ap_clk')
rA_t1.add_delay('300ns')
rA_t1.init_timer()
axis_input.writes(rA_t1, [
    {"tdata": strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}")},
    {"tdata": strToInt("{AMToken,0xabcd}"), "tlast": 1},
])
Release_A.add_thread(rA_t1)

rA_t2 = Thread()
axis_output.read(rA_t2, strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}"))
axis_output.read(rA_t2, strToInt("{AMToken,0xabcd}"))
rA_t2.print_elapsed_time("Release_A")
rA_t2.end_vector()
Release_A.add_thread(rA_t2)

#-------------------------------------------------------------------------------
# Medium Message A
#
#
#-------------------------------------------------------------------------------

Release_B = TestVector()
Release_B.add_thread(initT)

rB_t1 = Thread()
rB_t1.wait_negedge('ap_clk')
rB_t1.add_delay('200ns')
rB_t1.init_timer()
for i in range(3):
    axis_input.writes(rB_t1, [
        {"tdata": strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}")},
        {"tdata": 0xdead},
        {"tdata": strToInt("{AMToken,0xabcd}"), "tlast": 1},
    ])
Release_B.add_thread(rB_t1)

rB_t2 = Thread()
for i in range(3):
    axis_output.read(rB_t2, strToInt("{AMHeader,0x1,0x10,0,1,0x41,0}"))
    axis_output.read(rB_t2, 0xdead)
    axis_output.read(rB_t2, strToInt("{AMToken,0xabcd}"))
rB_t2.print_elapsed_time("Release_B")
rB_t2.end_vector()
Release_B.add_thread(rB_t2)


add_id.add_test_vector(Release_A)
add_id.add_test_vector(Release_B)
add_id.add_test_vector(Release_A)
add_id.add_test_vector(Release_B)

add_id.generateTB(filepath, 'sv')
