# shoal

## To use

```bash
# download this repository and enter it
git clone www.github.com/sharm294/shoal.git
cd shoal

# initialize the repo
source init.sh /path/to/shoal /path/to/Vivado /path/to/Vivado_HLS vivado_version hls_version
# e.g. source init.sh /home/sharm294/shoal /opt/Xilinx/Vivado /opt/Xilinx/Vivado 2019.1 2019.1
# the paths for Vivado and HLS are separate because in earlier Vivado versions, HLS and Vivado were installed in different directories.

# update the board to target
shoal-update-board BOARD_NAME

# make testbenches
make -C ./GAScore config

# make the GAScore IP for this board part
# NOTE: GAScore should be built with Vivado 2017.2. Other versions may not work. For example, 2018.2 doesn't work.
.GAScore/run.sh export 1 1

# make the shoal library to link against
make lib
```

The test kernels can be made through these commands

```bash
# one x86 node with kernels 0 and 1
make galapagos-node_1_kern_2 K_START=0 K_END=1 MODE=x86
make galapagos-hls_kernel K_START=0 K_END=1 MODE=x86
make galapagos-hls_kernel K_START=0 K_END=0 MODE=x86 BUILD_SUFFIX=_0
make galapagos-benchmark K_START=0 K_END=2 MODE=x86
make galapagos-benchmark K_START=0 K_END=2 MODE=x86 KERN_BUILD=2 DEBUG=0

# one x86 node with kernel 0, one HW node with kernel 1
# NOTE: kernels fail to build in Vivado 2017.2 but 2018.2 works.
make galapagos-node_2_kern_2 BUILD_SUFFIX=_0 K_START=0 K_END=0 MODE=x86
make galapagos-node_2_kern_2 BUILD_SUFFIX=_1 KERNEL=kern1 MODE=HLS
make galapagos-hls_kernel BUILD_SUFFIX= MODE=HLS KERNEL=hls_kernel
make galapagos-benchmark MODE=HLS KERNEL=benchmark

```

If you're using multiple nodes, you'll need to set them up in Galapagos

```bash
# first copy kernels and GAScore over to $GALAPAGOS_PATH/userIP

$ make -C $GALAPAGOS_PATH hlsmiddleware
$ make -C $GALAPAGOS_PATH middleware LOGICALFILE=$SHOAL_PATH/tests/logical.json MAPFILE=$SHOAL_PATH/tests/map.json PROJECTNAME=<some name>
```


## Dependencies

shoal has been tested with Make, g++-7, and Boost 1.66 on Ubuntu 16.04.

Boost can be installed with:
`wget -O boost_1_66_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.gz/download && tar xzvf boost_1_66_0.tar.gz && cd boost_1_66_0/ && ./bootstrap.sh --prefix=/usr/local && ./b2 --with=all install && bash -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/local.conf' && ldconfig`

There are also some dependencies of building parts of the project with different versions of Vivado (see notes above).

conda: python=2.7, enum34
