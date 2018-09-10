`timescale #TIMESCALE#
////////////////////////////////////////////////////////////////////////////////
// Company: #COMPANY#
// Engineer: #ENGINEER#
// 
// Create Date: #CURR_DATE#
// Module Name: #MODULE_NAME#
// Project Name: #PROJECT_NAME#
// Target Devices: #TARGET_DEVICES#
// Tool Versions: #TOOL_VERSIONS#
// Description: #DESCRIPTION#
// 
// Dependencies: #DEPENDENCIES#
// 
////////////////////////////////////////////////////////////////////////////////

//defines the axis interfaces
`include "axis_interface.sv"

//filename for the input data file
string dataFileName = #DATA_FILE#;

localparam MAX_DATA_SIZE = #MAX_DATA_SIZE#; //max width of the data to be read/writtn
localparam MAX_VECTORS = #MAX_VECTORS#; //number of test vectors
localparam MAX_PARALLEL = #MAX_PARALLEL#;  //max number of parallel sections in any vector
localparam MAX_SEEK_SIZE = 64; //base 2 log of the max number to fseek
localparam MAX_KEEP_SIZE = #MAX_KEEP_SIZE#; //max width (bits) of the keep field in axis

//This module provides the stimulus for the DUT by reading the data file
module exerciser (
    #EXERCISER_PORTS#
);

    logic [MAX_SEEK_SIZE-1:0] parallelSections [MAX_PARALLEL];

    logic [MAX_PARALLEL-1:0] testVectorEnd = 0;
    logic updateEnd = 0;

    /***************************************************************************
    * EDIT THIS TASK AS NEEDED
    ***************************************************************************/
    task evaluateData(
        input logic [MAX_DATA_SIZE-1:0] tdata,
        input logic tlast,
        input logic [MAX_KEEP_SIZE-1:0] tkeep,
        input string packetType_par,
        input string interfaceType_par,
        output logic done
    );
        
        if (packetType_par == "wait") begin
            #IF_ELSE_WAIT#
            else begin
                $display({"Unhandled case for wait type: ", interfaceType_par});
            end
        end
        else if (packetType_par == "signal") begin
            #IF_ELSE_SIGNAL#
            else begin
                $display({"Unhandled case for signal type: ", 
                    interfaceType_par});
            end
        end
        else if (packetType_par == "delay") begin
            if(interfaceType_par == "ns") begin
                #(tdata);
            end
            else begin
                $display({"Unhandled case for delay type: ", 
                    interfaceType_par});
            end
        end
        else if (packetType_par == "timestamp") begin
            $display("%s: %t", interfaceType_par, $time);
        end
        else if(packetType_par == "end") begin
            $display("Test vector %d complete", tdata);
            done = 1'b1;
        end
        #ELSE_IF_AXIS_IN#
        #ELSE_IF_AXIS_OUT#
        else begin
            $display({"Unhandled case: ", packetType_par, " " , 
                interfaceType_par});
            $finish;
        end
    endtask

    /***************************************************************************
    * DO NOT EDIT BELOW UNLESS YOU KNOW WHAT YOU'RE DOING
    ***************************************************************************/

    //clock generation
    #INITIAL_CLOCK#
    
    int vectorCount;

    initial begin
        int status;
        string packetType;
        string interfaceType;

        logic [MAX_SEEK_SIZE-1:0] testVectors [MAX_VECTORS];

        int dataFile_0;
        int parallelSectionCount;

        dataFile_0 = $fopen(dataFileName, "r");
        status = $fscanf(dataFile_0, "%s %s %d\n", packetType, interfaceType, 
            vectorCount);
        if (packetType == "TestVector" && interfaceType == "count") begin
            for(int i = 0; i < vectorCount; i++) begin
                status = $fscanf(dataFile_0, "%s %s %d\n", packetType, 
                    interfaceType, testVectors[i]);
            end
            for(int i = 0; i < vectorCount; i++) begin
                status = $fseek(dataFile_0, testVectors[i], 0);
                status = $fscanf(dataFile_0, "%s %s %d\n", packetType, 
                    interfaceType, parallelSectionCount);
                if (packetType == "ParallelSection" && 
                    interfaceType == "count") begin
                    for(int j = 0; j < parallelSectionCount; j++) begin
                        status = $fscanf(dataFile_0, "%s %s %d\n", packetType, 
                            interfaceType,parallelSections[j]);
                    end
                    updateEnd = 1;
                    wait(|testVectorEnd == 1);
                    updateEnd = 0;
                    @(posedge #VECTOR_CLOCK#)
                    for(int z = 0; z < MAX_PARALLEL; z++) begin
                        parallelSections[z] = 0;
                    end
                end
                else begin
                    $display("Bad data file - parallelsection header");
                    $finish;
                end
            end
            $display("All tests completed!");
        end
        else begin
            $display("Bad data file - vector header");
            $finish;
        end
    end    

    generate;
        for(genvar gen_i = 0; gen_i < MAX_PARALLEL; gen_i++) begin
            initial begin
                int status_par;
                int dataFile; 
                logic [MAX_DATA_SIZE-1:0] tdata;
                logic tlast;
                logic [MAX_KEEP_SIZE-1:0] tkeep; 
                string packetType_par; 
                string interfaceType_par; 
                int packetCount;
                
                dataFile = $fopen(dataFileName, "r");
                for(int w = 0; w < vectorCount; w++) begin
                    wait(updateEnd == 1'b1);
                    if (parallelSections[gen_i] != 0) begin
                        status_par = $fseek(dataFile, parallelSections[gen_i], 
                            0);
                        status_par = $fscanf(dataFile, "%s %s %d\n", 
                            packetType_par, interfaceType_par, packetCount);
                        for(int k = 0; k < packetCount; k++) begin
                            status_par = $fscanf(dataFile, "%s %s %d %d %d\n", 
                                packetType_par, interfaceType_par, tdata, 
                                tlast, tkeep);

                            evaluateData(tdata, tlast, tkeep, packetType_par,
                                interfaceType_par, testVectorEnd[gen_i]);
                        end
                    end
                    wait(updateEnd == '0);
                    testVectorEnd = '0;                    
                end
            end
        end
    endgenerate
   
endmodule

module am_rx_tb();

    #TB_SIGNAL_LIST#

    //initialize interfaces
    #TB_AXIS_LIST#

    #EXERCISER_INT#
    
    always_comb begin
        #AXIS_ASSIGN#
    end

    //initialize DUT
    #DUT_INST#

endmodule