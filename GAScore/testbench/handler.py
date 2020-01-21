import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS, SAXILite

from sonar_strToInt import strToInt

handler = Testbench.default('handler')
filepath = os.path.join(os.path.dirname(__file__), 'build/handler/')

dut = Module.default("DUT")
dut.add_clock_port('ap_clk', '20ns')
dut.add_reset_port('ap_rst_n')
dut.add_port('AMhandler_V', 'input', 4)
# dut.add_port('interrupt_V', 'output')

axis_handler = AXIS('axis_handler', 'slave', 'ap_clk', c_struct='axis_word', c_stream='uaxis_n')
axis_handler.port.init_channels('default', 64, True)
dut.add_interface(axis_handler)

ctrl_bus = SAXILite('s_axi_ctrl_bus', 'ap_clk', 'ap_rst_n')
ctrl_bus.add_register('config', 0x10)
ctrl_bus.add_register('arg', 0x18)
ctrl_bus.add_register('counter', 0x20)
ctrl_bus.add_register('barrier', 0x28)
ctrl_bus.add_register('memory', 0x30)
ctrl_bus.set_address('4K', 0) # address range is 4K at an offset of 0
ctrl_bus.port.init_channels(mode='default', dataWidth=32, addrWidth=5)
dut.add_interface(ctrl_bus)

handler.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('ap_clk')
initT.add_delay('40ns')
initT.set_signal('ap_rst_n', 1)
initT.set_signal('AMhandler_V', 2)

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t1 = Thread()
smA_t1.add_delay('100ns')
smA_t1.init_timer()
# ctrl_bus.write(smA_t1, 'counter_threshold', 4)
# ctrl_bus.write(smA_t1, 'config', 4)
# ctrl_bus.read(smA_t1, "counter_valid", 1)
ctrl_bus.read(smA_t1, "counter", 0)
axis_handler.write(smA_t1, 5, tlast=1, callTB=2)
# ctrl_bus.read(smA_t1, "counter_valid", 0)
# ctrl_bus.read(smA_t1, "counter_valid", 1)
smA_t1.wait_negedge('ap_clk')
smA_t1.add_delay('40ns')
ctrl_bus.read(smA_t1, "counter", 5)
ctrl_bus.write(smA_t1, "arg", 0x4)
ctrl_bus.write(smA_t1, "config", 0x12)
ctrl_bus.write(smA_t1, "config", 0x02)
smA_t1.wait_negedge('ap_clk')
smA_t1.add_delay('40ns')
ctrl_bus.read(smA_t1, "counter", 1)
# smA_t1.wait_level('interrupt_V == $value', value=1)
# ctrl_bus.write(smA_t1, 'config', 0)
smA_t1.print_elapsed_time("short_message_A")
smA_t1.end_vector()
short_message_A.add_thread(smA_t1)

handler.add_test_vector(short_message_A)

handler.generateTB(filepath, 'all')
