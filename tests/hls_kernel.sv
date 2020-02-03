module hls_kernel_wrap(
    input ap_clk,
    input ap_rst_n,
    input [15:0] id_0,
    input [15:0] id_1
);

    wire [31:0] axi_instr_0_araddr;
    wire [1:0] axi_instr_0_arburst;
    wire [3:0] axi_instr_0_arcache;
    wire [0:0] axi_instr_0_arid;
    wire [7:0] axi_instr_0_arlen;
    wire [0:0] axi_instr_0_arlock;
    wire [2:0] axi_instr_0_arprot;
    wire [3:0] axi_instr_0_arqos;
    wire [0:0] axi_instr_0_arready;
    wire [3:0] axi_instr_0_arregion;
    wire [2:0] axi_instr_0_arsize;
    wire [0:0] axi_instr_0_arvalid;
    wire [31:0] axi_instr_0_awaddr;
    wire [1:0] axi_instr_0_awburst;
    wire [3:0] axi_instr_0_awcache;
    wire [0:0] axi_instr_0_awid;
    wire [7:0] axi_instr_0_awlen;
    wire [0:0] axi_instr_0_awlock;
    wire [2:0] axi_instr_0_awprot;
    wire [3:0] axi_instr_0_awqos;
    wire [0:0] axi_instr_0_awready;
    wire [3:0] axi_instr_0_awregion;
    wire [2:0] axi_instr_0_awsize;
    wire [0:0] axi_instr_0_awvalid;
    wire [0:0] axi_instr_0_bid;
    wire [0:0] axi_instr_0_bready;
    wire [1:0] axi_instr_0_bresp;
    wire [0:0] axi_instr_0_bvalid;
    wire [63:0] axi_instr_0_rdata;
    wire [0:0] axi_instr_0_rid;
    wire [0:0] axi_instr_0_rlast;
    wire [0:0] axi_instr_0_rready;
    wire [1:0] axi_instr_0_rresp;
    wire [0:0] axi_instr_0_rvalid;
    wire [63:0] axi_instr_0_wdata;
    wire [0:0] axi_instr_0_wlast;
    wire [0:0] axi_instr_0_wready;
    wire [7:0] axi_instr_0_wstrb;
    wire [0:0] axi_instr_0_wvalid;
    wire [31:0] axi_instr_1_araddr;
    wire [1:0] axi_instr_1_arburst;
    wire [3:0] axi_instr_1_arcache;
    wire [0:0] axi_instr_1_arid;
    wire [7:0] axi_instr_1_arlen;
    wire [0:0] axi_instr_1_arlock;
    wire [2:0] axi_instr_1_arprot;
    wire [3:0] axi_instr_1_arqos;
    wire [0:0] axi_instr_1_arready;
    wire [3:0] axi_instr_1_arregion;
    wire [2:0] axi_instr_1_arsize;
    wire [0:0] axi_instr_1_arvalid;
    wire [31:0] axi_instr_1_awaddr;
    wire [1:0] axi_instr_1_awburst;
    wire [3:0] axi_instr_1_awcache;
    wire [0:0] axi_instr_1_awid;
    wire [7:0] axi_instr_1_awlen;
    wire [0:0] axi_instr_1_awlock;
    wire [2:0] axi_instr_1_awprot;
    wire [3:0] axi_instr_1_awqos;
    wire [0:0] axi_instr_1_awready;
    wire [3:0] axi_instr_1_awregion;
    wire [2:0] axi_instr_1_awsize;
    wire [0:0] axi_instr_1_awvalid;
    wire [0:0] axi_instr_1_bid;
    wire [0:0] axi_instr_1_bready;
    wire [1:0] axi_instr_1_bresp;
    wire [0:0] axi_instr_1_bvalid;
    wire [63:0] axi_instr_1_rdata;
    wire [0:0] axi_instr_1_rid;
    wire [0:0] axi_instr_1_rlast;
    wire [0:0] axi_instr_1_rready;
    wire [1:0] axi_instr_1_rresp;
    wire [0:0] axi_instr_1_rvalid;
    wire [63:0] axi_instr_1_wdata;
    wire [0:0] axi_instr_1_wlast;
    wire [0:0] axi_instr_1_wready;
    wire [7:0] axi_instr_1_wstrb;
    wire [0:0] axi_instr_1_wvalid;
    wire [31:0] axi_mem_0_araddr;
    wire [1:0] axi_mem_0_arburst;
    wire [3:0] axi_mem_0_arcache;
    wire [7:0] axi_mem_0_arlen;
    wire [0:0] axi_mem_0_arlock;
    wire [2:0] axi_mem_0_arprot;
    wire [3:0] axi_mem_0_arqos;
    wire  axi_mem_0_arready;
    wire [3:0] axi_mem_0_arregion;
    wire [2:0] axi_mem_0_arsize;
    wire  axi_mem_0_arvalid;
    wire [31:0] axi_mem_0_awaddr;
    wire [1:0] axi_mem_0_awburst;
    wire [3:0] axi_mem_0_awcache;
    wire [7:0] axi_mem_0_awlen;
    wire [0:0] axi_mem_0_awlock;
    wire [2:0] axi_mem_0_awprot;
    wire [3:0] axi_mem_0_awqos;
    wire  axi_mem_0_awready;
    wire [3:0] axi_mem_0_awregion;
    wire [2:0] axi_mem_0_awsize;
    wire  axi_mem_0_awvalid;
    wire  axi_mem_0_bready;
    wire [1:0] axi_mem_0_bresp;
    wire  axi_mem_0_bvalid;
    wire [31:0] axi_mem_0_rdata;
    wire  axi_mem_0_rlast;
    wire  axi_mem_0_rready;
    wire [1:0] axi_mem_0_rresp;
    wire  axi_mem_0_rvalid;
    wire [31:0] axi_mem_0_wdata;
    wire  axi_mem_0_wlast;
    wire  axi_mem_0_wready;
    wire [3:0] axi_mem_0_wstrb;
    wire  axi_mem_0_wvalid;
    wire [31:0] axi_mem_1_araddr;
    wire [1:0] axi_mem_1_arburst;
    wire [3:0] axi_mem_1_arcache;
    wire [7:0] axi_mem_1_arlen;
    wire [0:0] axi_mem_1_arlock;
    wire [2:0] axi_mem_1_arprot;
    wire [3:0] axi_mem_1_arqos;
    wire  axi_mem_1_arready;
    wire [3:0] axi_mem_1_arregion;
    wire [2:0] axi_mem_1_arsize;
    wire  axi_mem_1_arvalid;
    wire [31:0] axi_mem_1_awaddr;
    wire [1:0] axi_mem_1_awburst;
    wire [3:0] axi_mem_1_awcache;
    wire [7:0] axi_mem_1_awlen;
    wire [0:0] axi_mem_1_awlock;
    wire [2:0] axi_mem_1_awprot;
    wire [3:0] axi_mem_1_awqos;
    wire  axi_mem_1_awready;
    wire [3:0] axi_mem_1_awregion;
    wire [2:0] axi_mem_1_awsize;
    wire  axi_mem_1_awvalid;
    wire  axi_mem_1_bready;
    wire [1:0] axi_mem_1_bresp;
    wire  axi_mem_1_bvalid;
    wire [31:0] axi_mem_1_rdata;
    wire  axi_mem_1_rlast;
    wire  axi_mem_1_rready;
    wire [1:0] axi_mem_1_rresp;
    wire  axi_mem_1_rvalid;
    wire [31:0] axi_mem_1_wdata;
    wire  axi_mem_1_wlast;
    wire  axi_mem_1_wready;
    wire [3:0] axi_mem_1_wstrb;
    wire  axi_mem_1_wvalid;

    hls_kernel_bd_wrapper hls_kernel_bd_wrapper_i(
        axi_instr_0_araddr,
        axi_instr_0_arburst,
        axi_instr_0_arcache,
        // axi_instr_0_arid,
        axi_instr_0_arlen,
        axi_instr_0_arlock,
        axi_instr_0_arprot,
        axi_instr_0_arqos,
        axi_instr_0_arready,
        axi_instr_0_arregion,
        axi_instr_0_arsize,
        axi_instr_0_arvalid,
        axi_instr_0_awaddr,
        axi_instr_0_awburst,
        axi_instr_0_awcache,
        // axi_instr_0_awid,
        axi_instr_0_awlen,
        axi_instr_0_awlock,
        axi_instr_0_awprot,
        axi_instr_0_awqos,
        axi_instr_0_awready,
        axi_instr_0_awregion,
        axi_instr_0_awsize,
        axi_instr_0_awvalid,
        // axi_instr_0_bid,
        axi_instr_0_bready,
        axi_instr_0_bresp,
        axi_instr_0_bvalid,
        axi_instr_0_rdata,
        // axi_instr_0_rid,
        axi_instr_0_rlast,
        axi_instr_0_rready,
        axi_instr_0_rresp,
        axi_instr_0_rvalid,
        axi_instr_0_wdata,
        axi_instr_0_wlast,
        axi_instr_0_wready,
        axi_instr_0_wstrb,
        axi_instr_0_wvalid,
        axi_instr_1_araddr,
        axi_instr_1_arburst,
        axi_instr_1_arcache,
        // axi_instr_1_arid,
        axi_instr_1_arlen,
        axi_instr_1_arlock,
        axi_instr_1_arprot,
        axi_instr_1_arqos,
        axi_instr_1_arready,
        axi_instr_1_arregion,
        axi_instr_1_arsize,
        axi_instr_1_arvalid,
        axi_instr_1_awaddr,
        axi_instr_1_awburst,
        axi_instr_1_awcache,
        // axi_instr_1_awid,
        axi_instr_1_awlen,
        axi_instr_1_awlock,
        axi_instr_1_awprot,
        axi_instr_1_awqos,
        axi_instr_1_awready,
        axi_instr_1_awregion,
        axi_instr_1_awsize,
        axi_instr_1_awvalid,
        // axi_instr_1_bid,
        axi_instr_1_bready,
        axi_instr_1_bresp,
        axi_instr_1_bvalid,
        axi_instr_1_rdata,
        // axi_instr_1_rid,
        axi_instr_1_rlast,
        axi_instr_1_rready,
        axi_instr_1_rresp,
        axi_instr_1_rvalid,
        axi_instr_1_wdata,
        axi_instr_1_wlast,
        axi_instr_1_wready,
        axi_instr_1_wstrb,
        axi_instr_1_wvalid,
        ap_clk,
        id_0,
        id_1,
        ap_rst_n
    );

    blk_mem_gen_0 memory_0 (
      .s_aclk(ap_clk),                // input wire s_aclk
      .s_aresetn(ap_rst_n),          // input wire s_aresetn
      .s_axi_awid(axi_instr_0_awid),        // input wire [3 : 0] s_axi_awid
      .s_axi_awaddr(axi_instr_0_awaddr),    // input wire [31 : 0] s_axi_awaddr
      .s_axi_awlen(axi_instr_0_awlen),      // input wire [7 : 0] s_axi_awlen
      .s_axi_awsize(axi_instr_0_awsize),    // input wire [2 : 0] s_axi_awsize
      .s_axi_awburst(axi_instr_0_awburst),  // input wire [1 : 0] s_axi_awburst
      .s_axi_awvalid(axi_instr_0_awvalid),  // input wire s_axi_awvalid
      .s_axi_awready(axi_instr_0_awready),  // output wire s_axi_awready
      .s_axi_wdata(axi_instr_0_wdata),      // input wire [63 : 0] s_axi_wdata
      .s_axi_wstrb(axi_instr_0_wstrb),      // input wire [7 : 0] s_axi_wstrb
      .s_axi_wlast(axi_instr_0_wlast),      // input wire s_axi_wlast
      .s_axi_wvalid(axi_instr_0_wvalid),    // input wire s_axi_wvalid
      .s_axi_wready(axi_instr_0_wready),    // output wire s_axi_wready
      .s_axi_bid(axi_instr_0_bid),          // output wire [3 : 0] s_axi_bid
      .s_axi_bresp(axi_instr_0_bresp),      // output wire [1 : 0] s_axi_bresp
      .s_axi_bvalid(axi_instr_0_bvalid),    // output wire s_axi_bvalid
      .s_axi_bready(axi_instr_0_bready),    // input wire s_axi_bready
      .s_axi_arid(axi_instr_0_arid),        // input wire [3 : 0] s_axi_arid
      .s_axi_araddr(axi_instr_0_araddr),    // input wire [31 : 0] s_axi_araddr
      .s_axi_arlen(axi_instr_0_arlen),      // input wire [7 : 0] s_axi_arlen
      .s_axi_arsize(axi_instr_0_arsize),    // input wire [2 : 0] s_axi_arsize
      .s_axi_arburst(axi_instr_0_arburst),  // input wire [1 : 0] s_axi_arburst
      .s_axi_arvalid(axi_instr_0_arvalid),  // input wire s_axi_arvalid
      .s_axi_arready(axi_instr_0_arready),  // output wire s_axi_arready
      .s_axi_rid(axi_instr_0_rid),          // output wire [3 : 0] s_axi_rid
      .s_axi_rdata(axi_instr_0_rdata),      // output wire [63 : 0] s_axi_rdata
      .s_axi_rresp(axi_instr_0_rresp),      // output wire [1 : 0] s_axi_rresp
      .s_axi_rlast(axi_instr_0_rlast),      // output wire s_axi_rlast
      .s_axi_rvalid(axi_instr_0_rvalid),    // output wire s_axi_rvalid
      .s_axi_rready(axi_instr_0_rready)    // input wire s_axi_rready
    );

    blk_mem_gen_1 memory_1 (
      .s_aclk(ap_clk),                // input wire s_aclk
      .s_aresetn(ap_rst_n),          // input wire s_aresetn
      .s_axi_awid(axi_instr_1_awid),        // input wire [3 : 0] s_axi_awid
      .s_axi_awaddr(axi_instr_1_awaddr),    // input wire [31 : 0] s_axi_awaddr
      .s_axi_awlen(axi_instr_1_awlen),      // input wire [7 : 0] s_axi_awlen
      .s_axi_awsize(axi_instr_1_awsize),    // input wire [2 : 0] s_axi_awsize
      .s_axi_awburst(axi_instr_1_awburst),  // input wire [1 : 0] s_axi_awburst
      .s_axi_awvalid(axi_instr_1_awvalid),  // input wire s_axi_awvalid
      .s_axi_awready(axi_instr_1_awready),  // output wire s_axi_awready
      .s_axi_wdata(axi_instr_1_wdata),      // input wire [63 : 0] s_axi_wdata
      .s_axi_wstrb(axi_instr_1_wstrb),      // input wire [7 : 0] s_axi_wstrb
      .s_axi_wlast(axi_instr_1_wlast),      // input wire s_axi_wlast
      .s_axi_wvalid(axi_instr_1_wvalid),    // input wire s_axi_wvalid
      .s_axi_wready(axi_instr_1_wready),    // output wire s_axi_wready
      .s_axi_bid(axi_instr_1_bid),          // output wire [3 : 0] s_axi_bid
      .s_axi_bresp(axi_instr_1_bresp),      // output wire [1 : 0] s_axi_bresp
      .s_axi_bvalid(axi_instr_1_bvalid),    // output wire s_axi_bvalid
      .s_axi_bready(axi_instr_1_bready),    // input wire s_axi_bready
      .s_axi_arid(axi_instr_1_arid),        // input wire [3 : 0] s_axi_arid
      .s_axi_araddr(axi_instr_1_araddr),    // input wire [31 : 0] s_axi_araddr
      .s_axi_arlen(axi_instr_1_arlen),      // input wire [7 : 0] s_axi_arlen
      .s_axi_arsize(axi_instr_1_arsize),    // input wire [2 : 0] s_axi_arsize
      .s_axi_arburst(axi_instr_1_arburst),  // input wire [1 : 0] s_axi_arburst
      .s_axi_arvalid(axi_instr_1_arvalid),  // input wire s_axi_arvalid
      .s_axi_arready(axi_instr_1_arready),  // output wire s_axi_arready
      .s_axi_rid(axi_instr_1_rid),          // output wire [3 : 0] s_axi_rid
      .s_axi_rdata(axi_instr_1_rdata),      // output wire [63 : 0] s_axi_rdata
      .s_axi_rresp(axi_instr_1_rresp),      // output wire [1 : 0] s_axi_rresp
      .s_axi_rlast(axi_instr_1_rlast),      // output wire s_axi_rlast
      .s_axi_rvalid(axi_instr_1_rvalid),    // output wire s_axi_rvalid
      .s_axi_rready(axi_instr_1_rready)    // input wire s_axi_rready
    );
    
endmodule
