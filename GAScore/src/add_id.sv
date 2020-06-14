module add_id #(
    parameter TDATA_WIDTH = 64,
    parameter TDEST_WIDTH = 16,
    parameter TID_WIDTH = 16,
    parameter TUSER_WIDTH = 16,
    parameter FIFO_DEPTH = 2048,

    parameter TKEEP_WIDTH = TDATA_WIDTH/8
) (
    input ap_clk,
    input ap_rst_n,

    input [TDATA_WIDTH-1:0] in_TDATA,
    input in_TVALID,
    output in_TREADY,
    input [TDEST_WIDTH-1:0] in_TDEST,
    input in_TLAST,
    input [TKEEP_WIDTH-1:0] in_TKEEP,

    output [TDATA_WIDTH-1:0] out_TDATA,
    output out_TVALID,
    input out_TREADY,
    output [TDEST_WIDTH-1:0] out_TDEST,
    output out_TLAST,
    output [TKEEP_WIDTH-1:0] out_TKEEP,
    output [TID_WIDTH-1:0] out_TID,
    output [TUSER_WIDTH-1:0] out_TUSER
);

logic [TDATA_WIDTH-1:0] m_axis_tdata;
logic m_axis_tvalid;
logic m_axis_tready;
logic [TDEST_WIDTH-1:0] m_axis_tdest;
logic m_axis_tlast;
logic [TKEEP_WIDTH-1:0] m_axis_tkeep;
logic s_axis_tready;

localparam FIFO_WIDTH = $clog2(FIFO_DEPTH);

function integer roundToEight;
    
    input integer arg;
    begin
        if (arg % 8 == 0) begin
            roundToEight = arg;
        end else begin
            roundToEight = ((arg / 8) + 1) * 8;
        end
    end    
endfunction
localparam FIFO_WIDTH_ROUNDED = roundToEight(FIFO_WIDTH);

localparam STATE_WAIT = 2'd0;
localparam STATE_LAST = 2'd1;
localparam STATE_COUNT = 2'd2;
localparam STATE_ERROR = 2'd3;
logic [1:0] currentState, nextState;

// https://www.xilinx.com/support/documentation/sw_manuals/xilinx2018_1/ug974-vivado-ultrascale-libraries.pdf
// pg 48
xpm_fifo_axis #(
    .CDC_SYNC_STAGES(2), // only valid if clocking mode is "independent_clock"
    .CLOCKING_MODE("common_clock"), // "common_clock" or "independent_clock"
    .ECC_MODE("no_ecc"), // "no_ecc" or "en_ecc
    .FIFO_DEPTH(FIFO_DEPTH), // DECIMAL
    .FIFO_MEMORY_TYPE("auto"), // String
    .PACKET_FIFO("true"), // String
    .PROG_EMPTY_THRESH(7), // assert prog_empty_axis when word count <= (Decimal)
    .PROG_FULL_THRESH(7), // assert prog_empty_axis when word count >= (Decimal)
    .RD_DATA_COUNT_WIDTH(FIFO_WIDTH), // DECIMAL
    .RELATED_CLOCKS(0), // DECIMAL
    .TDATA_WIDTH(TDATA_WIDTH), // DECIMAL
    .TDEST_WIDTH(TDEST_WIDTH), // DECIMAL
    .TID_WIDTH(TID_WIDTH), // DECIMAL
    .TUSER_WIDTH(TUSER_WIDTH), // DECIMAL
    .USE_ADV_FEATURES("1000"), // enable r/w counts, prog empty/full
    .WR_DATA_COUNT_WIDTH(1) // DECIMAL
) xpm_fifo_axis_inst (
    .s_aclk(ap_clk), // 1-bit input: Slave Interface Clock
    .m_aclk(ap_clk), // 1-bit input: Master Interface Clock
    .s_aresetn(ap_rst_n), // 1-bit input: Active low asynchronous reset.
    
    .s_axis_tdata(in_TDATA), // TDATA_WIDTH-bit input: TDATA
    .s_axis_tdest(in_TDEST), // TDEST_WIDTH-bit input: TDEST
    .s_axis_tid('0), // TID_WIDTH-bit input: TID
    .s_axis_tkeep(in_TKEEP), // TDATA_WIDTH-bit input: TKEEP
    .s_axis_tlast(in_TLAST), // 1-bit input: TLAST
    .s_axis_tstrb('1), // TDATA_WIDTH-bit input: TSTRB
    .s_axis_tuser('0), // TUSER_WIDTH-bit input: TUSER
    .s_axis_tvalid(in_TVALID & (currentState != STATE_LAST)), // 1-bit input: TVALID
    .s_axis_tready(s_axis_tready), // 1-bit output: TREADY
    
    .m_axis_tdata(m_axis_tdata), // TDATA_WIDTH-bit output: TDATA
    .m_axis_tdest(m_axis_tdest), // TDEST_WIDTH-bit output: TDEST
    .m_axis_tid(), // TID_WIDTH-bit output: TID
    .m_axis_tkeep(m_axis_tkeep), // TDATA_WIDTH-bit output: TKEEP
    .m_axis_tlast(m_axis_tlast), // 1-bit output: TLAST
    .m_axis_tstrb(), // TDATA_WIDTH-bit output: TSTRB
    .m_axis_tuser(), // TUSER_WIDTH-bit output: TUSER
    .m_axis_tvalid(m_axis_tvalid), // 1-bit output: TVALID
    .m_axis_tready(m_axis_tready), // 1-bit input: TREADY
    
    .almost_empty_axis(), // 1-bit output: one more read can be performed
    .almost_full_axis(), // 1-bit output: one more write can be performed
    .prog_empty_axis(), // 1-bit output: Programmable Empty
    .prog_full_axis(), // 1-bit output: Programmable Full
    .rd_data_count_axis(), // RD_DATA_COUNT_WIDTH-bit output: Read Data Count
    .wr_data_count_axis(), // WR_DATA_COUNT_WIDTH-bit output: Write Data Count

    .sbiterr_axis(), // 1-bit output: Single Bit Error
    .dbiterr_axis(), // 1-bit output: Double Bit Error
    .injectdbiterr_axis(), // 1-bit input: Double Bit Error Injection
    .injectsbiterr_axis() // 1-bit input: Single Bit Error Injection
);

logic increment_counter;

always @(*) begin
    increment_counter = 0;
    case(currentState)
        STATE_WAIT: begin
            if (in_TVALID & in_TREADY) begin
                if (in_TLAST) begin
                    nextState = STATE_LAST;
                end else begin
                    nextState = STATE_COUNT;
                end
            end else begin
                nextState = STATE_WAIT;
            end
        end
        STATE_COUNT: begin
            if (in_TVALID & in_TREADY) begin
                increment_counter = 1;
                if (in_TLAST) begin
                    nextState = STATE_LAST;
                end else begin
                    nextState = STATE_COUNT;
                end
            end else begin
                nextState = STATE_COUNT;
            end
        end
        STATE_LAST: begin
            if (counter_fifo_tready)
                nextState = STATE_WAIT;
            else
                nextState = STATE_LAST;
        end
        default:
            nextState = STATE_ERROR;
    endcase
end

logic [TID_WIDTH-1:0] tid_reg;
logic [FIFO_WIDTH_ROUNDED-1:0] counter;
logic [FIFO_WIDTH_ROUNDED-1:0] counter_out;

always @(posedge ap_clk) begin
    if (~ap_rst_n) begin
        counter <= 1'b1;
        currentState <= STATE_WAIT;
        tid_reg <= '0;
    end else begin
        currentState <= nextState;
        case(currentState)
            STATE_WAIT: begin
                tid_reg <= in_TDATA[39:24]; // AM_DST
                counter <= 1'b1;
            end
            STATE_COUNT: begin
                if (increment_counter) begin
                    counter <= counter + 1'b1;
                end
            end
            STATE_LAST:
                counter <= 1'b1;
        endcase
    end
end

logic counter_fifo_tready;
logic counter_valid;
logic [TID_WIDTH-1:0] counter_tid;

xpm_fifo_axis #(
    .CDC_SYNC_STAGES(2), // only valid if clocking mode is "independent_clock"
    .CLOCKING_MODE("common_clock"), // "common_clock" or "independent_clock"
    .ECC_MODE("no_ecc"), // "no_ecc" or "en_ecc
    .FIFO_DEPTH(FIFO_DEPTH), // DECIMAL
    .FIFO_MEMORY_TYPE("auto"), // String
    .PACKET_FIFO("true"), // String
    .PROG_EMPTY_THRESH(7), // assert prog_empty_axis when word count <= (Decimal)
    .PROG_FULL_THRESH(7), // assert prog_empty_axis when word count >= (Decimal)
    .RD_DATA_COUNT_WIDTH(FIFO_WIDTH), // DECIMAL
    .RELATED_CLOCKS(0), // DECIMAL
    .TDATA_WIDTH(FIFO_WIDTH_ROUNDED), // DECIMAL
    .TDEST_WIDTH(1), // DECIMAL
    .TID_WIDTH(TID_WIDTH), // DECIMAL
    .TUSER_WIDTH(1), // DECIMAL
    .USE_ADV_FEATURES("1000"), // enable r/w counts, prog empty/full
    .WR_DATA_COUNT_WIDTH(1) // DECIMAL
) xpm_fifo_counter (
    .s_aclk(ap_clk), // 1-bit input: Slave Interface Clock
    .m_aclk(ap_clk), // 1-bit input: Master Interface Clock
    .s_aresetn(ap_rst_n), // 1-bit input: Active low asynchronous reset.
    
    .s_axis_tdata(counter), // TDATA_WIDTH-bit input: TDATA
    .s_axis_tdest(), // TDEST_WIDTH-bit input: TDEST
    .s_axis_tid(tid_reg), // TID_WIDTH-bit input: TID
    .s_axis_tkeep('1), // TDATA_WIDTH-bit input: TKEEP
    .s_axis_tlast(1'b1), // 1-bit input: TLAST
    .s_axis_tstrb('1), // TDATA_WIDTH-bit input: TSTRB
    .s_axis_tuser(), // TUSER_WIDTH-bit input: TUSER
    .s_axis_tvalid(currentState == STATE_LAST), // 1-bit input: TVALID
    .s_axis_tready(counter_fifo_tready), // 1-bit output: TREADY
    
    .m_axis_tdata(counter_out), // TDATA_WIDTH-bit output: TDATA
    .m_axis_tdest(), // TDEST_WIDTH-bit output: TDEST
    .m_axis_tid(counter_tid), // TID_WIDTH-bit output: TID
    .m_axis_tkeep(), // TDATA_WIDTH-bit output: TKEEP
    .m_axis_tlast(), // 1-bit output: TLAST
    .m_axis_tstrb(), // TDATA_WIDTH-bit output: TSTRB
    .m_axis_tuser(), // TUSER_WIDTH-bit output: TUSER
    .m_axis_tvalid(counter_valid), // 1-bit output: TVALID
    .m_axis_tready(m_axis_tlast & out_TREADY), // 1-bit input: TREADY
    
    .almost_empty_axis(), // 1-bit output: one more read can be performed
    .almost_full_axis(), // 1-bit output: one more write can be performed
    .prog_empty_axis(), // 1-bit output: Programmable Empty
    .prog_full_axis(), // 1-bit output: Programmable Full
    .rd_data_count_axis(), // RD_DATA_COUNT_WIDTH-bit output: Read Data Count
    .wr_data_count_axis(), // WR_DATA_COUNT_WIDTH-bit output: Write Data Count

    .sbiterr_axis(), // 1-bit output: Single Bit Error
    .dbiterr_axis(), // 1-bit output: Double Bit Error
    .injectdbiterr_axis(), // 1-bit input: Double Bit Error Injection
    .injectsbiterr_axis() // 1-bit input: Single Bit Error Injection
);

assign out_TID = counter_tid;
assign out_TUSER = counter_out;
assign out_TDATA = m_axis_tdata;
assign out_TVALID = m_axis_tvalid & counter_valid;
assign m_axis_tready = out_TREADY & counter_valid;
assign out_TDEST = m_axis_tdest;
assign out_TLAST = m_axis_tlast;
assign out_TKEEP = m_axis_tkeep;
assign in_TREADY = s_axis_tready & (currentState != STATE_LAST);
    
endmodule
