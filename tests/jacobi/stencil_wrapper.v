module stencil_wrapper (
    input clk,
    input reset_n,

    input [31:0] from_kernel_tdata,
    input [7:0] from_kernel_tuser,
    input [3:0] from_kernel_tkeep,
    input [7:0] from_kernel_tid,
    input [7:0] from_kernel_tdest,
    input from_kernel_tvalid,
    output from_kernel_tready,
    input from_kernel_tlast,

    input [7:0] from_kernel_ctrl_tdata,
    input [7:0] from_kernel_ctrl_tuser,
    input [0:0] from_kernel_ctrl_tkeep,
    input [7:0] from_kernel_ctrl_tid,
    input [7:0] from_kernel_ctrl_tdest,
    input from_kernel_ctrl_tvalid,
    output from_kernel_ctrl_tready,
    input from_kernel_ctrl_tlast,

    output [31:0] to_kernel_tdata,
    output [7:0] to_kernel_tuser,
    output [3:0] to_kernel_tkeep,
    output [7:0] to_kernel_tid,
    output [7:0] to_kernel_tdest,
    output to_kernel_tvalid,
    input to_kernel_tready,
    output to_kernel_tlast,

    output [7:0] to_kernel_ctrl_tdata,
    output [7:0] to_kernel_ctrl_tuser,
    output [0:0] to_kernel_ctrl_tkeep,
    output [7:0] to_kernel_ctrl_tid,
    output [7:0] to_kernel_ctrl_tdest,
    output to_kernel_ctrl_tvalid,
    input to_kernel_ctrl_tready,
    output to_kernel_ctrl_tlast,

    input m_axi_aclk,
    input m_axi_aresetn,
    input m_axi_awready,
    output m_axi_awvalid,
    output [31:0] m_axi_awaddr,
    output [7:0] m_axi_awlen,
    output [2:0] m_axi_awsize,
    output [1:0] m_axi_awburst,
    output [2:0] m_axi_awprot,
    output [3:0] m_axi_awcache,
    input m_axi_wready,
    output m_axi_wvalid,
    output [31:0] m_axi_wdata,
    output [3:0] m_axi_wstrb,
    output m_axi_wlast,
    output m_axi_bready,
    input m_axi_bvalid,
    input [1:0] m_axi_bresp,
    input m_axi_arready,
    output m_axi_arvalid,
    output [31:0] m_axi_araddr,
    output [7:0] m_axi_arlen,
    output [2:0] m_axi_arsize,
    output [1:0] m_axi_arburst,
    output [2:0] m_axi_arprot,
    output [3:0] m_axi_arcache,
    output m_axi_rready,
    input m_axi_rvalid,
    input [31:0] m_axi_rdata,
    input [1:0] m_axi_rresp,
    input m_axi_rlast,

    output dbg_data_to_pams_read,
    output dbg_data_to_pams_valid,
    output [31:0] dbg_data_to_pams,
    output dbg_data_from_pams_read,
    output dbg_data_from_pams_valid,
    output [31:0] dbg_data_from_pams,
    output [3:0] dbg_pams_ctrl_inputs,
    output [3:0] dbg_pams_ctrl_inputs_ack,
    output [15:0] dbg_pams_ctrl_outputs,
    output [15:0] dbg_pams_ctrl_outputs_ack,
    output [3:0] dbg_currentstate
);

wire reset = ~reset_n;

localparam state_reset = 0;
localparam state_wait_for_nodedata = 1;
localparam state_wait_for_clear = 2;
localparam state_wait_to_ack_clear = 3;
localparam state_ack_clear = 4;
localparam state_resp_clear = 5;
localparam state_wait_to_start = 6;
localparam state_start = 7;
localparam state_wait_for_computation = 8;
localparam state_wait_to_ack_done = 9;
localparam state_ack_done = 10;
localparam state_resp_done = 11;
localparam state_read_stats = 12;

reg [3:0] currentstate, nextstate;

wire [15:0] pams_ctrl_outputs_ack;
wire [3:0] pams_ctrl_inputs;

reg [1:0] stats_counter;

always @(*) begin
    case (currentstate)
        state_reset: begin
            if(reset) begin
                nextstate = state_reset;
            end else begin
                nextstate = state_wait_for_nodedata;
            end
        end
        state_wait_for_nodedata: begin
            if(from_kernel_tvalid & from_kernel_tready & from_kernel_tlast) begin
                nextstate = state_wait_for_clear;
            end else begin
                nextstate = state_wait_for_nodedata;
            end
        end
        state_wait_for_clear: begin
            if(&pams_ctrl_inputs[1:0]) begin
                nextstate = state_wait_to_ack_clear;
            end else begin
                nextstate = state_wait_for_clear;
            end
        end
        state_wait_to_ack_clear: begin
            if(from_kernel_ctrl_tready & from_kernel_ctrl_tvalid) begin
                nextstate = state_ack_clear;
            end else begin
                nextstate = state_wait_to_ack_clear;
            end
        end
        state_ack_clear: begin
            nextstate = state_resp_clear;
        end
        state_resp_clear: begin
            if(to_kernel_ctrl_tready & to_kernel_ctrl_tvalid) begin
                nextstate = state_wait_to_start;
            end else begin
                nextstate = state_resp_clear;
            end
        end
        state_wait_to_start: begin
            if(from_kernel_ctrl_tready & from_kernel_ctrl_tvalid) begin
                nextstate = state_start;
            end else begin
                nextstate = state_wait_to_start;
            end
        end
        state_start: begin
            nextstate = state_wait_for_computation;
        end
        state_wait_for_computation: begin
            if(pams_ctrl_inputs[0]) begin
                nextstate = state_wait_to_ack_done;
            end else begin
                nextstate = state_wait_for_computation;
            end
        end
        state_wait_to_ack_done: begin
            if(from_kernel_ctrl_tready & from_kernel_ctrl_tvalid) begin
                nextstate = state_ack_done;
            end else begin
                nextstate = state_wait_to_ack_done;
            end
        end
        state_ack_done: begin
            nextstate = state_resp_done;
        end
        state_resp_done: begin
            if(to_kernel_ctrl_tready & to_kernel_ctrl_tvalid) begin
                nextstate = state_read_stats;
            end else begin
                nextstate = state_resp_done;
            end
        end
        state_read_stats: begin
            if(stats_counter == 3) begin
                nextstate = state_wait_to_start;
            end else begin
                nextstate = state_read_stats;
            end
        end
        // after this, read the stats and then wait to restart the next iteration
        default: begin
            nextstate = state_reset;
        end
    endcase
end

wire [15:0] pams_ctrl_outputs;
wire [3:0] pams_ctrl_inputs_ack;
always @(posedge clk) begin
    if (reset) begin
        currentstate <= state_reset;
//        pams_ctrl_outputs <= 16'b0;
        stats_counter <= 2'b0;
    end else begin
        currentstate <= nextstate;
        if(currentstate == state_read_stats) begin
            if(to_kernel_tvalid & to_kernel_tready) begin
                stats_counter <= stats_counter + 1;
            end 
        end else begin
            stats_counter <= 2'b0;
        end
        // if(from_kernel_ctrl_tready & from_kernel_ctrl_tvalid) begin
        //     pams_ctrl_outputs <= {12'b0, from_kernel_ctrl_tdata[3:0]};
        // end else if(currentstate_from == state_from_ack & pams_ctrl_outputs_ack[0]) begin
        //     pams_ctrl_outputs <= 16'b0;
        // end
    end
end
assign pams_ctrl_outputs = currentstate == state_start ? 16'b1 : 16'b0;

assign from_kernel_ctrl_tready = 
    (currentstate == state_wait_to_ack_clear) |
    (currentstate == state_wait_to_start) |
    (currentstate == state_wait_to_ack_done);

assign to_kernel_ctrl_tvalid = 
    (currentstate == state_resp_clear) |
    (currentstate == state_resp_done);

assign to_kernel_ctrl_tdata = {4'b0, pams_ctrl_inputs};
assign to_kernel_ctrl_tlast = 1'b0;
assign to_kernel_ctrl_tid = 8'b0;
assign to_kernel_ctrl_tdest = 8'b0;
assign to_kernel_ctrl_tuser = 8'b0;
assign to_kernel_ctrl_tkeep = 8'hFF;
assign pams_ctrl_inputs_ack = currentstate == state_ack_clear ? 4'd3 : 
    (currentstate == state_ack_done ? 4'b1 : 4'b0);

wire data_to_pams_read = currentstate == state_read_stats & stats_counter < 3 ? to_kernel_tready : 1'b0;
wire data_to_pams_valid;

stencil_core stencil_core_i(
    .clock(clk),
    .reset(reset),
    .m_axi_aclk(m_axi_aclk),
    .m_axi_aresetn(m_axi_aresetn),
    .m_axi_awready(m_axi_awready),
    .m_axi_awvalid(m_axi_awvalid),
    .m_axi_awaddr(m_axi_awaddr),
    .m_axi_awlen(m_axi_awlen),
    .m_axi_awsize(m_axi_awsize),
    .m_axi_awburst(m_axi_awburst),
    .m_axi_awprot(m_axi_awprot),
    .m_axi_awcache(m_axi_awcache),
    .m_axi_wready(m_axi_wready),
    .m_axi_wvalid(m_axi_wvalid),
    .m_axi_wdata(m_axi_wdata),
    .m_axi_wstrb(m_axi_wstrb),
    .m_axi_wlast(m_axi_wlast),
    .m_axi_bready(m_axi_bready),
    .m_axi_bvalid(m_axi_bvalid),
    .m_axi_bresp(m_axi_bresp),
    .m_axi_arready(m_axi_arready),
    .m_axi_arvalid(m_axi_arvalid),
    .m_axi_araddr(m_axi_araddr),
    .m_axi_arlen(m_axi_arlen),
    .m_axi_arsize(m_axi_arsize),
    .m_axi_arburst(m_axi_arburst),
    .m_axi_arprot(m_axi_arprot),
    .m_axi_arcache(m_axi_arcache),
    .m_axi_rready(m_axi_rready),
    .m_axi_rvalid(m_axi_rvalid),
    .m_axi_rdata(m_axi_rdata),
    .m_axi_rresp(m_axi_rresp),
    .m_axi_rlast(m_axi_rlast),
    .data_to_pams_read(data_to_pams_read),
    .data_to_pams_valid(data_to_pams_valid),
    .data_to_pams(to_kernel_tdata),
    .data_from_pams_read(from_kernel_tready),
    .data_from_pams_valid(from_kernel_tvalid),
    .data_from_pams(from_kernel_tdata),
    .pams_ctrl_inputs(pams_ctrl_inputs),
    .pams_ctrl_inputs_ack(pams_ctrl_inputs_ack),
    .pams_ctrl_outputs(pams_ctrl_outputs),
    .pams_ctrl_outputs_ack(pams_ctrl_outputs_ack)
);

assign dbg_data_to_pams_read = data_to_pams_read;
assign dbg_data_to_pams_valid = data_to_pams_valid;
assign dbg_data_to_pams = to_kernel_tdata;
assign dbg_data_from_pams_read = from_kernel_tready;
assign dbg_data_from_pams_valid = from_kernel_tvalid;
assign dbg_data_from_pams = from_kernel_tdata;
assign dbg_pams_ctrl_inputs = pams_ctrl_inputs;
assign dbg_pams_ctrl_inputs_ack = pams_ctrl_inputs_ack;
assign dbg_pams_ctrl_outputs = pams_ctrl_outputs;
assign dbg_pams_ctrl_outputs_ack = pams_ctrl_outputs_ack;
assign dbg_currentstate = currentstate;

assign to_kernel_tuser = 8'b0;
assign to_kernel_tkeep = 4'hF;
assign to_kernel_tdest = 8'b0;
assign to_kernel_tid = 8'b0;
assign to_kernel_tlast = 1'b0;
assign to_kernel_tvalid = data_to_pams_valid & currentstate == state_read_stats & stats_counter < 3;
    
endmodule