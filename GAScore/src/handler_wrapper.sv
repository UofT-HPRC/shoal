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

`define INTERRUPT_PORTS(n) \
    output logic interrupt_``n``

`define ASSIGN_S_AXILITE(n) \
    s_axi_ctrl_bus_AWVALID[``n``] = s_axi_ctrl_bus_``n``_AWVALID; \
    s_axi_ctrl_bus_``n``_AWREADY = s_axi_ctrl_bus_AWREADY[``n``]; \
    s_axi_ctrl_bus_AWADDR[``n``] = s_axi_ctrl_bus_``n``_AWADDR; \
    s_axi_ctrl_bus_WVALID[``n``] = s_axi_ctrl_bus_``n``_WVALID; \
    s_axi_ctrl_bus_``n``_WREADY = s_axi_ctrl_bus_WREADY[``n``]; \
    s_axi_ctrl_bus_WDATA[``n``] = s_axi_ctrl_bus_``n``_WDATA; \
    s_axi_ctrl_bus_WSTRB[``n``] = s_axi_ctrl_bus_``n``_WSTRB; \
    s_axi_ctrl_bus_ARVALID[``n``] = s_axi_ctrl_bus_``n``_ARVALID; \
    s_axi_ctrl_bus_``n``_ARREADY = s_axi_ctrl_bus_ARREADY[``n``]; \
    s_axi_ctrl_bus_ARADDR[``n``] = s_axi_ctrl_bus_``n``_ARADDR; \
    s_axi_ctrl_bus_``n``_RVALID = s_axi_ctrl_bus_RVALID[``n``]; \
    s_axi_ctrl_bus_RREADY[``n``] = s_axi_ctrl_bus_``n``_RREADY; \
    s_axi_ctrl_bus_``n``_RDATA = s_axi_ctrl_bus_RDATA[``n``]; \
    s_axi_ctrl_bus_``n``_RRESP = s_axi_ctrl_bus_RRESP[``n``]; \
    s_axi_ctrl_bus_``n``_BVALID = s_axi_ctrl_bus_BVALID[``n``]; \
    s_axi_ctrl_bus_BREADY[``n``] = s_axi_ctrl_bus_``n``_BREADY; \
    s_axi_ctrl_bus_``n``_BRESP = s_axi_ctrl_bus_BRESP[``n``];

`define ASSIGN_INTERRUPT(n) \
    interrupt_``n`` = interrupt_V[``n``];

module handler_wrapper #(
    parameter NUM_KERNELS = 2,

    // these should match the values in handler.v from HLS
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 32,
    parameter STRB_WIDTH = DATA_WIDTH/8
)(
    input clock,
    input reset_n,

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

    input [63:0] axis_handler_tdata,
    input axis_handler_tlast,
    output axis_handler_tready,
    input axis_handler_tvalid,

    input [15:0] address_offset,

    `INTERRUPT_PORTS(0),
    `INTERRUPT_PORTS(1),
    `INTERRUPT_PORTS(2),
    `INTERRUPT_PORTS(3),
    `INTERRUPT_PORTS(4),
    `INTERRUPT_PORTS(5),
    `INTERRUPT_PORTS(6),
    `INTERRUPT_PORTS(7),
    `INTERRUPT_PORTS(8),
    `INTERRUPT_PORTS(9),
    `INTERRUPT_PORTS(10),
    `INTERRUPT_PORTS(11),
    `INTERRUPT_PORTS(12),
    `INTERRUPT_PORTS(13),
    `INTERRUPT_PORTS(14),
    `INTERRUPT_PORTS(15)
);
    localparam KERNEL_WIDTH = $clog2(NUM_KERNELS);
    logic [KERNEL_WIDTH-1:0] address;
    logic [3:0] AMhandler;

    enum bit {st_header, st_payload} currentState, nextState;

    always @(*) begin
        case(currentState)
            st_header: begin
                nextState = axis_handler_tvalid & axis_handler_tready ? st_payload : st_header;
            end
            st_payload: begin
                if(axis_handler_tlast & axis_handler_tvalid) begin
                    nextState = st_header;
                end
                else begin
                    nextState = st_payload;
                end
            end
            default: begin

            end
        endcase
    end

    wire [15:0] address_wire = axis_handler_tdata[39:24] - address_offset;

    always @(posedge clock) begin
        if(!reset_n) begin
            address <= '0;
            AMhandler <= '0;
            currentState <= st_header;
        end
        else begin
            currentState <= nextState;
            if (currentState == st_header) begin
                // TODO use define for bit indices
                address <= address_wire[KERNEL_WIDTH-1:0];
                AMhandler <= axis_handler_tdata[55:52];
            end
        end
    end

    logic s_axi_ctrl_bus_AWVALID [NUM_KERNELS];
    logic s_axi_ctrl_bus_AWREADY [NUM_KERNELS];
    logic [ADDR_WIDTH - 1:0] s_axi_ctrl_bus_AWADDR [NUM_KERNELS];
    logic s_axi_ctrl_bus_WVALID [NUM_KERNELS];
    logic s_axi_ctrl_bus_WREADY [NUM_KERNELS];
    logic [DATA_WIDTH - 1:0] s_axi_ctrl_bus_WDATA [NUM_KERNELS];
    logic [STRB_WIDTH - 1:0] s_axi_ctrl_bus_WSTRB [NUM_KERNELS];
    logic s_axi_ctrl_bus_ARVALID [NUM_KERNELS];
    logic s_axi_ctrl_bus_ARREADY [NUM_KERNELS];
    logic [ADDR_WIDTH - 1:0] s_axi_ctrl_bus_ARADDR [NUM_KERNELS];
    logic s_axi_ctrl_bus_RVALID [NUM_KERNELS];
    logic s_axi_ctrl_bus_RREADY [NUM_KERNELS];
    logic [DATA_WIDTH - 1:0] s_axi_ctrl_bus_RDATA [NUM_KERNELS];
    logic [1:0] s_axi_ctrl_bus_RRESP [NUM_KERNELS];
    logic s_axi_ctrl_bus_BVALID [NUM_KERNELS];
    logic s_axi_ctrl_bus_BREADY [NUM_KERNELS];
    logic [1:0] s_axi_ctrl_bus_BRESP [NUM_KERNELS];
    logic interrupt_V [NUM_KERNELS];

    logic [NUM_KERNELS-1:0] tready;

    always @(*) begin
        if (NUM_KERNELS > 15) begin
            `ASSIGN_S_AXILITE(15)
        end
        if (NUM_KERNELS > 14) begin
            `ASSIGN_S_AXILITE(14)
        end
        if (NUM_KERNELS > 13) begin
            `ASSIGN_S_AXILITE(13)
        end
        if (NUM_KERNELS > 12) begin
            `ASSIGN_S_AXILITE(12)
        end
        if (NUM_KERNELS > 11) begin
            `ASSIGN_S_AXILITE(11)
        end
        if (NUM_KERNELS > 10) begin
            `ASSIGN_S_AXILITE(10)
        end
        if (NUM_KERNELS > 9) begin
            `ASSIGN_S_AXILITE(9)
        end
        if (NUM_KERNELS > 8) begin
            `ASSIGN_S_AXILITE(8)
        end
        if (NUM_KERNELS > 7) begin
            `ASSIGN_S_AXILITE(7)
        end
        if (NUM_KERNELS > 6) begin
            `ASSIGN_S_AXILITE(6)
        end
        if (NUM_KERNELS > 5) begin
            `ASSIGN_S_AXILITE(5)
        end
        if (NUM_KERNELS > 4) begin
            `ASSIGN_S_AXILITE(4)
        end
        if (NUM_KERNELS > 3) begin
            `ASSIGN_S_AXILITE(3)
        end
        if (NUM_KERNELS > 2) begin
            `ASSIGN_S_AXILITE(2)
        end
        if (NUM_KERNELS > 1) begin
            `ASSIGN_S_AXILITE(1)
        end
        if (NUM_KERNELS > 0) begin
            `ASSIGN_S_AXILITE(0)
        end
    end

    genvar i;
    generate
        for (i = 0; i < NUM_KERNELS; i++) begin
            handler handler_inst(
                .s_axi_ctrl_bus_AWVALID(s_axi_ctrl_bus_AWVALID[i]),
                .s_axi_ctrl_bus_AWREADY(s_axi_ctrl_bus_AWREADY[i]),
                .s_axi_ctrl_bus_AWADDR(s_axi_ctrl_bus_AWADDR[i]),
                .s_axi_ctrl_bus_WVALID(s_axi_ctrl_bus_WVALID[i]),
                .s_axi_ctrl_bus_WREADY(s_axi_ctrl_bus_WREADY[i]),
                .s_axi_ctrl_bus_WDATA(s_axi_ctrl_bus_WDATA[i]),
                .s_axi_ctrl_bus_WSTRB(s_axi_ctrl_bus_WSTRB[i]),
                .s_axi_ctrl_bus_ARVALID(s_axi_ctrl_bus_ARVALID[i]),
                .s_axi_ctrl_bus_ARREADY(s_axi_ctrl_bus_ARREADY[i]),
                .s_axi_ctrl_bus_ARADDR(s_axi_ctrl_bus_ARADDR[i]),
                .s_axi_ctrl_bus_RVALID(s_axi_ctrl_bus_RVALID[i]),
                .s_axi_ctrl_bus_RREADY(s_axi_ctrl_bus_RREADY[i]),
                .s_axi_ctrl_bus_RDATA(s_axi_ctrl_bus_RDATA[i]),
                .s_axi_ctrl_bus_RRESP(s_axi_ctrl_bus_RRESP[i]),
                .s_axi_ctrl_bus_BVALID(s_axi_ctrl_bus_BVALID[i]),
                .s_axi_ctrl_bus_BREADY(s_axi_ctrl_bus_BREADY[i]),
                .s_axi_ctrl_bus_BRESP(s_axi_ctrl_bus_BRESP[i]),
                .ap_clk(clock),
                .ap_rst_n(reset_n),
                .AMhandler_V(AMhandler),
                .axis_handler_TDATA(axis_handler_tdata),
                .axis_handler_TLAST(axis_handler_tlast),
                .interrupt_V(interrupt_V[i]),
                .axis_handler_TVALID(axis_handler_tvalid & address == i & currentState == st_payload),
                .axis_handler_TREADY(tready[i])
            );
        end
    endgenerate

    always @(*) begin
        if (NUM_KERNELS > 15) begin
            `ASSIGN_INTERRUPT(15)
        end
        if (NUM_KERNELS > 14) begin
            `ASSIGN_INTERRUPT(14)
        end
        if (NUM_KERNELS > 13) begin
            `ASSIGN_INTERRUPT(13)
        end
        if (NUM_KERNELS > 12) begin
            `ASSIGN_INTERRUPT(12)
        end
        if (NUM_KERNELS > 11) begin
            `ASSIGN_INTERRUPT(11)
        end
        if (NUM_KERNELS > 10) begin
            `ASSIGN_INTERRUPT(10)
        end
        if (NUM_KERNELS > 9) begin
            `ASSIGN_INTERRUPT(9)
        end
        if (NUM_KERNELS > 8) begin
            `ASSIGN_INTERRUPT(8)
        end
        if (NUM_KERNELS > 7) begin
            `ASSIGN_INTERRUPT(7)
        end
        if (NUM_KERNELS > 6) begin
            `ASSIGN_INTERRUPT(6)
        end
        if (NUM_KERNELS > 5) begin
            `ASSIGN_INTERRUPT(5)
        end
        if (NUM_KERNELS > 4) begin
            `ASSIGN_INTERRUPT(4)
        end
        if (NUM_KERNELS > 3) begin
            `ASSIGN_INTERRUPT(3)
        end
        if (NUM_KERNELS > 2) begin
            `ASSIGN_INTERRUPT(2)
        end
        if (NUM_KERNELS > 1) begin
            `ASSIGN_INTERRUPT(1)
        end
        if (NUM_KERNELS > 0) begin
            `ASSIGN_INTERRUPT(0)
        end
    end

    assign axis_handler_tready = tready[address];

endmodule