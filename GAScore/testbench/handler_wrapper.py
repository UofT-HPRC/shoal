import os

from sonar.testbench import Testbench, Module, TestVector, Thread
from sonar.interfaces import AXIS, SAXILite

from sonar_strToInt import strToInt

handler_wrapper = Testbench.default('handler_wrapper')
filepath = os.path.join(os.path.dirname(__file__), 'build/handler_wrapper/')

dut = Module.default("DUT")
dut.add_clock_port('clock', '20ns')
dut.add_reset_port('reset_n')
dut.add_port('address_offset', 'input', 16)
# dut.add_port('interrupt_0', 'output')
# dut.add_port('interrupt_1', 'output')

axis_handler = AXIS('axis_handler', 'slave', 'clock', c_struct='axis_word', c_stream='uaxis_n')
axis_handler.port.init_channels('default', 64, False)
dut.add_interface(axis_handler)

ctrl_bus_0 = SAXILite('s_axi_ctrl_bus_00', 'clock', 'reset_n')
ctrl_bus_0.add_register('config', 0x10)
ctrl_bus_0.add_register('arg', 0x18)
ctrl_bus_0.add_register('counter', 0x20)
ctrl_bus_0.add_register('barrier', 0x28)
ctrl_bus_0.add_register('memory', 0x30)
ctrl_bus_0.set_address('4K', 0) # address range is 4K at an offset of 0
ctrl_bus_0.port.init_channels(mode='default', dataWidth=32, addrWidth=6)
dut.add_interface(ctrl_bus_0)

ctrl_bus_1 = SAXILite('s_axi_ctrl_bus_01', 'clock', 'reset_n')
ctrl_bus_1.add_register('config', 0x10)
ctrl_bus_1.add_register('arg', 0x18)
ctrl_bus_1.add_register('counter', 0x20)
ctrl_bus_1.add_register('barrier', 0x28)
ctrl_bus_1.add_register('memory', 0x30)
ctrl_bus_1.set_address('4K', 0) # address range is 4K at an offset of 0
ctrl_bus_1.port.init_channels(mode='default', dataWidth=32, addrWidth=6)
dut.add_interface(ctrl_bus_1)

handler_wrapper.add_module(dut)

################################################################################
# Test Vectors
################################################################################

# Initialization thread (added to each test vector to reset everything)
initT = Thread()
initT.init_signals()
initT.wait_negedge('clock')
initT.add_delay('40ns')
initT.set_signal('reset_n', 1)
initT.set_signal('address_offset', 0)

#-------------------------------------------------------------------------------
#
#-------------------------------------------------------------------------------

short_message_A = TestVector()
short_message_A.add_thread(initT)

smA_t1 = Thread()
smA_t1.add_delay('100ns')
smA_t1.init_timer()
# ctrl_bus_0.write(smA_t1, 'counter_threshold', 1)
# ctrl_bus_0.write(smA_t1, 'config', 2)
# ctrl_bus_1.write(smA_t1, 'counter_threshold', 4)
# ctrl_bus_1.write(smA_t1, 'config', 4)
axis_handler.write(smA_t1, strToInt("{AMHeader,0xAA,0x1,0xC,2,0x5,1}"))
axis_handler.write(smA_t1, 5, tlast=1)
# axis_handler.write(smA_t1, strToInt("{AMHeader,0xAA,0x0,0xC,1,0x5,0}"), tlast=1)
smA_t1.set_flag(0)
short_message_A.add_thread(smA_t1)

smA_t2 = short_message_A.add_thread()
smA_t2.wait_flag(0)
smA_t2.add_delay('200ns')
ctrl_bus_1.read(smA_t2, "counter", 5)
# ctrl_bus_0.read(smA_t2, "memory", 1)
smA_t2.print_elapsed_time("short_message_A")
smA_t2.end_vector()

handler_wrapper.add_test_vector(short_message_A)

handler_wrapper.generateTB(filepath, 'sv')
