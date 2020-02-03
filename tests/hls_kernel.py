import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS, SAXILite

hls_kernel = Testbench.default('hls_kernel')
filepath = os.path.join(os.path.dirname(__file__), 'build/hls_kernel/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('id_0', 'input', 16)
dut.add_port('id_1', 'input', 16)

hls_kernel.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('id_0', 0)
initT.set_signal('id_1', 1)

#-------------------------------------------------------------------------------
# case_0
#
#-------------------------------------------------------------------------------

case_0 = TestVector()
case_0.add_thread(initT)

smA_t1 = case_0.add_thread()
smA_t1.add_delay('100ns')
smA_t1.print_elapsed_time("case_0")
smA_t1.end_vector()

hls_kernel.add_test_vector(case_0)

hls_kernel.generateTB(filepath, 'sv')
