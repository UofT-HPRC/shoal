# shoal

## To use

* Initialize the repository with `source init.sh /hpath/to/shoal /path/to/Vivado /path/to/Vivado_HLS vivado_version hls_version` e.g. `source init.sh /home/sharm294/Documents/shoal /opt/Xilinx/Vivado /opt/Xilinx/Vivado 2019.1 2019.1`
* Set the board to compile for: `shoal-update-board BOARD_NAME`
* Make the testbenches: `make -C ./GAScore config` 
* Make the GAScore IP: `.GAScore/run.sh export 1 1`
* Make library: `make lib`
* Make kernels e.g. 

`make node_2_kern_2 BUILD_SUFFIX=_0 K_START=0 K_END=0 MODE=x86`
`make node_2_kern_2 BUILD_SUFFIX=_1 KERNEL=kern1 MODE=HLS`

* Copy kernels and GAScore over to Galapagos/userIP
* Run 
    make -C $GALAPAGOS_PATH hlsmiddleware
    make -C $GALAPAGOS_PATH middleware LOGICALFILE=$SHOAL_PATH/tests/logical.json MAPFILE=$SHOAL_PATH/tests/map.json PROJECTNAME=node_2_kern_2


## Dependencies

shoal has been tested with Make, g++-7, and Boost 1.66 on Ubuntu 16.04.

Boost can be installed with:
`wget -O boost_1_66_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz/download && tar xzvf boost_1_66_0.tar.gz && cd boost_1_66_0/ && ./bootstrap.sh --prefix=/usr/local && ./b2 --with=all install && bash -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/local.conf' && ldconfig`
