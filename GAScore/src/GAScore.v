`define S_AXILITE_INSTANTIATE(n) \
    .s_axi_ctrl_bus_``n``_AWVALID(s_axi_ctrl_bus_``n``_AWVALID), \
    .s_axi_ctrl_bus_``n``_AWREADY(s_axi_ctrl_bus_``n``_AWREADY), \
    .s_axi_ctrl_bus_``n``_AWADDR(s_axi_ctrl_bus_``n``_AWADDR), \
    .s_axi_ctrl_bus_``n``_WVALID(s_axi_ctrl_bus_``n``_WVALID), \
    .s_axi_ctrl_bus_``n``_WREADY(s_axi_ctrl_bus_``n``_WREADY), \
    .s_axi_ctrl_bus_``n``_WDATA(s_axi_ctrl_bus_``n``_WDATA), \
    .s_axi_ctrl_bus_``n``_WSTRB(s_axi_ctrl_bus_``n``_WSTRB), \
    .s_axi_ctrl_bus_``n``_ARVALID(s_axi_ctrl_bus_``n``_ARVALID), \
    .s_axi_ctrl_bus_``n``_ARREADY(s_axi_ctrl_bus_``n``_ARREADY), \
    .s_axi_ctrl_bus_``n``_ARADDR(s_axi_ctrl_bus_``n``_ARADDR), \
    .s_axi_ctrl_bus_``n``_RVALID(s_axi_ctrl_bus_``n``_RVALID), \
    .s_axi_ctrl_bus_``n``_RREADY(s_axi_ctrl_bus_``n``_RREADY), \
    .s_axi_ctrl_bus_``n``_RDATA(s_axi_ctrl_bus_``n``_RDATA), \
    .s_axi_ctrl_bus_``n``_RRESP(s_axi_ctrl_bus_``n``_RRESP), \
    .s_axi_ctrl_bus_``n``_BVALID(s_axi_ctrl_bus_``n``_BVALID), \
    .s_axi_ctrl_bus_``n``_BREADY(s_axi_ctrl_bus_``n``_BREADY), \
    .s_axi_ctrl_bus_``n``_BRESP(s_axi_ctrl_bus_``n``_BRESP)

`define S_AXILITE_PORTS(n) \
    input wire s_axi_ctrl_bus_``n``_AWVALID, \
    output wire s_axi_ctrl_bus_``n``_AWREADY, \
    input wire [ADDR_WIDTH - 1:0] s_axi_ctrl_bus_``n``_AWADDR, \
    input wire s_axi_ctrl_bus_``n``_WVALID, \
    output wire s_axi_ctrl_bus_``n``_WREADY, \
    input wire [DATA_WIDTH - 1:0] s_axi_ctrl_bus_``n``_WDATA, \
    input wire [STRB_WIDTH - 1:0] s_axi_ctrl_bus_``n``_WSTRB, \
    input wire s_axi_ctrl_bus_``n``_ARVALID, \
    output wire s_axi_ctrl_bus_``n``_ARREADY, \
    input wire [ADDR_WIDTH - 1:0] s_axi_ctrl_bus_``n``_ARADDR, \
    output wire s_axi_ctrl_bus_``n``_RVALID, \
    input wire s_axi_ctrl_bus_``n``_RREADY, \
    output wire [DATA_WIDTH - 1:0] s_axi_ctrl_bus_``n``_RDATA, \
    output wire [1:0] s_axi_ctrl_bus_``n``_RRESP, \
    output wire s_axi_ctrl_bus_``n``_BVALID, \
    input wire s_axi_ctrl_bus_``n``_BREADY, \
    output wire [1:0] s_axi_ctrl_bus_``n``_BRESP

`define AXI_INSTANTIATE(n,m) \
    .s_axi_``n``_awid(s_axi_``m``_awid), \
    .s_axi_``n``_awaddr(s_axi_``m``_awaddr), \
    .s_axi_``n``_awlen(s_axi_``m``_awlen), \
    .s_axi_``n``_awsize(s_axi_``m``_awsize), \
    .s_axi_``n``_awburst(s_axi_``m``_awburst), \
    .s_axi_``n``_awlock(s_axi_``m``_awlock), \
    .s_axi_``n``_awcache(s_axi_``m``_awcache), \
    .s_axi_``n``_awprot(s_axi_``m``_awprot), \
    .s_axi_``n``_awqos(s_axi_``m``_awqos), \
    .s_axi_``n``_awvalid(s_axi_``m``_awvalid), \
    .s_axi_``n``_awready(s_axi_``m``_awready), \
    .s_axi_``n``_wdata(s_axi_``m``_wdata), \
    .s_axi_``n``_wstrb(s_axi_``m``_wstrb), \
    .s_axi_``n``_wlast(s_axi_``m``_wlast), \
    .s_axi_``n``_wvalid(s_axi_``m``_wvalid), \
    .s_axi_``n``_wready(s_axi_``m``_wready), \
    .s_axi_``n``_bid(s_axi_``m``_bid), \
    .s_axi_``n``_bresp(s_axi_``m``_bresp), \
    .s_axi_``n``_bvalid(s_axi_``m``_bvalid), \
    .s_axi_``n``_bready(s_axi_``m``_bready), \
    .s_axi_``n``_arid(s_axi_``m``_arid), \
    .s_axi_``n``_araddr(s_axi_``m``_araddr), \
    .s_axi_``n``_arlen(s_axi_``m``_arlen), \
    .s_axi_``n``_arsize(s_axi_``m``_arsize), \
    .s_axi_``n``_arburst(s_axi_``m``_arburst), \
    .s_axi_``n``_arlock(s_axi_``m``_arlock), \
    .s_axi_``n``_arcache(s_axi_``m``_arcache), \
    .s_axi_``n``_arprot(s_axi_``m``_arprot), \
    .s_axi_``n``_arqos(s_axi_``m``_arqos), \
    .s_axi_``n``_arvalid(s_axi_``m``_arvalid), \
    .s_axi_``n``_arready(s_axi_``m``_arready), \
    .s_axi_``n``_rid(s_axi_``m``_rid), \
    .s_axi_``n``_rdata(s_axi_``m``_rdata), \
    .s_axi_``n``_rresp(s_axi_``m``_rresp), \
    .s_axi_``n``_rlast(s_axi_``m``_rlast), \
    .s_axi_``n``_rvalid(s_axi_``m``_rvalid), \
    .s_axi_``n``_rready(s_axi_``m``_rready)

`define AXI_PORT_LIST(n) \
    input wire [ID_WIDTH-1 : 0] s_axi_``n``_awid, \
    input wire [(32*1)-1 : 0] s_axi_``n``_awaddr, \
    input wire [(8*1)-1 : 0] s_axi_``n``_awlen, \
    input wire [(3*1)-1 : 0] s_axi_``n``_awsize, \
    input wire [(2*1)-1 : 0] s_axi_``n``_awburst, \
    input wire [1-1 : 0] s_axi_``n``_awlock, \
    input wire [(4*1)-1 : 0] s_axi_``n``_awcache, \
    input wire [(3*1)-1 : 0] s_axi_``n``_awprot, \
    input wire [(4*1)-1 : 0] s_axi_``n``_awqos, \
    input wire [1-1 : 0] s_axi_``n``_awvalid, \
    output wire [1-1 : 0] s_axi_``n``_awready, \
    input wire [(64*1)-1 : 0] s_axi_``n``_wdata, \
    input wire [(8*1)-1 : 0] s_axi_``n``_wstrb, \
    input wire [1-1 : 0] s_axi_``n``_wlast, \
    input wire [1-1 : 0] s_axi_``n``_wvalid, \
    output wire [1-1 : 0] s_axi_``n``_wready, \
    output wire [ID_WIDTH-1 : 0] s_axi_``n``_bid, \
    output wire [(2*1)-1 : 0] s_axi_``n``_bresp, \
    output wire [1-1 : 0] s_axi_``n``_bvalid, \
    input wire [1-1 : 0] s_axi_``n``_bready, \
    input wire [ID_WIDTH-1 : 0] s_axi_``n``_arid, \
    input wire [(32*1)-1 : 0] s_axi_``n``_araddr, \
    input wire [(8*1)-1 : 0] s_axi_``n``_arlen, \
    input wire [(3*1)-1 : 0] s_axi_``n``_arsize, \
    input wire [(2*1)-1 : 0] s_axi_``n``_arburst, \
    input wire [1-1 : 0] s_axi_``n``_arlock, \
    input wire [(4*1)-1 : 0] s_axi_``n``_arcache, \
    input wire [(3*1)-1 : 0] s_axi_``n``_arprot, \
    input wire [(4*1)-1 : 0] s_axi_``n``_arqos, \
    input wire [1-1 : 0] s_axi_``n``_arvalid, \
    output wire [1-1 : 0] s_axi_``n``_arready, \
    output wire [ID_WIDTH-1 : 0] s_axi_``n``_rid, \
    output wire [(64*1)-1 : 0] s_axi_``n``_rdata, \
    output wire [(2*1)-1 : 0] s_axi_``n``_rresp, \
    output wire [1-1 : 0] s_axi_``n``_rlast, \
    output wire [1-1 : 0] s_axi_``n``_rvalid, \
    input wire [1-1 : 0] s_axi_``n``_rready

module GAScore #(
    parameter ADDRESS_LOW = 0,
    parameter ADDRESS_HIGH = 15,
    parameter NUM_KERNELS = 2,
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 32,
    parameter STRB_WIDTH = DATA_WIDTH/8,
    parameter ID_WIDTH = NUM_KERNELS == 1 ? 1 : $clog2(NUM_KERNELS)
)(
    input clock,
    input reset_n,
    input axis_kernel_in_tvalid,
    output axis_kernel_in_tready,
    input [63:0] axis_kernel_in_tdata,
    input axis_kernel_in_tlast,
    input [7:0] axis_kernel_in_tkeep,
    input axis_net_in_tvalid,
    output axis_net_in_tready,
    output [63:0] axis_net_out_tdata,
    output [7:0] axis_net_out_tkeep,
    output axis_net_out_tlast,
    output axis_net_out_tvalid,
    input axis_net_out_tready,
    input [63:0] axis_net_in_tdata,
    input axis_net_in_tlast,
    input [7:0] axis_net_in_tkeep,
    output axis_kernel_out_tvalid,
    input axis_kernel_out_tready,
    output [63:0] axis_kernel_out_tdata,
    output [7:0] axis_kernel_out_tkeep,
    output axis_kernel_out_tlast,
    output [15:0] axis_kernel_out_tdest,
    output [31:0] axi_remote_araddr,
    output [1:0] axi_remote_arburst,
    output [3:0] axi_remote_arcache,
    output [7:0] axi_remote_arlen,
    output [0:0] axi_remote_arlock,
    output [2:0] axi_remote_arprot,
    output [3:0] axi_remote_arqos,
    input axi_remote_arready,
    // output [3:0] axi_remote_arregion,
    output [2:0] axi_remote_arsize,
    output axi_remote_arvalid,
    output [31:0] axi_remote_awaddr,
    output [1:0] axi_remote_awburst,
    output [3:0] axi_remote_awcache,
    output [7:0] axi_remote_awlen,
    output [0:0] axi_remote_awlock,
    output [2:0] axi_remote_awprot,
    output [3:0] axi_remote_awqos,
    input axi_remote_awready,
    // output [3:0] axi_remote_awregion,
    output [2:0] axi_remote_awsize,
    output axi_remote_awvalid,
    output axi_remote_bready,
    input [1:0] axi_remote_bresp,
    input axi_remote_bvalid,
    input [63:0] axi_remote_rdata,
    input axi_remote_rlast,
    output axi_remote_rready,
    input [1:0] axi_remote_rresp,
    input axi_remote_rvalid,
    output [63:0] axi_remote_wdata,
    output axi_remote_wlast,
    input axi_remote_wready,
    output [7:0] axi_remote_wstrb,
    output axi_remote_wvalid,
    `S_AXILITE_PORTS(00),
    `S_AXILITE_PORTS(01),
    `S_AXILITE_PORTS(02),
    `S_AXILITE_PORTS(03),
    `S_AXILITE_PORTS(04),
    `S_AXILITE_PORTS(05),
    `S_AXILITE_PORTS(06),
    `S_AXILITE_PORTS(07),
    `S_AXILITE_PORTS(08),
    `S_AXILITE_PORTS(09),
    `S_AXILITE_PORTS(10),
    `S_AXILITE_PORTS(11),
    `S_AXILITE_PORTS(12),
    `S_AXILITE_PORTS(13),
    `S_AXILITE_PORTS(14),
    `S_AXILITE_PORTS(15),
    `AXI_PORT_LIST(00),
    `AXI_PORT_LIST(01),
    `AXI_PORT_LIST(02),
    `AXI_PORT_LIST(03),
    `AXI_PORT_LIST(04),
    `AXI_PORT_LIST(05),
    `AXI_PORT_LIST(06),
    `AXI_PORT_LIST(07),
    `AXI_PORT_LIST(08),
    `AXI_PORT_LIST(09),
    `AXI_PORT_LIST(10),
    `AXI_PORT_LIST(11),
    `AXI_PORT_LIST(12),
    `AXI_PORT_LIST(13),
    `AXI_PORT_LIST(14),
    `AXI_PORT_LIST(15),
    output wire [ID_WIDTH-1 : 0] axi_local_awid,
    output wire [31 : 0] axi_local_awaddr,
    output wire [7 : 0] axi_local_awlen,
    output wire [2 : 0] axi_local_awsize,
    output wire [1 : 0] axi_local_awburst,
    output wire [0 : 0] axi_local_awlock,
    output wire [3 : 0] axi_local_awcache,
    output wire [2 : 0] axi_local_awprot,
    output wire [3 : 0] axi_local_awregion,
    output wire [3 : 0] axi_local_awqos,
    output wire [0 : 0] axi_local_awvalid,
    input wire [0 : 0] axi_local_awready,
    output wire [63 : 0] axi_local_wdata,
    output wire [7 : 0] axi_local_wstrb,
    output wire [0 : 0] axi_local_wlast,
    output wire [0 : 0] axi_local_wvalid,
    input wire [0 : 0] axi_local_wready,
    input wire [ID_WIDTH-1 : 0] axi_local_bid,
    input wire [1 : 0] axi_local_bresp,
    input wire [0 : 0] axi_local_bvalid,
    output wire [0 : 0] axi_local_bready,
    output wire [ID_WIDTH-1 : 0] axi_local_arid,
    output wire [31 : 0] axi_local_araddr,
    output wire [7 : 0] axi_local_arlen,
    output wire [2 : 0] axi_local_arsize,
    output wire [1 : 0] axi_local_arburst,
    output wire [0 : 0] axi_local_arlock,
    output wire [3 : 0] axi_local_arcache,
    output wire [2 : 0] axi_local_arprot,
    output wire [3 : 0] axi_local_arregion,
    output wire [3 : 0] axi_local_arqos,
    output wire [0 : 0] axi_local_arvalid,
    input wire [0 : 0] axi_local_arready,
    input wire [ID_WIDTH-1 : 0] axi_local_rid,
    input wire [63 : 0] axi_local_rdata,
    input wire [1 : 0] axi_local_rresp,
    input wire [0 : 0] axi_local_rlast,
    input wire [0 : 0] axi_local_rvalid,
    output wire [0 : 0] axi_local_rready,
    output interrupt_0,
    output interrupt_1,
    output interrupt_2,
    output interrupt_3,
    output interrupt_4,
    output interrupt_5,
    output interrupt_6,
    output interrupt_7,
    output interrupt_8,
    output interrupt_9,
    output interrupt_10,
    output interrupt_11,
    output interrupt_12,
    output interrupt_13,
    output interrupt_14,
    output interrupt_15
);

    wire [63:0] axis_handler_tdata;
    wire axis_handler_tlast;
    wire axis_handler_tvalid;
    wire axis_handler_tready;

    GAScore_bd_wrapper GAScore_bd_i(
        .clk(clock),
        .reset_n(reset_n),
        .axis_kernel_in_tvalid(axis_kernel_in_tvalid),
        .axis_kernel_in_tready(axis_kernel_in_tready),
        .axis_kernel_in_tdata(axis_kernel_in_tdata),
        .axis_kernel_in_tlast(axis_kernel_in_tlast),
        .axis_kernel_in_tkeep(axis_kernel_in_tkeep),
        .axis_net_in_tdata(axis_net_in_tdata),
        .axis_net_in_tkeep(axis_net_in_tkeep),
        .axis_net_in_tlast(axis_net_in_tlast),
        .axis_net_in_tready(axis_net_in_tready),
        .axis_net_in_tvalid(axis_net_in_tvalid),
        .axis_net_out_tdata(axis_net_out_tdata),
        .axis_net_out_tkeep(axis_net_out_tkeep),
        .axis_net_out_tlast(axis_net_out_tlast),
        .axis_net_out_tready(axis_net_out_tready),
        .axis_net_out_tvalid(axis_net_out_tvalid),
        .axis_kernel_out_tvalid(axis_kernel_out_tvalid),
        .axis_kernel_out_tready(axis_kernel_out_tready),
        .axis_kernel_out_tdata(axis_kernel_out_tdata),
        .axis_kernel_out_tkeep(axis_kernel_out_tkeep),
        .axis_kernel_out_tlast(axis_kernel_out_tlast),
        .axis_kernel_out_tdest(axis_kernel_out_tdest),
        .axis_handler_tvalid(axis_handler_tvalid),
        .axis_handler_tready(axis_handler_tready),
        .axis_handler_tdata(axis_handler_tdata),
        .axis_handler_tlast(axis_handler_tlast),
        .axi_mem_awaddr(axi_remote_awaddr),
//        .axi_mem_awid(axi_remote_awid),
        .axi_mem_awlen(axi_remote_awlen),
        .axi_mem_awsize(axi_remote_awsize),
        .axi_mem_awburst(axi_remote_awburst),
        .axi_mem_awlock(axi_remote_awlock),
        .axi_mem_awcache(axi_remote_awcache),
        .axi_mem_awprot(axi_remote_awprot),
//        .axi_mem_awregion(axi_remote_awregion),
        .axi_mem_awqos(axi_remote_awqos),
        .axi_mem_awvalid(axi_remote_awvalid),
        .axi_mem_awready(axi_remote_awready),
        .axi_mem_wdata(axi_remote_wdata),
        .axi_mem_wstrb(axi_remote_wstrb),
        .axi_mem_wlast(axi_remote_wlast),
        .axi_mem_wvalid(axi_remote_wvalid),
        .axi_mem_wready(axi_remote_wready),
        .axi_mem_bresp(axi_remote_bresp),
        .axi_mem_bvalid(axi_remote_bvalid),
//        .axi_mem_bid(axi_remote_bid),
        .axi_mem_bready(axi_remote_bready),
        .axi_mem_araddr(axi_remote_araddr),
//        .axi_mem_arid(axi_remote_arid),
        .axi_mem_arlen(axi_remote_arlen),
        .axi_mem_arsize(axi_remote_arsize),
        .axi_mem_arburst(axi_remote_arburst),
        .axi_mem_arlock(axi_remote_arlock),
        .axi_mem_arcache(axi_remote_arcache),
        .axi_mem_arprot(axi_remote_arprot),
//        .axi_mem_arregion(axi_remote_arregion),
        .axi_mem_arqos(axi_remote_arqos),
        .axi_mem_arvalid(axi_remote_arvalid),
        .axi_mem_arready(axi_remote_arready),
        .axi_mem_rdata(axi_remote_rdata),
//        .axi_mem_rid(axi_remote_rid),
        .axi_mem_rresp(axi_remote_rresp),
        .axi_mem_rlast(axi_remote_rlast),
        .axi_mem_rvalid(axi_remote_rvalid),
        .axi_mem_rready(axi_remote_rready),
        .address_low(ADDRESS_LOW),
        .address_high(ADDRESS_HIGH)
    );

    memory #(
        .KERNELS(NUM_KERNELS),
        .ID_WIDTH(ID_WIDTH)
    ) memory_i (
        `AXI_INSTANTIATE(0,00),
        `AXI_INSTANTIATE(1,01),
        `AXI_INSTANTIATE(2,02),
        `AXI_INSTANTIATE(3,03),
        `AXI_INSTANTIATE(4,04),
        `AXI_INSTANTIATE(5,05),
        `AXI_INSTANTIATE(6,06),
        `AXI_INSTANTIATE(7,07),
        `AXI_INSTANTIATE(8,08),
        `AXI_INSTANTIATE(9,09),
        `AXI_INSTANTIATE(10,10),
        `AXI_INSTANTIATE(11,11),
        `AXI_INSTANTIATE(12,12),
        `AXI_INSTANTIATE(13,13),
        `AXI_INSTANTIATE(14,14),
        `AXI_INSTANTIATE(15,15),
        .m_axi_awid(axi_local_awid),
        .m_axi_awaddr(axi_local_awaddr),
        .m_axi_awlen(axi_local_awlen),
        .m_axi_awsize(axi_local_awsize),
        .m_axi_awburst(axi_local_awburst),
        .m_axi_awlock(axi_local_awlock),
        .m_axi_awcache(axi_local_awcache),
        .m_axi_awprot(axi_local_awprot),
        .m_axi_awregion(axi_local_awregion),
        .m_axi_awqos(axi_local_awqos),
        .m_axi_awvalid(axi_local_awvalid),
        .m_axi_awready(axi_local_awready),
        .m_axi_wdata(axi_local_wdata),
        .m_axi_wstrb(axi_local_wstrb),
        .m_axi_wlast(axi_local_wlast),
        .m_axi_wvalid(axi_local_wvalid),
        .m_axi_wready(axi_local_wready),
        .m_axi_bid(axi_local_bid),
        .m_axi_bresp(axi_local_bresp),
        .m_axi_bvalid(axi_local_bvalid),
        .m_axi_bready(axi_local_bready),
        .m_axi_arid(axi_local_arid),
        .m_axi_araddr(axi_local_araddr),
        .m_axi_arlen(axi_local_arlen),
        .m_axi_arsize(axi_local_arsize),
        .m_axi_arburst(axi_local_arburst),
        .m_axi_arlock(axi_local_arlock),
        .m_axi_arcache(axi_local_arcache),
        .m_axi_arprot(axi_local_arprot),
        .m_axi_arregion(axi_local_arregion),
        .m_axi_arqos(axi_local_arqos),
        .m_axi_arvalid(axi_local_arvalid),
        .m_axi_arready(axi_local_arready),
        .m_axi_rid(axi_local_rid),
        .m_axi_rdata(axi_local_rdata),
        .m_axi_rresp(axi_local_rresp),
        .m_axi_rlast(axi_local_rlast),
        .m_axi_rvalid(axi_local_rvalid),
        .m_axi_rready(axi_local_rready)
    );

    handler_wrapper #(
        .NUM_KERNELS(NUM_KERNELS),
        .ADDR_WIDTH(ADDR_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .STRB_WIDTH(STRB_WIDTH)
    ) handler_wrapper_i (
        .clock(clock),
        .reset_n(reset_n),
        `S_AXILITE_INSTANTIATE(00),
        `S_AXILITE_INSTANTIATE(01),
        `S_AXILITE_INSTANTIATE(02),
        `S_AXILITE_INSTANTIATE(03),
        `S_AXILITE_INSTANTIATE(04),
        `S_AXILITE_INSTANTIATE(05),
        `S_AXILITE_INSTANTIATE(06),
        `S_AXILITE_INSTANTIATE(07),
        `S_AXILITE_INSTANTIATE(08),
        `S_AXILITE_INSTANTIATE(09),
        `S_AXILITE_INSTANTIATE(10),
        `S_AXILITE_INSTANTIATE(11),
        `S_AXILITE_INSTANTIATE(12),
        `S_AXILITE_INSTANTIATE(13),
        `S_AXILITE_INSTANTIATE(14),
        `S_AXILITE_INSTANTIATE(15),
        .axis_handler_tdata(axis_handler_tdata),
        .axis_handler_tlast(axis_handler_tlast),
        .axis_handler_tready(axis_handler_tready),
        .axis_handler_tvalid(axis_handler_tvalid),
        .address_offset(ADDRESS_LOW),
        .interrupt_0(interrupt_0),
        .interrupt_1(interrupt_1),
        .interrupt_2(interrupt_2),
        .interrupt_3(interrupt_3),
        .interrupt_4(interrupt_4),
        .interrupt_5(interrupt_5),
        .interrupt_6(interrupt_6),
        .interrupt_7(interrupt_7),
        .interrupt_8(interrupt_8),
        .interrupt_9(interrupt_9),
        .interrupt_10(interrupt_10),
        .interrupt_11(interrupt_11),
        .interrupt_12(interrupt_12),
        .interrupt_13(interrupt_13),
        .interrupt_14(interrupt_14),
        .interrupt_15(interrupt_15)
    );
    
    // blk_mem_gen_0 memory (
    //   .s_aclk(clock),                // input wire s_aclk
    //   .s_aresetn(reset_n),          // input wire s_aresetn
    //   .s_axi_awid('0),        // input wire [3 : 0] s_axi_awid
    //   .s_axi_awaddr(axi_mem_awaddr),    // input wire [31 : 0] s_axi_awaddr
    //   .s_axi_awlen(axi_mem_awlen),      // input wire [7 : 0] s_axi_awlen
    //   .s_axi_awsize(axi_mem_awsize),    // input wire [2 : 0] s_axi_awsize
    //   .s_axi_awburst(axi_mem_awburst),  // input wire [1 : 0] s_axi_awburst
    //   .s_axi_awvalid(axi_mem_awvalid),  // input wire s_axi_awvalid
    //   .s_axi_awready(axi_mem_awready),  // output wire s_axi_awready
    //   .s_axi_wdata(axi_mem_wdata),      // input wire [63 : 0] s_axi_wdata
    //   .s_axi_wstrb(axi_mem_wstrb),      // input wire [7 : 0] s_axi_wstrb
    //   .s_axi_wlast(axi_mem_wlast),      // input wire s_axi_wlast
    //   .s_axi_wvalid(axi_mem_wvalid),    // input wire s_axi_wvalid
    //   .s_axi_wready(axi_mem_wready),    // output wire s_axi_wready
    //   .s_axi_bid(),          // output wire [3 : 0] s_axi_bid
    //   .s_axi_bresp(axi_mem_bresp),      // output wire [1 : 0] s_axi_bresp
    //   .s_axi_bvalid(axi_mem_bvalid),    // output wire s_axi_bvalid
    //   .s_axi_bready(axi_mem_bready),    // input wire s_axi_bready
    //   .s_axi_arid('0),        // input wire [3 : 0] s_axi_arid
    //   .s_axi_araddr(axi_mem_araddr),    // input wire [31 : 0] s_axi_araddr
    //   .s_axi_arlen(axi_mem_arlen),      // input wire [7 : 0] s_axi_arlen
    //   .s_axi_arsize(axi_mem_arsize),    // input wire [2 : 0] s_axi_arsize
    //   .s_axi_arburst(axi_mem_arburst),  // input wire [1 : 0] s_axi_arburst
    //   .s_axi_arvalid(axi_mem_arvalid),  // input wire s_axi_arvalid
    //   .s_axi_arready(axi_mem_arready),  // output wire s_axi_arready
    //   .s_axi_rid(),          // output wire [3 : 0] s_axi_rid
    //   .s_axi_rdata(axi_mem_rdata),      // output wire [63 : 0] s_axi_rdata
    //   .s_axi_rresp(axi_mem_rresp),      // output wire [1 : 0] s_axi_rresp
    //   .s_axi_rlast(axi_mem_rlast),      // output wire s_axi_rlast
    //   .s_axi_rvalid(axi_mem_rvalid),    // output wire s_axi_rvalid
    //   .s_axi_rready(axi_mem_rready)    // input wire s_axi_rready
    // );
    
endmodule
