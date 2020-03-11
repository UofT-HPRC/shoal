import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS

from sonar_strToInt import strToInt

hold_buffer = Testbench.default('hold_buffer_dest')
filepath = os.path.join(os.path.dirname(__file__), 'build/hold_buffer_dest/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('dataRelease_V', 'input', 16)

axis_input = AXIS('axis_input', 'slave', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_input.port.init_channels('tkeep', 64, True)
axis_input.port.add_channel("TDEST", "tdest", 16)
dut.add_interface(axis_input)

axis_output = AXIS('axis_output', 'master', 'ap_clk', c_struct='axis_word', c_stream='uaxis_l')
axis_output.port.init_channels('tkeep', 64, True)
axis_output.port.add_channel("TDEST", "tdest", 16)
dut.add_interface(axis_output)

hold_buffer.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('axis_output_tready', 1)

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------

Release_A = TestVector()
Release_A.add_thread(initT)

rA_t1 = Thread()
rA_t1.add_delay('100ns')
rA_t1.init_timer()
rA_t1.set_signal('dataRelease_V', 1)
axis_input.writes(rA_t1, [
    {"tdata": 0xDEF, "callTB": 1},
    {"tdata": 0xFED, "callTB": 1},
])
Release_A.add_thread(rA_t1)

rA_t2 = Thread()
axis_output.read(rA_t2, 0xDEF)
axis_output.read(rA_t2, 0xFED)
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
rB_t1.add_delay('100ns')
rB_t1.init_timer()
axis_input.writes(rB_t1, [
    {"tdata": 0xDEF, "callTB": 1},
    {"tdata": 0xFED, "callTB": 1},
])
rB_t1.set_signal('dataRelease_V', 1)
Release_B.add_thread(rB_t1)

rB_t2 = Thread()
axis_output.read(rB_t2, 0xDEF)
axis_output.read(rB_t2, 0xFED)
rB_t2.print_elapsed_time("Release_B")
rB_t2.end_vector()
Release_B.add_thread(rB_t2)

hold_buffer.add_test_vector(Release_A)
hold_buffer.add_test_vector(Release_B)

hold_buffer.generateTB(filepath, 'all')
