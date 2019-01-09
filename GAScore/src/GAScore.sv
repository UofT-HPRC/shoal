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
    input logic s_axi_ctrl_bus_``n``_AWVALID, \
    output logic s_axi_ctrl_bus_``n``_AWREADY, \
    input logic [ADDR_WIDTH - 1:0] s_axi_ctrl_bus_``n``_AWADDR, \
    input logic s_axi_ctrl_bus_``n``_WVALID, \
    output logic s_axi_ctrl_bus_``n``_WREADY, \
    input logic [DATA_WIDTH - 1:0] s_axi_ctrl_bus_``n``_WDATA, \
    input logic [STRB_WIDTH - 1:0] s_axi_ctrl_bus_``n``_WSTRB, \
    input logic s_axi_ctrl_bus_``n``_ARVALID, \
    output logic s_axi_ctrl_bus_``n``_ARREADY, \
    input logic [ADDR_WIDTH - 1:0] s_axi_ctrl_bus_``n``_ARADDR, \
    output logic s_axi_ctrl_bus_``n``_RVALID, \
    input logic s_axi_ctrl_bus_``n``_RREADY, \
    output logic [DATA_WIDTH - 1:0] s_axi_ctrl_bus_``n``_RDATA, \
    output logic [1:0] s_axi_ctrl_bus_``n``_RRESP, \
    output logic s_axi_ctrl_bus_``n``_BVALID, \
    input logic s_axi_ctrl_bus_``n``_BREADY, \
    output logic [1:0] s_axi_ctrl_bus_``n``_BRESP

module GAScore #(
    parameter ADDRESS_LOW = 0,
    parameter ADDRESS_HIGH = 15,
    parameter NUM_KERNELS = 2,
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 32,
    parameter STRB_WIDTH = DATA_WIDTH/8
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
    // output [31:0]axi_mem_araddr,
    // output [1:0]axi_mem_arburst,
    // output [3:0]axi_mem_arcache,
    // output [7:0]axi_mem_arlen,
    // output [0:0]axi_mem_arlock,
    // output [2:0]axi_mem_arprot,
    // output [3:0]axi_mem_arqos,
    // input axi_mem_arready,
    // output [3:0]axi_mem_arregion,
    // output [2:0]axi_mem_arsize,
    // output axi_mem_arvalid,
    // output [31:0]axi_mem_awaddr,
    // output [1:0]axi_mem_awburst,
    // output [3:0]axi_mem_awcache,
    // output [7:0]axi_mem_awlen,
    // output [0:0]axi_mem_awlock,
    // output [2:0]axi_mem_awprot,
    // output [3:0]axi_mem_awqos,
    // input axi_mem_awready,
    // output [3:0]axi_mem_awregion,
    // output [2:0]axi_mem_awsize,
    // output axi_mem_awvalid,
    // output axi_mem_bready,
    // input [1:0]axi_mem_bresp,
    // input axi_mem_bvalid,
    // input [63:0]axi_mem_rdata,
    // input axi_mem_rlast,
    // output axi_mem_rready,
    // input [1:0]axi_mem_rresp,
    // input axi_mem_rvalid,
    // output [63:0]axi_mem_wdata,
    // output axi_mem_wlast,
    // input axi_mem_wready,
    // output [7:0]axi_mem_wstrb,
    // output axi_mem_wvalid,
    `S_AXILITE_PORTS(0),
    `S_AXILITE_PORTS(1),
    `S_AXILITE_PORTS(2),
    `S_AXILITE_PORTS(3),
    `S_AXILITE_PORTS(4),
    `S_AXILITE_PORTS(5),
    `S_AXILITE_PORTS(6),
    `S_AXILITE_PORTS(7),
    `S_AXILITE_PORTS(8),
    `S_AXILITE_PORTS(9),
    `S_AXILITE_PORTS(10),
    `S_AXILITE_PORTS(11),
    `S_AXILITE_PORTS(12),
    `S_AXILITE_PORTS(13),
    `S_AXILITE_PORTS(14),
    `S_AXILITE_PORTS(15),
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

    wire [31:0]axi_mem_araddr;
//    wire axi_mem_awid;
    wire [1:0]axi_mem_arburst;
    wire [3:0]axi_mem_arcache;
    wire [7:0]axi_mem_arlen;
    wire [0:0]axi_mem_arlock;
    wire [2:0]axi_mem_arprot;
    wire [3:0]axi_mem_arqos;
    wire axi_mem_arready;
//    wire [3:0]axi_mem_arregion;
    wire [2:0]axi_mem_arsize;
    wire axi_mem_arvalid;
//    wire axi_mem_arid;
    wire [31:0]axi_mem_awaddr;
    wire [1:0]axi_mem_awburst;
    wire [3:0]axi_mem_awcache;
    wire [7:0]axi_mem_awlen;
    wire [0:0]axi_mem_awlock;
    wire [2:0]axi_mem_awprot;
    wire [3:0]axi_mem_awqos;
    wire axi_mem_awready;
//    wire [3:0]axi_mem_awregion;
    wire [2:0]axi_mem_awsize;
    wire axi_mem_awvalid;
//    wire axi_mem_bid;
    wire axi_mem_bready;
    wire [1:0]axi_mem_bresp;
    wire axi_mem_bvalid;
    wire [63:0]axi_mem_rdata;
    wire axi_mem_rid;
    wire axi_mem_rlast;
    wire axi_mem_rready;
    wire [1:0]axi_mem_rresp;
    wire axi_mem_rvalid;
    wire [63:0]axi_mem_wdata;
    wire axi_mem_wlast;
    wire axi_mem_wready;
    wire [7:0]axi_mem_wstrb;
    wire axi_mem_wvalid;

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
        .axi_mem_awaddr(axi_mem_awaddr),
//        .axi_mem_awid(axi_mem_awid),
        .axi_mem_awlen(axi_mem_awlen),
        .axi_mem_awsize(axi_mem_awsize),
        .axi_mem_awburst(axi_mem_awburst),
        .axi_mem_awlock(axi_mem_awlock),
        .axi_mem_awcache(axi_mem_awcache),
        .axi_mem_awprot(axi_mem_awprot),
//        .axi_mem_awregion(axi_mem_awregion),
        .axi_mem_awqos(axi_mem_awqos),
        .axi_mem_awvalid(axi_mem_awvalid),
        .axi_mem_awready(axi_mem_awready),
        .axi_mem_wdata(axi_mem_wdata),
        .axi_mem_wstrb(axi_mem_wstrb),
        .axi_mem_wlast(axi_mem_wlast),
        .axi_mem_wvalid(axi_mem_wvalid),
        .axi_mem_wready(axi_mem_wready),
        .axi_mem_bresp(axi_mem_bresp),
        .axi_mem_bvalid(axi_mem_bvalid),
//        .axi_mem_bid(axi_mem_bid),
        .axi_mem_bready(axi_mem_bready),
        .axi_mem_araddr(axi_mem_araddr),
//        .axi_mem_arid(axi_mem_arid),
        .axi_mem_arlen(axi_mem_arlen),
        .axi_mem_arsize(axi_mem_arsize),
        .axi_mem_arburst(axi_mem_arburst),
        .axi_mem_arlock(axi_mem_arlock),
        .axi_mem_arcache(axi_mem_arcache),
        .axi_mem_arprot(axi_mem_arprot),
//        .axi_mem_arregion(axi_mem_arregion),
        .axi_mem_arqos(axi_mem_arqos),
        .axi_mem_arvalid(axi_mem_arvalid),
        .axi_mem_arready(axi_mem_arready),
        .axi_mem_rdata(axi_mem_rdata),
//        .axi_mem_rid(axi_mem_rid),
        .axi_mem_rresp(axi_mem_rresp),
        .axi_mem_rlast(axi_mem_rlast),
        .axi_mem_rvalid(axi_mem_rvalid),
        .axi_mem_rready(axi_mem_rready),
        .address_low(ADDRESS_LOW),
        .address_high(ADDRESS_HIGH)
    );

    handler_wrapper #(
        .NUM_KERNELS(NUM_KERNELS),
        .ADDR_WIDTH(ADDR_WIDTH),
        .DATA_WIDTH(DATA_WIDTH),
        .STRB_WIDTH(STRB_WIDTH)
    ) handler_wrapper_i (
        .clock(clock),
        .reset_n(reset_n),
        `S_AXILITE_INSTANTIATE(0),
        `S_AXILITE_INSTANTIATE(1),
        `S_AXILITE_INSTANTIATE(2),
        `S_AXILITE_INSTANTIATE(3),
        `S_AXILITE_INSTANTIATE(4),
        `S_AXILITE_INSTANTIATE(5),
        `S_AXILITE_INSTANTIATE(6),
        `S_AXILITE_INSTANTIATE(7),
        `S_AXILITE_INSTANTIATE(8),
        `S_AXILITE_INSTANTIATE(9),
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
    
    blk_mem_gen_0 memory (
      .s_aclk(clock),                // input wire s_aclk
      .s_aresetn(reset_n),          // input wire s_aresetn
      .s_axi_awid('0),        // input wire [3 : 0] s_axi_awid
      .s_axi_awaddr(axi_mem_awaddr),    // input wire [31 : 0] s_axi_awaddr
      .s_axi_awlen(axi_mem_awlen),      // input wire [7 : 0] s_axi_awlen
      .s_axi_awsize(axi_mem_awsize),    // input wire [2 : 0] s_axi_awsize
      .s_axi_awburst(axi_mem_awburst),  // input wire [1 : 0] s_axi_awburst
      .s_axi_awvalid(axi_mem_awvalid),  // input wire s_axi_awvalid
      .s_axi_awready(axi_mem_awready),  // output wire s_axi_awready
      .s_axi_wdata(axi_mem_wdata),      // input wire [63 : 0] s_axi_wdata
      .s_axi_wstrb(axi_mem_wstrb),      // input wire [7 : 0] s_axi_wstrb
      .s_axi_wlast(axi_mem_wlast),      // input wire s_axi_wlast
      .s_axi_wvalid(axi_mem_wvalid),    // input wire s_axi_wvalid
      .s_axi_wready(axi_mem_wready),    // output wire s_axi_wready
      .s_axi_bid(),          // output wire [3 : 0] s_axi_bid
      .s_axi_bresp(axi_mem_bresp),      // output wire [1 : 0] s_axi_bresp
      .s_axi_bvalid(axi_mem_bvalid),    // output wire s_axi_bvalid
      .s_axi_bready(axi_mem_bready),    // input wire s_axi_bready
      .s_axi_arid('0),        // input wire [3 : 0] s_axi_arid
      .s_axi_araddr(axi_mem_araddr),    // input wire [31 : 0] s_axi_araddr
      .s_axi_arlen(axi_mem_arlen),      // input wire [7 : 0] s_axi_arlen
      .s_axi_arsize(axi_mem_arsize),    // input wire [2 : 0] s_axi_arsize
      .s_axi_arburst(axi_mem_arburst),  // input wire [1 : 0] s_axi_arburst
      .s_axi_arvalid(axi_mem_arvalid),  // input wire s_axi_arvalid
      .s_axi_arready(axi_mem_arready),  // output wire s_axi_arready
      .s_axi_rid(),          // output wire [3 : 0] s_axi_rid
      .s_axi_rdata(axi_mem_rdata),      // output wire [63 : 0] s_axi_rdata
      .s_axi_rresp(axi_mem_rresp),      // output wire [1 : 0] s_axi_rresp
      .s_axi_rlast(axi_mem_rlast),      // output wire s_axi_rlast
      .s_axi_rvalid(axi_mem_rvalid),    // output wire s_axi_rvalid
      .s_axi_rready(axi_mem_rready)    // input wire s_axi_rready
    );
    
endmodule