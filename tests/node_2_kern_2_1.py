import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS
from sonar.profile import Profiler

node_2_kern_2_1 = Testbench.default('node_2_kern_2_1')
filepath = os.path.join(os.path.dirname(__file__), 'build/node_2_kern_2_1')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('interrupt_V', 'input')

axis_in = AXIS('in_r', 'slave', 'ap_clk')
axis_in.port.init_channels('tkeep', 64, True)
axis_in.port.add_channel('TDEST', 'tdest', 8)
dut.add_interface(axis_in)

axis_out = AXIS('out_r', 'master', 'ap_clk')
axis_out.port.init_channels('tkeep', 64, True)
axis_out.port.add_channel('TDEST', 'tdest', 8)
dut.add_interface(axis_out)

node_2_kern_2_1.add_module(dut)
profiled_data = Profiler(filepath + '/' + 'output.txt', 'PROFILE')

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('interrupt_V', 1)

#-------------------------------------------------------------------------------
# case_0
#
#-------------------------------------------------------------------------------

case_0 = TestVector()
case_0.add_thread(initT)

smA_t1 = case_0.add_thread()
smA_t1.add_delay('100ns')
# axis_in.writes(smA_t1, profiled_data.to_stream(profiled_data.read_data))
profiled_data.write(smA_t1, axis_in, axis_out)
smA_t1.print_elapsed_time("case_0")
smA_t1.end_vector()

# smA_t2 = case_0.add_thread()
# axis_out.writes(smA_t2, profiled_data.to_stream(profiled_data.write_data))
# smA_t2.print_elapsed_time("case_0")
# smA_t2.end_vector()

node_2_kern_2_1.add_test_vector(case_0)

node_2_kern_2_1.generateTB(filepath, 'sv')
