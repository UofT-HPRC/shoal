module datamover_mem(
    input [31:0] axi_read_araddr,
    input [1:0] axi_read_arburst,
    input [3:0] axi_read_arcache,
    input [7:0] axi_read_arlen,
    input [0:0] axi_read_arlock,
    input [2:0] axi_read_arprot,
    input [3:0] axi_read_arqos,
    output axi_read_arready,
    // input [3:0] axi_read_arregion,
    input [2:0] axi_read_arsize,
    input axi_read_arvalid,
    output [63:0] axi_read_rdata,
    output axi_read_rlast,
    input axi_read_rready,
    output [1:0] axi_read_rresp,
    output axi_read_rvalid,

    input [31:0] axi_write_awaddr,
    input [1:0] axi_write_awburst,
    input [3:0] axi_write_awcache,
    input [7:0] axi_write_awlen,
    input [0:0] axi_write_awlock,
    input [2:0] axi_write_awprot,
    input [3:0] axi_write_awqos,
    output axi_write_awready,
    // input [3:0] axi_write_awregion,
    input [2:0] axi_write_awsize,
    input axi_write_awvalid,
    input axi_write_bready,
    output [1:0] axi_write_bresp,
    output axi_write_bvalid,
    input [63:0] axi_write_wdata,
    input axi_write_wlast,
    output axi_write_wready,
    input [7:0] axi_write_wstrb,
    input axi_write_wvalid,

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
    output axi_remote_wvalid
);

    assign axi_remote_araddr = axi_read_araddr;
    assign axi_remote_arburst = axi_read_arburst;
    assign axi_remote_arcache = axi_read_arcache;
    assign axi_remote_arlen = axi_read_arlen;
    assign axi_remote_arlock = axi_read_arlock;
    assign axi_remote_arprot = axi_read_arprot;
    assign axi_remote_arqos = axi_read_arqos;
    assign axi_read_arready = axi_remote_arready;
    assign axi_remote_arsize = axi_read_arsize;
    assign axi_remote_arvalid = axi_read_arvalid;
    assign axi_read_rdata = axi_remote_rdata;
    assign axi_read_rlast = axi_remote_rlast;
    assign axi_remote_rready = axi_read_rready;
    assign axi_read_rresp = axi_remote_rresp;
    assign axi_read_rvalid = axi_remote_rvalid;

    assign axi_remote_awaddr = axi_write_awaddr;
    assign axi_remote_awburst = axi_write_awburst;
    assign axi_remote_awcache = axi_write_awcache;
    assign axi_remote_awlen = axi_write_awlen;
    assign axi_remote_awlock = axi_write_awlock;
    assign axi_remote_awprot = axi_write_awprot;
    assign axi_remote_awqos = axi_write_awqos;
    assign axi_write_awready = axi_remote_awready;
    assign axi_remote_awsize = axi_write_awsize;
    assign axi_remote_awvalid = axi_write_awvalid;
    assign axi_remote_bready = axi_write_bready;
    assign axi_write_bresp = axi_remote_bresp;
    assign axi_write_bvalid = axi_remote_bvalid;
    assign axi_remote_wdata = axi_write_wdata;
    assign axi_remote_wlast = axi_write_wlast;
    assign axi_write_wready = axi_remote_wready;
    assign axi_remote_wstrb = axi_write_wstrb;
    assign axi_remote_wvalid = axi_write_wvalid;

endmodule