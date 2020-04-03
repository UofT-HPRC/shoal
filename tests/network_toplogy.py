#!/usr/bin/env python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost, OVSController
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel
from mininet.util import pmonitor
from signal import SIGINT
from time import time

class SingleSwitchTopo( Topo ):
    "Single switch connected to n hosts."
    def build( self, n=2 ):
        switch = self.addSwitch( 's1' )
        for h in range(n):
            # Each host gets 50%/n of system CPU
            host = self.addHost( 'h%s' % (h + 1),
                            cpu=.5/n )
            # 10 Mbps, 5ms delay, 2% loss, 1000 packet queue
            self.addLink( host, switch), #bw=10, delay='5ms', loss=2,
                            #max_queue_size=1000, use_htb=True )

def perfTest():
    "Create network and run simple performance test"
    topo = SingleSwitchTopo( n=2 )
    net = Mininet( topo=topo, link=TCLink, controller = OVSController)
    net.start()
    print("Dumping host connections")
    dumpNodeConnections( net.hosts )
    print("Testing network connectivity")
    net.pingAll()
    print("Testing bandwidth between h1 and h2")
    h1, h2 = net.get( 'h1', 'h2' )
    print(h1.IP())
    print(h2.IP())
    # h1_results = h1.cmd("./build/bin/node_2_kern_2_0")
    # h2_results = h2.cmd("./build/bin/node_2_kern_2_1")
    # net.iperf( (h1, h4) )
    seconds = 10
    popens = {}
    popens[ h1 ] = h1.popen("./build/bin/hls_kernel_0")
    popens[ h2 ] = h2.popen("./build/bin/hls_kernel_1")
    print "Monitoring output for", seconds, "seconds"
    endTime = time() + seconds
    for h, line in pmonitor( popens, timeoutms=500 ):
        if h:
            print('%s: %s' % ( h.name, line ))
        if time() >= endTime:
            for p in popens.values():
                p.send_signal( SIGINT )
    # print(h1_results)
    # print(h2_results)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    perfTest()