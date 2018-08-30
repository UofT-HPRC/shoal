`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 08/22/2018 09:57:31 PM
// Design Name: 
// Module Name: sample_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


// module sample_tb();

//     reg clock;
//     reg rst_n;

//     wire ack;
//     wire [2:0] state_out;
    
//     reg [63:0] input_tdata;
//     reg input_tlast;
//     wire input_tready;
//     reg input_tvalid;

//     wire [63:0] output_tdata;
//     wire output_tlast;
//     reg output_tready;
//     wire output_tvalid;

//     initial begin
//         clock = 0;
//         rst_n = 0;
//         output_tready = 1;
//         input_tdata = 64'hABCD;
//         input_tlast = 0;
//         input_tvalid = 0;

//         #50 rst_n = 1;
//         #38
//         input_tvalid = 1;
//         @(posedge clock iff input_tready) //waits for both events simultaneously
//         #20 input_tvalid = 0;

//         wait(ack == 1);
//         #18
//         input_tvalid = 1;
//         input_tdata = 9;
//         @(posedge clock iff input_tready)
//         #20 input_tvalid = 0;

//         wait(ack == 1);
        
//     end

//     always #10 clock <= ~clock;
    
//     sample sample_i(
//         .ack_V(ack),
//         .ap_clk(clock),
//         .ap_rst_n(rst_n),
//         .axis_input_TDATA(input_tdata),
//         .axis_input_TLAST(input_tlast),
//         .axis_input_TREADY(input_tready),
//         .axis_input_TVALID(input_tvalid),
//         .axis_output_TDATA(output_tdata),
//         .axis_output_TLAST(output_tlast),
//         .axis_output_TREADY(output_tready),
//         .axis_output_TVALID(output_tvalid),
//         .state_out_V(state_out)
//     );

// endmodule

//*******************************************************

// `include "/home/sharm294/Documents/masters/git_repos/shoal/share/include/axis_interface.sv"

// module exerciser (
//     input clock,
//     input ack,
//     output logic rst_n,
//     axi_stream.axis_m_mp AXIS_M,
//     axi_stream.axis_s_mp AXIS_S
// );
//     initial begin
//         rst_n = 0;
//         AXIS_M.tvalid = 0;
//         AXIS_M.tdata = 0;
//         AXIS_M.tlast = 0;
//         AXIS_S.tready = 1;
//         #50 rst_n = 1;

//         #38
//         AXIS_M.tvalid <= 1;
//         @(posedge clock iff AXIS_M.tready) //waits for both events simultaneously
//         #20 AXIS_M.tvalid <= 0;

//         wait(ack == 1);
//         #18
//         AXIS_M.tvalid <= 1;
//         AXIS_M.tdata <= 9;
//         @(posedge clock iff AXIS_M.tready)
//         #20 AXIS_M.tvalid <= 0;

//         wait(ack == 1);        
//     end
    
// endmodule

// module sample_tb();

//     logic clock;
//     logic rst_n;

//     logic ack;
//     logic [2:0] state_out;

//     logic [63:0] input_tdata;
//     logic input_tlast;
//     logic input_tready;
//     logic input_tvalid;

//     logic [63:0] output_tdata;
//     logic output_tlast;
//     logic output_tready;
//     logic output_tvalid;

//     //clock generation
//     initial begin
//         clock = 0;
//         forever #10 clock <= ~clock;
//     end

//     //initialize interfaces
//     axi_stream master(
//         .aclk(clock)
//     );
    
//     axi_stream slave(
//         .aclk(clock)
//     );

//     axis_m master_m(
//         .AXIS_M(master),
//         .tdata(input_tdata),
//         .tvalid(input_tvalid),
//         .tlast(input_tlast),
//         .tready(input_tready)
//     );

//     axis_s slave_s(
//         .AXIS_S(slave),
//         .tdata(output_tdata),
//         .tvalid(output_tvalid),
//         .tlast(output_tlast),
//         .tready(output_tready)
//     );

//     exerciser exerciser_i(
//         .clock(clock),
//         .ack(ack),
//         .rst_n(rst_n),
//         .AXIS_M(master),
//         .AXIS_S(slave)
//     );

//     always_comb begin
//         master_m.write();
//         slave_s.read();
//     end

//     //initialize DUT
//     sample sample_i(
//         .ack_V(ack),
//         .ap_clk(clock),
//         .ap_rst_n(rst_n),
//         .axis_input_TDATA(input_tdata),
//         .axis_input_TLAST(input_tlast),
//         .axis_input_TREADY(input_tready),
//         .axis_input_TVALID(input_tvalid),
//         .axis_output_TDATA(output_tdata),
//         .axis_output_TLAST(output_tlast),
//         .axis_output_TREADY(output_tready),
//         .axis_output_TVALID(output_tvalid),
//         .state_out_V(state_out)
//     );

// endmodule

//*******************************************************

`include "/home/sharm294/Documents/masters/git_repos/shoal/share/include/axis_interface.sv"

module exerciser (
    output logic clock,
    output logic rst_n,

    input ack,
    axi_stream.axis_m_mp AXIS_M,
    axi_stream.axis_s_mp AXIS_S
);

    string interfaceInput = "axis_input";
    string interfaceOutput = "axis_output";
    string waitCondition = "ack";
    string interfaceWait = "wait";

    //clock generation
    initial begin
        clock = 0;
        forever #10 clock <= ~clock;
    end

    initial begin
        AXIS_M.tvalid = 0;
        AXIS_M.tdata = 0;
        AXIS_M.tlast = 0;
        AXIS_S.tready = 1;

        #100

        if (interfaceInput == "axis_input") begin
            AXIS_M.tvalid = 1;
            AXIS_M.tdata = 64'hABCD;
            AXIS_M.tlast = 0;
            @(posedge clock iff AXIS_M.tready)
            #20 AXIS_M.tvalid <= 0;
        end

        if (interfaceWait == "wait") begin
            if(waitCondition == "ack") begin
                wait(ack == 1);
            end
            //...
        end

        if (interfaceInput == "axis_input") begin
            AXIS_M.tvalid = 1;
            AXIS_M.tdata = 0;
            AXIS_M.tlast = 0;
            @(posedge clock iff AXIS_M.tready)
            #20 AXIS_M.tvalid <= 0;
        end

        if (interfaceWait == "wait") begin
            if(waitCondition == "ack") begin
                wait(ack == 1);
            end
            //...
        end     
    end

    initial begin //need an initial for reset
        rst_n = 0;
        #50 rst_n = 1;
    end

    initial begin 
        if (interfaceWait == "wait") begin
            if(waitCondition == "ack") begin
                wait(ack == 1);
            end
            //...
        end

        if (interfaceWait == "wait") begin
            if(waitCondition == "ack") begin
                wait(ack == 1);
            end
            //...
        end

        if (interfaceOutput == "axis_output") begin
            @(posedge clock iff AXIS_S.tready && AXIS_S.tvalid)
            assert(AXIS_S.tdata == 64'h1);
            assert(AXIS_S.tlast == 1'b0);
        end
    end    
endmodule

module sample_tb();

    logic clock;
    logic rst_n;

    logic ack;
    logic [2:0] state_out;

    logic [63:0] input_tdata;
    logic input_tlast;
    logic input_tready;
    logic input_tvalid;

    logic [63:0] output_tdata;
    logic output_tlast;
    logic output_tready;
    logic output_tvalid;

    //initialize interfaces
    axi_stream master(
        .aclk(clock)
    );
    
    axi_stream slave(
        .aclk(clock)
    );

    axis_m master_m(
        .AXIS_M(master),
        .tdata(input_tdata),
        .tvalid(input_tvalid),
        .tlast(input_tlast),
        .tready(input_tready)
    );

    axis_s slave_s(
        .AXIS_S(slave),
        .tdata(output_tdata),
        .tvalid(output_tvalid),
        .tlast(output_tlast),
        .tready(output_tready)
    );

    exerciser exerciser_i(
        .clock(clock),
        .ack(ack),
        .rst_n(rst_n),
        .AXIS_M(master),
        .AXIS_S(slave)
    );

    always_comb begin
        master_m.write();
        slave_s.read();
    end

    //initialize DUT
    sample sample_i(
        .ack_V(ack),
        .ap_clk(clock),
        .ap_rst_n(rst_n),
        .axis_input_TDATA(input_tdata),
        .axis_input_TLAST(input_tlast),
        .axis_input_TREADY(input_tready),
        .axis_input_TVALID(input_tvalid),
        .axis_output_TDATA(output_tdata),
        .axis_output_TLAST(output_tlast),
        .axis_output_TREADY(output_tready),
        .axis_output_TVALID(output_tvalid),
        .state_out_V(state_out)
    );

endmodule
