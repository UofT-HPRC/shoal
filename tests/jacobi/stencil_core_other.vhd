------------------------------------------------------------------------------
-- stencil_core - entity/architecture pair
------------------------------------------------------------------------------
------------------------------------------------------------------------------
-- Filename:          stencil_core
-- Version:           1.00.a
-- Description:       Computation core (VHDL).
-- Date:              Mon Nov 21 16:06:07 2011 (by Create and Import Peripheral Wizard)
-- VHDL Standard:     VHDL'93
------------------------------------------------------------------------------
-- Naming Conventions:
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

library stencil_core_v2_00_a;
use stencil_core_v2_00_a.sync_fifo_bram_large;

entity stencil_core is
	generic
	(
        C_M_AXI_ADDR_WIDTH : integer := 32;
        C_M_AXI_DATA_WIDTH : integer := 32;
		C_MAX_BURST_LEN   : integer := 16;
		C_BRAMS_PER_ROW: integer := 8;
		C_MEM_BASEADDR: std_logic_vector(31 downto 0) := X"80000000";
		C_MEM_SIZE: std_logic_vector(31 downto 0)     := X"80000000";
		C_CLEAR_MEM : integer := 0
	);
	port 
	(
		-- Bus protocol ports, do not add or delete. 
		HOST_Clk	: in	std_logic;
		HOST_Rst	: in	std_logic;
		
		FROM_GC_Clk	: out	std_logic;
		FROM_GC_Read	: out	std_logic;
		FROM_GC_Data	: in	std_logic_vector(0 to 31);
		FROM_GC_Control	: in	std_logic;
		FROM_GC_Exists	: in	std_logic;
		TO_GC_Clk	: out	std_logic;
		TO_GC_Write	: out	std_logic;
		TO_GC_Data	: out	std_logic_vector(0 to 31);
		TO_GC_Control	: out	std_logic;
		TO_GC_Full	: in	std_logic;
		AM_RETURN_Clk	: out	std_logic;
		AM_RETURN_Read	: out	std_logic;
		AM_RETURN_Data	: in	std_logic_vector(0 to 31);
		AM_RETURN_Control	: in	std_logic;
		AM_RETURN_Exists	: in	std_logic;
		HANDLER_RETURN_Clk	: out	std_logic;
		HANDLER_RETURN_Write	: out	std_logic;
		HANDLER_RETURN_Data	: out	std_logic_vector(0 to 31);
		HANDLER_RETURN_Control	: out	std_logic;
		HANDLER_RETURN_Full	: in	std_logic;


		-- AXI PORT
        m_axi_aclk               : in  std_logic;
        m_axi_aresetn               : in  std_logic;
		md_error                 : out std_logic;

    	-- Write Address Channel
        m_axi_awready            : in  std_logic;
        m_axi_awvalid            : out std_logic;
        m_axi_awaddr             : out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
        m_axi_awlen              : out std_logic_vector(7 downto 0);
        m_axi_awsize             : out std_logic_vector(2 downto 0);
        m_axi_awburst            : out std_logic_vector(1 downto 0);
        m_axi_awprot             : out std_logic_vector(2 downto 0);
        m_axi_awcache            : out std_logic_vector(3 downto 0);

        -- Write Data Channel
        m_axi_wready             : in  std_logic;
        m_axi_wvalid             : out std_logic;
        m_axi_wdata              : out std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
        m_axi_wstrb              : out std_logic_vector((C_M_AXI_DATA_WIDTH/8)-1 downto 0);
        m_axi_wlast              : out std_logic;

        -- Write Response Channel
        m_axi_bready             : out std_logic;
        m_axi_bvalid             : in  std_logic;
        m_axi_bresp              : in  std_logic_vector(1 downto 0);

        -- Read Address Channel
        m_axi_arready            : in  std_logic;
        m_axi_arvalid            : out std_logic;
        m_axi_araddr             : out std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
        m_axi_arlen              : out std_logic_vector(7 downto 0);
        m_axi_arsize             : out std_logic_vector(2 downto 0);
        m_axi_arburst            : out std_logic_vector(1 downto 0);
        m_axi_arprot             : out std_logic_vector(2 downto 0);
        m_axi_arcache            : out std_logic_vector(3 downto 0);
                                                                                           
        -- Read Data Channel
        m_axi_rready             : out std_logic;
        m_axi_rvalid             : in  std_logic;
        m_axi_rdata              : in  std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
        m_axi_rresp              : in  std_logic_vector(1 downto 0);
        m_axi_rlast              : in  std_logic


	);


end stencil_core;

architecture archie of stencil_core is

	-- functions
	function burst_start(addr: in std_logic_vector(31 downto 0)) return std_logic_vector is
	begin
		return addr(31 downto 6) & "000000";
	end function burst_start;

	function burstlen_to_end(addr: in std_logic_vector(31 downto 0)) return std_logic_vector is
	begin
		return "1111" - addr(5 downto 2);
	end function burstlen_to_end;
	
	function burstlen_from_start(addr: in std_logic_vector(31 downto 0)) return std_logic_vector is
	begin
		return addr(5 downto 2) - "0000"; -- obsolete, right?
	end function burstlen_from_start;
	
	function burstlen(addr: in std_logic_vector(29 downto 0); lineend: in std_logic_vector(29 downto 0)) return std_logic_vector is
	begin
		if (addr(29 downto 4)=lineend(29 downto 4)) then
			return lineend(3 downto 0) - addr(3 downto 0); 
		else
			return "1111" - addr(3 downto 0);
		end if;
	end function burstlen;

	function abs_diff(a: in std_logic_vector(31 downto 0); b: in std_logic_vector(31 downto 0)) return std_logic_vector is
		variable diff: std_logic_vector(31 downto 0);
	begin
		diff := a - b;
		if (diff(31)='1') then -- negative
			return 0-diff;
		else				   -- positive
			return diff;
		end if; 
	end function abs_diff; --TODO: check efficiency of mux/inversion

	function pick_larger(a: in std_logic_vector(31 downto 0); b: in std_logic_vector(31 downto 0)) return std_logic_vector is
	begin
		if a>b then
			return a;
		else
			return b;
		end if;
	end function pick_larger;
	
	-- prototypes
	component Sync_FIFO_BRAM_LARGE
		generic(WordWidth : integer := 32;
			    BRAMs     : integer := 16);
		port(Reset      : in  std_logic;
			 Clk        : in  std_logic;
			 WE         : in  std_logic;
			 DataIn     : in  std_logic_vector(WordWidth - 1 downto 0);
			 Full       : out std_logic;
			 Reset_Read : in  std_logic;
			 RD         : in  std_logic;
			 DataOut    : out std_logic_vector(WordWidth - 1 downto 0);
			 Exists     : out std_logic);
	end component Sync_FIFO_BRAM_LARGE;

	component psm is
	generic (
		ctrl_inputs: integer := 8;
		ctrl_outputs: integer := 8;
		coderam_size: integer := 1024
	);
	port (
		clock : in std_logic;
		reset : in std_logic;
		ctrl_in : 			in std_logic_vector( ctrl_inputs-1 downto 0);
		ctrl_in_ack : 		out std_logic_vector( ctrl_inputs-1 downto 0);
		ctrl_out : 			out std_logic_vector( ctrl_outputs-1 downto 0);
		ctrl_out_ack : 		in std_logic_vector( ctrl_outputs-1 downto 0);
		core_data_in_valid : in std_logic;
		core_data_in_read :  out std_logic;
		core_data_in :       in std_logic_vector(31 downto 0);
		core_data_out_valid : out std_logic;
		core_data_out_read :  in std_logic;
		core_data_out :       out std_logic_vector(31 downto 0);
		FROM_GC_Clk	: out	std_logic;
		FROM_GC_Read	: out	std_logic;
		FROM_GC_Data	: in	std_logic_vector(0 to 31);
		FROM_GC_Control	: in	std_logic;
		FROM_GC_Exists	: in	std_logic;
		TO_GC_Clk	: out	std_logic;
		TO_GC_Write	: out	std_logic;
		TO_GC_Data	: out	std_logic_vector(0 to 31);
		TO_GC_Control	: out	std_logic;
		TO_GC_Full	: in	std_logic;
		HANDLER_RETURN_Clk	: out	std_logic;
		HANDLER_RETURN_Write	: out	std_logic;
		HANDLER_RETURN_Data	: out	std_logic_vector(0 to 31);
		HANDLER_RETURN_Control	: out	std_logic;
		HANDLER_RETURN_Full	: in	std_logic;
		AM_RETURN_Clk	: out	std_logic;
		AM_RETURN_Read	: out	std_logic;
		AM_RETURN_Data	: in	std_logic_vector(0 to 31);
		AM_RETURN_Control	: in	std_logic;
		AM_RETURN_Exists	: in	std_logic);
	end component;
	
	component multiplier
	port (
		clk : in std_logic;
		a : in std_logic_vector(31 downto 0);
		b : in std_logic_vector(31 downto 0);
		ce : in std_logic;
		sclr : in std_logic;
		p : out std_logic_vector(63 downto 0)
		);
	end component;
	
	
	-- general signals
	signal clock: std_logic;
	signal reset: std_logic;

	-- buffer interface
	signal DataIn : std_logic_vector(31 downto 0);
	signal Read0, Read1, Read2, Read3 : std_logic;
	signal ReadRows : std_logic;
	signal InWE, WE0, WE1, WE2, WE3 : std_logic;
	signal InFull, Full0,   Full1,   Full2,   Full3 : std_logic;
	signal ResetBuffer0,ResetBuffer1,ResetBuffer2,ResetBuffer3 : std_logic;
	signal Reset_Write0, Reset_Write1, Reset_Write2, Reset_Write3 : std_logic;

	signal Reset_Read0, Reset_Read1, Reset_Read2, Reset_Read3 : std_logic;
	signal Reset_ReadA, Reset_ReadB, Reset_ReadC : std_logic;
	signal DataOut0,    DataOut1,    DataOut2,    DataOut3 : std_logic_vector(31 downto 0);
	signal DataOutA,    DataOutB,    DataOutC : std_logic_vector(31 downto 0);
	signal Exists0, Exists1, Exists2, Exists3 : std_logic;
	signal ExistsA, ExistsB, ExistsC : std_logic;
	
	-- data size-dependent constants
	constant BurstSize: integer := C_MAX_BURST_LEN;
	constant RowBufferDepth: std_logic_vector(15 downto 0) := CONV_STD_LOGIC_VECTOR(C_BRAMS_PER_ROW*1024,16);-------1024; -- 1 BRAM = 4kbyte = 1kWords
	
	constant Row_Xmin: std_logic_vector(15 downto 0) := CONV_STD_LOGIC_VECTOR(1,16);--integer := 1;									-- BurstSize; -- 16
	constant Row_Xmin_aura: std_logic_vector(15 downto 0) := CONV_STD_LOGIC_VECTOR(0,16);--integer :=  0;								-- Row_Xmin -1; -- 15 
	constant Ymin: std_logic_vector(15 downto 0) := CONV_STD_LOGIC_VECTOR(1,16);--integer :=  1;
	constant Ymin_aura: std_logic_vector(15 downto 0) := CONV_STD_LOGIC_VECTOR(0,16);--integer :=  0;





	signal XSIZEsig : std_logic_vector(15 downto 0);
	signal YSIZEsig : std_logic_vector(15 downto 0);


	signal Row_Xmax : std_logic_vector(15 downto 0);-- XSIZE; -- 2000 						-- Row_Xmin + XSIZE - 1; -- 2015  
	signal Row_Xmax_aura: std_logic_vector(15 downto 0);--  XSIZE+1;--Row_Xmin_aura + Xsize_aura - 1; -- 2001
										 -- 2017  
	signal Yoffset: std_logic_vector(15 downto 0);-- XSIZE+2;							--(1 + ((XSIZE) / BurstSize) + 1) * BurstSize; -- 2032 

	signal Ymax: std_logic_vector(15 downto 0);--  YSIZE; -- 2000  
	signal Ymax_aura: std_logic_vector(15 downto 0);-- :=  YSIZE+1; -- 2001  


	constant MEM_END: std_logic_vector(31 downto 0)     := C_MEM_BASEADDR + C_MEM_SIZE;
	constant BlockA: std_logic_vector(31 downto 0) := C_MEM_BASEADDR;									--X"80000000";
	constant BlockB: std_logic_vector(31 downto 0) := C_MEM_BASEADDR + ('0' & C_MEM_SIZE(30 downto 0));	--X"C0000000";


	-- state machines
	type read_col_state_type is 
		(rcClearMemory,rcWaitForSync, rcRow0, rcRow1, rcRow2, rcRow3, rcRowStart, rcRowEnd, rcWaitColumnEnd);

	type read_row_state_type is
		(rrIdle,rrLoadStartBurst,rrPrepareNextBurst,rrLoadNextBurst); 

	type write_col_state_type is 
		(wcIdle, wcWaitForSync, wcRowWait, wcRowStart, wcRowEnd, wcWaitColumnEnd);

	type write_row_state_type is
		(wrIdle,wrLoadStartBurst,wrPrepareNextBurst,wrLoadNextBurst); 

	-- signals
	signal BlockAorB: std_logic;

	
	signal InCol_Xmin: std_logic_vector(15 downto 0);--natural range 0 to Row_Xmax_aura+1;
	signal InCol_Xmax: std_logic_vector(15 downto 0);--natural range 0 to Row_Xmax_aura+1;
	
	signal read_col_state, next_read_col_state, last_read_col_state: read_col_state_type;
	signal read_row_state, next_read_row_state, last_read_row_state: read_row_state_type;
	signal rc_reset_state : read_col_state_type;

	signal in_row_count : std_logic_vector(1 downto 0); 

	signal InReadY : std_logic_vector(15 downto 0);--integer;
	signal InReadX : std_logic_vector(15 downto 0);--integer;

	signal ReadRowStartOffset: std_logic_vector(29 downto 0);
	signal ReadBurstAddr : std_logic_vector(29 downto 0);
	signal NextReadBurstAddr : std_logic_vector(29 downto 0);
	signal ReadBurstLen : std_logic_vector(7 downto 0);


	signal OutCol_Xmin : std_logic_vector(15 downto 0);--natural range 0 to Row_Xmax+1;
	signal OutCol_Xmax : std_logic_vector(15 downto 0);--natural range 0 to Row_Xmax+1;

	signal write_col_state, next_write_col_state, last_write_col_state: write_col_state_type;
	signal write_row_state, next_write_row_state, last_write_row_state: write_row_state_type;

	signal out_row_count : std_logic_vector(1 downto 0); 

	signal OutWriteY :  std_logic_vector(15 downto 0);--natural range 0 to 65535;--Ymax+1;
	signal OutReadX : std_logic_vector(15 downto 0);--natural range 0 to 65535;--Row_Xmax_aura+1;
	signal OutWriteX : std_logic_vector(15 downto 0);--natural range 0 to 65535;--Row_Xmax+1;
	
	signal WriteRowStartOffset: std_logic_vector(29 downto 0);
	signal WriteBurstAddr : std_logic_vector(29 downto 0);
	signal NextWriteBurstAddr : std_logic_vector(29 downto 0);
	signal WriteBurstLen : std_logic_vector(7 downto 0);
	signal wlast_count : std_logic_vector(7 downto 0);
	signal WriteBurst_Active : boolean;

	signal RowsExist : std_logic;

	signal A1,A2,A3 : std_logic_vector(31 downto 0); 
	signal B1,B2,B3 : std_logic_vector(31 downto 0); 
	signal C1,C2,C3 : std_logic_vector(31 downto 0); 
	
--	signal GoL_cellsum : std_logic_vector(2 downto 0);
--	signal GoL_newcell : std_logic;

	-- Event signals
	signal SyncReceived : boolean;
	

	signal RowRead_Finished : boolean;
	signal RowRead_Start : boolean;
	signal ReadWordTransfer : boolean;
	signal ColumnReadEnd : boolean;
	signal Signal_Rows_Readable : boolean;

	signal enter_rcState: boolean;
	signal leave_rcState: boolean;
	signal enter_rrState: boolean;
	signal leave_rrState: boolean;
	
	signal enter_row0to3 : boolean;
	signal go_rcWaitForSync : boolean;
	signal leave_rcWaitColumnEnd : boolean;


	signal RowWrite_Finished : boolean;
	signal RowWrite_Start : boolean;
	signal ColumnWriteEnd : boolean; 
	signal Signal_Rows_Processed : boolean;
	
	signal enter_wcState: boolean;
	signal leave_wcState: boolean;
	signal enter_wrState: boolean;
	signal leave_wrState: boolean;
	
	signal go_wcWaitForSync : boolean;
	signal leave_wcWaitForSync : boolean;
	signal leave_wcWaitColumnEnd : boolean;
	
	signal Signal_CoreDone: std_logic;
	signal Signal_CoreDone_ACK: std_logic;
	signal Signal_ClearDone: std_logic;
	signal Signal_ClearDone_ACK: std_logic;
	signal Signal_SyncDone: std_logic;
	signal Signal_SyncDone_ACK: std_logic;

	-- output signal buffers
	signal bm_axi_arvalid : std_logic;
	signal bm_axi_rready : std_logic;


	--- DEBUG
	
	signal DEBUG_count_events_16b : std_logic_vector(15 downto 0);
	signal DEBUG_count_cycles_32b : std_logic_vector(31 downto 0);
	
	attribute equivalent_register_removal: string;   
	attribute keep:string;

	attribute equivalent_register_removal of DEBUG_count_events_16b : signal is "no";
	attribute keep of DEBUG_count_events_16b :signal is "true";
	attribute equivalent_register_removal of DEBUG_count_cycles_32b : signal is "no";
	attribute keep of DEBUG_count_cycles_32b :signal is "true";	
	
	
--	signal DEBUG_sync_delaycount : integer;
--	constant DEBUG_SYNC_DELAY : integer := 50;
	
	signal DEBUG_WaitSync : std_logic;
	signal DEBUG_SyncReceived : std_logic;

	attribute equivalent_register_removal of DEBUG_WaitSync : signal is "no";
	attribute keep of DEBUG_WaitSync :signal is "true";
	attribute equivalent_register_removal of DEBUG_SyncReceived : signal is "no";
	attribute keep of DEBUG_SyncReceived :signal is "true";	
--	signal DEBUG_FeedCounter : std_logic_vector(31 downto 0);
	signal data_to_pams_read : std_logic;
	signal data_from_pams : std_logic_vector(31 downto 0);
	signal data_to_pams : std_logic_vector(31 downto 0);
	signal data_from_pams_valid : std_logic;

	signal clear_axi_awready            : std_logic;
	signal clear_axi_awvalid            : std_logic;
	signal clear_axi_awaddr             : std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	signal clear_axi_wready             : std_logic;
	signal clear_axi_wvalid             : std_logic;
	signal clear_axi_wlast              : std_logic;
	signal clear_axi_bvalid             : std_logic;

	signal compute_axi_awready            : std_logic;
	signal compute_axi_awvalid            : std_logic;
	signal compute_axi_awaddr             : std_logic_vector(C_M_AXI_ADDR_WIDTH-1 downto 0);
	signal compute_axi_awlen              : std_logic_vector(7 downto 0);
	signal compute_axi_wready             : std_logic;
	signal compute_axi_wvalid             : std_logic;
	signal compute_axi_wdata              : std_logic_vector(C_M_AXI_DATA_WIDTH-1 downto 0);
	signal compute_axi_wstrb              : std_logic_vector((C_M_AXI_DATA_WIDTH/8)-1 downto 0);
	signal compute_axi_wlast              : std_logic;
	signal compute_axi_bvalid             : std_logic;
	
	signal clear_burst_count : std_logic_vector(4 downto 0);
	signal clear_done : std_logic;
	signal LastWriteTransfer : boolean;
	signal WriteTransfer : boolean;
	signal WriteAddrTransfer : boolean;
	signal write_possible : boolean;
	signal StCol1valid : std_logic;
	signal StCol2valid : std_logic;
	signal StCol3valid : std_logic;
	signal JacobiSumValid : std_logic;
	signal JacobiSum : std_logic_vector(31 downto 0);
	
	signal GradientN : std_logic_vector(31 downto 0);
	signal GradientS : std_logic_vector(31 downto 0);
	signal GradientW : std_logic_vector(31 downto 0);
	signal GradientE : std_logic_vector(31 downto 0);
	signal GradientNS : std_logic_vector(31 downto 0);
	signal GradientWE : std_logic_vector(31 downto 0);
	signal MaxLocalGradient : std_logic_vector(31 downto 0);
	signal MaxGradient : std_logic_vector(31 downto 0);
	signal Gradients1valid : std_logic;
	signal Gradients2valid : std_logic;
	signal Gradients3valid : std_logic;
	signal MaxGradientValid : std_logic;
	signal X_stage123, Y_stage123 : std_logic_vector(15 downto 0);
	signal X_stage4,   Y_stage4 : std_logic_vector(15 downto 0);
	signal X_stage5,   Y_stage5 : std_logic_vector(15 downto 0);
	signal X_stage6,   Y_stage6 : std_logic_vector(15 downto 0); 
	signal X_stage7,   Y_stage7 : std_logic_vector(15 downto 0);
	signal X_stage8,   Y_stage8 : std_logic_vector(15 downto 0);
	signal Pole1XY : std_logic_vector(31 downto 0);
	signal Pole1Value : std_logic_vector(31 downto 0);
	signal Pole2XY : std_logic_vector(31 downto 0);
	signal Pole2Value : std_logic_vector(31 downto 0);
	signal Pole3XY : std_logic_vector(31 downto 0);
	signal Pole3Value : std_logic_vector(31 downto 0);
	signal Pole4XY : std_logic_vector(31 downto 0);
	signal Pole4Value : std_logic_vector(31 downto 0);
	signal Pole1Sum : std_logic_vector(31 downto 0);
	signal Pole1SumValid : std_logic;
	signal Pole2Sum : std_logic_vector(31 downto 0);
	signal Pole2SumValid : std_logic;
	signal Pole3Sum : std_logic_vector(31 downto 0);
	signal Pole3SumValid : std_logic;
	signal Pole4Sum : std_logic_vector(31 downto 0);
	signal Pole4SumValid : std_logic;
	signal B2_stage4 : std_logic_vector(31 downto 0);
	signal B2_stage5 : std_logic_vector(31 downto 0);
	signal B2_stage6 : std_logic_vector(31 downto 0);
	signal B2_stage7 : std_logic_vector(31 downto 0);
	signal B2_stage8 : std_logic_vector(31 downto 0);
	signal cell_change : std_logic_vector(31 downto 0);
	signal cell_change_squared : std_logic_vector(63 downto 0);
	signal errormult_in_valid : std_logic;
	signal errormult_out_valid : std_logic;
	signal errormult_latency : std_logic_vector(5 downto 0);
	signal cell_change_squared_acc : std_logic_vector(63 downto 0);
	
	signal data_to_pams_count : std_logic_vector(1 downto 0);
	signal data_from_pams_count : std_logic_vector(3 downto 0);
	
	--PSM etc. signals
	signal pams_ctrl_inputs      : std_logic_vector(3 downto 0) := (others => '0');
	signal pams_ctrl_inputs_ack  : std_logic_vector(3 downto 0) := (others => '0');
	signal pams_ctrl_outputs     : std_logic_vector(15 downto 0) := (others => '0');
	signal pams_ctrl_outputs_ack : std_logic_vector(15 downto 0) := (others => '0');
	signal StencilSum : std_logic_vector(31 downto 0);
	signal leave_rcWaitForSync : boolean;
	
begin

	
	-- Write multiplex
	clear_axi_awready    <= m_axi_awready when (read_col_state= rcClearMemory) else '0';
	compute_axi_awready  <= m_axi_awready when (read_col_state/=rcClearMemory) else '0';
	
	m_axi_awvalid    <= clear_axi_awvalid when (read_col_state= rcClearMemory) else compute_axi_awvalid;
	m_axi_awaddr     <= clear_axi_awaddr  when (read_col_state= rcClearMemory) else compute_axi_awaddr;
	m_axi_awlen      <= "00001111"        when (read_col_state= rcClearMemory) else compute_axi_awlen;
	
	clear_axi_wready    <= m_axi_wready when (read_col_state= rcClearMemory) else '0';
	compute_axi_wready  <= m_axi_wready when (read_col_state/=rcClearMemory) else '0';
	
	m_axi_wvalid <= clear_axi_wvalid  when (read_col_state= rcClearMemory) else compute_axi_wvalid;
	m_axi_wdata  <= X"00000000"       when (read_col_state= rcClearMemory) else compute_axi_wdata;
	m_axi_wstrb  <= "1111"            when (read_col_state= rcClearMemory) else compute_axi_wstrb;
	m_axi_wlast  <= clear_axi_wlast   when (read_col_state= rcClearMemory) else compute_axi_wlast;
	
	clear_axi_bvalid    <= m_axi_bvalid when (read_col_state= rcClearMemory) else '0';
	compute_axi_bvalid  <= m_axi_bvalid when (read_col_state/=rcClearMemory) else '0';
	
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				clear_axi_awaddr <= C_MEM_BASEADDR;
			elsif (clear_axi_awvalid='1' and clear_axi_awready='1') then
				clear_axi_awaddr <= clear_axi_awaddr + 64;
			end if;
		end if;
	end process;
	clear_axi_awvalid <= '1' when ((read_col_state=rcClearMemory) and (clear_done='0') and (clear_burst_count<=1)) else '0';
	
	
	gen_clearmem: if (C_CLEAR_MEM=1) generate
	begin
		rc_reset_state <= rcClearMemory;
		clear_done <= '1' when ((clear_axi_awaddr=MEM_END) and (clear_axi_wlast='1')) else '0';

		process(clock)
		begin
			if (rising_edge(clock)) then
				if (reset='1') then
					Signal_ClearDone <= '0';
				elsif (clear_done='1') then
					Signal_ClearDone <= '1';
				elsif (Signal_ClearDone_ACK='1') then
					Signal_ClearDone <= '0';
				end if;
			end if;
		end process;
	end generate gen_clearmem;
	--
	gen_noclearmem: if (C_CLEAR_MEM/=1) generate
	begin 
		rc_reset_state <= rcWaitForSync;
		clear_done <= '1';
		
		process(clock)
		begin
			if (rising_edge(clock)) then
				if (reset='1') then
					Signal_ClearDone <= '1';
				elsif (Signal_ClearDone_ACK='1') then
					Signal_ClearDone <= '0';
				end if;
			end if;
		end process;
	end generate gen_noclearmem;
	
	
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				clear_burst_count <= (others => '0');
			elsif (clear_axi_awvalid='1' and clear_axi_awready='1') then
				clear_burst_count <= "10000";
			elsif (clear_axi_wvalid='1' and clear_axi_wready='1') then
				clear_burst_count <= clear_burst_count - 1;
			end if;
		end if;
	end process;
	clear_axi_wvalid <= '1' when (clear_burst_count/=0) else '0';
	clear_axi_wlast  <= '1' when (clear_burst_count="00001") else '0';
	
	
	
	-- DEBUG START
		process(clock)
		begin
			if (rising_edge(clock)) then
				if (reset='1') then
					DEBUG_count_events_16b <= (others => '0');
				elsif (go_rcWaitForSync) then
					DEBUG_count_events_16b <= DEBUG_count_events_16b + 1; 
				end if;
			end if;
		end process;
	
		process(clock)
		begin
			if (rising_edge(clock)) then
				if (reset='1') then
					DEBUG_count_cycles_32b <= (others => '0');
				else
					DEBUG_count_cycles_32b <= DEBUG_count_cycles_32b + 1; 
				end if;
			end if;
		end process;
		
		DEBUG_SyncReceived <= '1' when (SyncReceived) else '0';
		DEBUG_WaitSync <= '1' when (go_rcWaitForSync) else '0';
		
	-- DEBUG END
	
	
	clock <= HOST_Clk;
	reset <= HOST_Rst;
	
	md_error <= '0';

	m_axi_arsize <= "010"; -- 4 word burst data width
    m_axi_arburst <= "01"; -- Increment address - only supported mode
    m_axi_arprot <= "000"; -- always 000
    m_axi_arcache <= "0011"; -- always 0000
	m_axi_awsize <= "010"; -- 4 word burst data width
    m_axi_awburst <= "01"; -- Increment address - only supported mode
    m_axi_awprot <= "000"; -- always 000
    m_axi_awcache <= "0011"; -- always 0000


	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset = '1') then
				BlockAorB <= '1';
			elsif (go_rcWaitForSync) then
				BlockAorB <= not BlockAorB;
			end if;
		end if;
	end process;


-- INPUT
	m_axi_arvalid <= bm_axi_arvalid;
	m_axi_araddr <= (((ReadBurstAddr+ReadRowStartOffset) & "00")+BlockA) when (BlockAorB='0')
					else (((ReadBurstAddr+ReadRowStartOffset) & "00")+BlockB);
	m_axi_arlen <= ReadBurstLen;
	
	m_axi_rready <= bm_axi_rready;
	bm_axi_rready <= not InFull; -- TOD plus state condition?
	DataIn <= m_axi_rdata;
	InWE <= '1' when ReadWordTransfer else '0'; -- TOD plus state condition?

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or ColumnReadEnd) then
				ReadRowStartOffset <= (others => '0');
			elsif (RowRead_Finished) then
				ReadRowStartOffset <= ReadRowStartOffset + Yoffset; 
			end if;
		end if;
	end process;

	-- processing by column
	readcol:process(reset,read_col_state, InReadY, RowRead_Finished, SyncReceived, RowRead_Start, ColumnWriteEnd, InCol_Xmax, clear_done, rc_reset_state, Row_Xmax_aura, Ymax_aura)
		variable next_state: read_col_state_type; 
	begin
		if (reset='1') then
			next_state := rc_reset_state;	--rcClearMemory,rcWaitForSync;
		else
			next_state := read_col_state;
			
			case read_col_state is
			when rcClearMemory =>
					if (clear_done='1') then
						next_state := rcWaitForSync; end if;
				when rcWaitForSync =>
					if (SyncReceived) then
						next_state := rcRow0; end if; 
				when rcRow0 =>
					if (RowRead_Finished) then
						next_state := rcRow1; end if; 
				when rcRow1 =>
					if (RowRead_Finished) then
						next_state := rcRow2; end if;
				when rcRow2 =>
					if (RowRead_Finished) then
						next_state := rcRow3; end if; 
				when rcRow3 =>
					if (RowRead_Finished) then
						next_state := rcRowStart; end if; 
				when rcRowStart =>
					if (RowRead_Start) then
						next_state := rcRowEnd; end if; 
				when rcRowEnd =>
					if (RowRead_Finished) then
						if (InReadY=Ymax_aura) then
							next_state := rcWaitColumnEnd;
						else
							next_state := rcRowStart; end if;
					end if; 
				when rcWaitColumnEnd =>
					if (ColumnWriteEnd) then
						if (InCol_Xmax=Row_Xmax_aura) then
							next_state := rcWaitForSync; 
						else
							next_state := rcRow0;
						end if;
					end if;
				--when others => 
					--next_state := read_col_state; 
			end case;
		end if;		
		next_read_col_state <= next_state;
	end process;
	read_col_state <= next_read_col_state when rising_edge(clock) else read_col_state;
	last_read_col_state <= read_col_state when rising_edge(clock) else last_read_col_state;

	enter_rcState <= (read_col_state/=last_read_col_state);
	leave_rcState <= (read_col_state/=next_read_col_state);
	enter_rrState <= (read_row_state/=last_read_row_state);
	leave_rrState <= (read_row_state/=next_read_row_state);

	go_rcWaitForSync <= leave_rcState and next_read_col_state=rcWaitForSync;
	leave_rcWaitForSync <= leave_rcState and read_col_state=rcWaitForSync;
	leave_rcWaitColumnEnd <= read_col_state=rcWaitColumnEnd and leave_rcState;
	enter_row0to3 <= enter_rcState and (
				(read_col_state=rcRow0) or (read_col_state=rcRow1) or (read_col_state=rcRow2) or (read_col_state=rcRow3) 
				);
				
	ReadWordTransfer <= m_axi_rvalid='1' and bm_axi_rready='1';

	RowRead_Start <= true when (enter_row0to3)
						else true when (Signal_Rows_Processed and read_col_state=rcRowStart)
						else false;
	RowRead_Finished <= (InReadX=InCol_Xmax) and ReadWordTransfer;
	ColumnReadEnd <= (InReadY=Ymax_aura) and RowRead_Finished;

	--load input column parameters
	process(clock)
	begin
		if (rising_edge(clock)) then
			--if (reset='1') then
			--	InCol_Xmin <= 0;
			--	InCol_Xmax <= 0;
			if (reset='1' or leave_rcWaitForSync) then
				InCol_Xmin <= Row_Xmin_aura;
				if (Row_Xmin_aura+RowBufferDepth-1  <  Row_Xmax_aura) then
					InCol_Xmax <= Row_Xmin_aura+RowBufferDepth-1;
				else
					InCol_Xmax <= Row_Xmax_aura;
				end if;
			elsif (leave_rcWaitColumnEnd) then
				InCol_Xmin <= InCol_Xmax-1;
				if (InCol_Xmax+RowBufferDepth-2  <  Row_Xmax_aura) then
					InCol_Xmax <= InCol_Xmax+RowBufferDepth-2;
				else
					InCol_Xmax <= Row_Xmax_aura;
				end if;
			end if;
		end if;
	end process;


	-- Y row position
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or go_rcWaitForSync or leave_rcWaitColumnEnd) then
				InReadY <= Ymin_aura;
				in_row_count <= "00";
			elsif (RowRead_Finished) then
				InReadY <= InReadY + 1;
				in_row_count <= in_row_count + 1;
			end if;
		end if;
	end process;


	-- X position in column row
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or RowRead_Start) then
				InReadX <= InCol_Xmin;
			elsif (ReadWordTransfer) then
				InReadX <= InReadX + 1; 
			end if;
		end if;
	end process;

	-- Read X burst information
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				ReadBurstAddr <= (others => '0');
				ReadBurstLen <=  (others => '0');
				bm_axi_arvalid <= '0';
				read_row_state <= rrIdle;
			elsif (RowRead_Start) then -- RowStart
				ReadBurstAddr <= "00" & X"000" & InCol_Xmin;
				ReadBurstLen <=  "0000" & burstlen("00" & X"000" & InCol_Xmin,"00" & X"000" & InCol_Xmax); 
				bm_axi_arvalid <= '1';
				read_row_state <= rrLoadStartBurst;
			elsif ((read_row_state=rrLoadStartBurst or read_row_state=rrLoadNextBurst) and m_axi_arready='1') then
				NextReadBurstAddr <= (ReadBurstAddr(29 downto 4) + 1) & "0000"; 
				bm_axi_arvalid <= '0';
				read_row_state <= rrPrepareNextBurst;
			elsif (read_row_state=rrPrepareNextBurst) then
				ReadBurstAddr <= NextReadBurstAddr;
				ReadBurstLen <=  "0000" & burstlen(NextReadBurstAddr,"00" & X"000" & InCol_Xmax); 
				if (NextReadBurstAddr <= "00" & X"000" & InCol_Xmax) then 
					bm_axi_arvalid <= '1';
					read_row_state <= rrLoadNextBurst;
				else
					bm_axi_arvalid <= '0';
					read_row_state <= rrIdle;
				end if;
			end if;
		end if;
	end process;
	
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Signal_Rows_Readable <= false;
			elsif (RowRead_Finished and InReadY>=2) then
				Signal_Rows_Readable <= true;
			elsif (RowWrite_Start) then
				Signal_Rows_Readable <= false; 
			end if;
		end if;
	end process;

	ResetBuffer0 <= Reset or Reset_Write0;
	ResetBuffer1 <= Reset or Reset_Write1;
	ResetBuffer2 <= Reset or Reset_Write2;
	ResetBuffer3 <= Reset or Reset_Write3;

	Reset_Write0 <= '1' when 
					(((in_row_count="00") and (read_col_state=rcRowStart and RowRead_Start)) or ColumnWriteEnd) else '0';
	Reset_Write1 <= '1' when 
					(((in_row_count="01") and (read_col_state=rcRowStart and RowRead_Start)) or ColumnWriteEnd) else '0';
	Reset_Write2 <= '1' when 
					(((in_row_count="10") and (read_col_state=rcRowStart and RowRead_Start)) or ColumnWriteEnd) else '0';
	Reset_Write3 <= '1' when 
					(((in_row_count="11") and (read_col_state=rcRowStart and RowRead_Start)) or ColumnWriteEnd) else '0';

	-- buffer selection
	WE0 <= InWE when (in_row_count="00") else '0';
	WE1 <= InWE when (in_row_count="01") else '0';
	WE2 <= InWE when (in_row_count="10") else '0';
	WE3 <= InWE when (in_row_count="11") else '0';
	
	with in_row_count select
	InFull <= 	Full0 when "00", 
				Full1 when "01",
				Full2 when "10",
				Full3 when others;

	
-- BUFFERS
								
	buf0: Sync_FIFO_BRAM_LARGE
		generic map(WordWidth => 32,
			        BRAMs     => C_BRAMS_PER_ROW)
		port map(Reset      => ResetBuffer0,
			     Clk        => clock,
			     WE         => WE0,
			     DataIn     => DataIn,
			     Full       => Full0,
			     Reset_Read => Reset_Read0,
			     RD         => Read0,
			     DataOut    => DataOut0,
			     Exists     => Exists0);
			  
	buf1: Sync_FIFO_BRAM_LARGE
		generic map(WordWidth => 32,
			        BRAMs     => C_BRAMS_PER_ROW)
		port map(Reset      => ResetBuffer1,
			     Clk        => clock,
			     WE         => WE1,
			     DataIn     => DataIn,
			     Full       => Full1,
			     Reset_Read => Reset_Read1,
			     RD         => Read1,
			     DataOut    => DataOut1,
			     Exists     => Exists1);

	buf2: Sync_FIFO_BRAM_LARGE
		generic map(WordWidth => 32,
			        BRAMs     => C_BRAMS_PER_ROW)
		port map(Reset      => ResetBuffer2,
			     Clk        => clock,
			     WE         => WE2,
			     DataIn     => DataIn,
			     Full       => Full2,
			     Reset_Read => Reset_Read2,
			     RD         => Read2,
			     DataOut    => DataOut2,
			     Exists     => Exists2);
			     
	buf3: Sync_FIFO_BRAM_LARGE
		generic map(WordWidth => 32,
			        BRAMs     => C_BRAMS_PER_ROW)
		port map(Reset      => ResetBuffer3,
			     Clk        => clock,
			     WE         => WE3,
			     DataIn     => DataIn,
			     Full       => Full3,
			     Reset_Read => Reset_Read3,
			     RD         => Read3,
			     DataOut    => DataOut3,
			     Exists     => Exists3);



-- OUTPUT

	compute_axi_awaddr <= (((WriteBurstAddr+WriteRowStartOffset) & "00")+BlockA) when (BlockAorB='1')
						else (((WriteBurstAddr+WriteRowStartOffset) & "00")+BlockB);
	compute_axi_awlen <= WriteBurstLen;

	compute_axi_wstrb <= "1111";
	
	m_axi_bready <= '1';
	

	with out_row_count select
	Reset_Read0 <= 	Reset_ReadA when "00", Reset_ReadB when "11", Reset_ReadC when "10", '0' when others;

	with out_row_count select
	Reset_Read1 <= 	Reset_ReadA when "01", Reset_ReadB when "00", Reset_ReadC when "11", '0' when others;

	with out_row_count select
	Reset_Read2 <= 	Reset_ReadA when "10", Reset_ReadB when "01", Reset_ReadC when "00", '0' when others;

	with out_row_count select
	Reset_Read3 <= 	Reset_ReadA when "11", Reset_ReadB when "10", Reset_ReadC when "01", '0' when others;



--	with out_row_count select	Read0 <= 	ReadRows when "00", ReadRows when "11", ReadRows when "10", '0' when others;
	Read0 <= '0' when (out_row_count="01") else ReadRows;

--	with out_row_count select	Read1 <= 	ReadRows when "01", ReadRows when "00", ReadRows when "11", '0' when others;
	Read1 <= '0' when (out_row_count="10") else ReadRows;

--	with out_row_count select	Read2 <= 	ReadRows when "10", ReadRows when "01", ReadRows when "00", '0' when others;
	Read2 <= '0' when (out_row_count="11") else ReadRows;

--	with out_row_count select	Read3 <= 	ReadRows when "11", ReadRows when "10", ReadRows when "01", '0' when others;
	Read3 <= '0' when (out_row_count="00") else ReadRows;



	with out_row_count select
	ExistsA	<=	Exists0 when "00", Exists1 when "01", Exists2 when "10", Exists3 when others; --11

	with out_row_count select
	ExistsB	<=	Exists1 when "00", Exists2 when "01", Exists3 when "10", Exists0 when others; --11

	with out_row_count select
	ExistsC	<=	Exists2 when "00", Exists3 when "01", Exists0 when "10", Exists1 when others; --11



	with out_row_count select
	DataOutA <=	DataOut0 when "00",	DataOut1 when "01", DataOut2 when "10", DataOut3 when others; --11

	with out_row_count select
	DataOutB <=	DataOut1 when "00", DataOut2 when "01",	DataOut3 when "10",	DataOut0 when others; --11

	with out_row_count select
	DataOutC <=	DataOut2 when "00",	DataOut3 when "01",	DataOut0 when "10",	DataOut1 when others; --11


	-- Output column state
	writecol:process(reset, OutCol_Xmax, OutWriteY, RowWrite_Finished, SyncReceived, write_col_state, InCol_Xmin, OutReadX, ReadRows, Signal_Rows_Readable, clear_done, Row_Xmax, Ymax)
		variable next_state: write_col_state_type; 
	begin
		if (reset='1') then
			next_state := wcIdle;
		else
			next_state := write_col_state;
			
			case write_col_state is
			when wcIdle =>
					if (clear_done='1') then
						next_state := wcWaitForSync; end if; 
				when wcWaitForSync =>
					if (SyncReceived) then
						next_state := wcRowWait; end if; 
				when wcRowWait =>
					if (Signal_Rows_Readable) then
						next_state := wcRowStart; end if; 
				when wcRowStart =>
					if (OutReadX=InCol_XMin+2 and ReadRows='1') then
						next_state := wcRowEnd; end if; 
				when wcRowEnd =>
					if (RowWrite_Finished) then
						if (OutWriteY=Ymax) then
							next_state := wcWaitColumnEnd;
						else
							next_state := wcRowWait; end if;
					end if; 
				when wcWaitColumnEnd =>
					--if (ColumnWriteEnd) then
						if (OutCol_Xmax=Row_Xmax) then
							next_state := wcWaitForSync; 
						else
							next_state := wcRowWait;
						end if;
					--end if;
--				when others => 
--					next_state := write_col_state; 
			end case;
		end if;		
		next_write_col_state <= next_state;
	end process;
	write_col_state <= next_write_col_state when rising_edge(clock) else write_col_state;
	last_write_col_state <= write_col_state when rising_edge(clock) else last_write_col_state;

	enter_wcState <= (write_col_state/=last_write_col_state);
	leave_wcState <= (write_col_state/=next_write_col_state);
	enter_wrState <= (write_row_state/=last_write_row_state);
	leave_wrState <= (write_row_state/=next_write_row_state);

	go_wcWaitForSync <= leave_wcState and next_write_col_state=wcWaitForSync;
	leave_wcWaitForSync <= leave_wcState and write_col_state=wcWaitForSync;
	leave_wcWaitColumnEnd <= write_col_state=wcWaitColumnEnd and leave_wcState;  

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or ColumnWriteEnd) then
				WriteRowStartOffset <= "00" & X"000" & Yoffset;
			elsif (RowWrite_Finished) then
				WriteRowStartOffset <= WriteRowStartOffset + Yoffset; 
			end if;
		end if;
	end process;

	--load output column parameters
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or leave_wcWaitForSync) then
				OutCol_Xmin <= Row_Xmin;
				if (Row_Xmin+RowBufferDepth-3  <  Row_Xmax) then
					OutCol_Xmax <= Row_Xmin+RowBufferDepth-3;
				else
					OutCol_Xmax <= Row_Xmax;
				end if;
			elsif (leave_wcWaitColumnEnd) then
				OutCol_Xmin <= OutCol_Xmax+1;
				if (OutCol_Xmax+1+RowBufferDepth-3  <  Row_Xmax) then
					OutCol_Xmax <= OutCol_Xmax+1+RowBufferDepth-3;
				else
					OutCol_Xmax <= Row_Xmax;
				end if;
			end if;
		end if;
	end process;


	ColumnWriteEnd <= RowWrite_Finished when (OutWriteY=Ymax) else false;
	RowWrite_Start <= Signal_Rows_Readable and write_col_state=wcRowWait;
	RowWrite_Finished <= OutWriteX=OutCol_Xmax and WriteTransfer;

	-- Y row position
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or ColumnWriteEnd) then
				OutWriteY <= Ymin;
				out_row_count <= "00";
			elsif (RowWrite_Finished) then
				OutWriteY <= OutWriteY + 1;
				out_row_count <= out_row_count + 1;
			end if;
		end if;
	end process;

	-- X position in column row
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or RowWrite_Start) then
				OutReadX <= InCol_Xmin;
				OutWriteX <= OutCol_Xmin;
			else
				if (ReadRows='1') then 
					OutReadX <= OutReadX + 1;
				end if;
				if (WriteTransfer) then
					OutWriteX <= OutWriteX + 1;
				end if;
			end if;
		end if;
	end process;


	-- Write X burst information
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				WriteBurstAddr <= (others => '0');
				WriteBurstLen <=  (others => '0');
				compute_axi_awvalid <= '0';
				write_row_state <= wrIdle;
			elsif (Rowwrite_Start) then -- RowStart
				WriteBurstAddr <= "00" & X"000" & OutCol_Xmin;
				WriteBurstLen <=  "0000" & burstlen("00" & X"000" & OutCol_Xmin,"00" & X"000" & OutCol_Xmax); 
				compute_axi_awvalid <= '1';
				write_row_state <= wrLoadStartBurst;
			elsif ((write_row_state=wrLoadStartBurst or write_row_state=wrLoadNextBurst) and compute_axi_awready='1') then
				NextWriteBurstAddr <= (WriteBurstAddr(29 downto 4) + 1) & "0000"; 
				compute_axi_awvalid <= '0';
				write_row_state <= wrPrepareNextBurst;
			elsif (write_row_state=wrPrepareNextBurst and ((LastWriteTransfer) or WriteBurst_Active=false)) then
				WriteBurstAddr <= NextWriteBurstAddr;
				WriteBurstLen <=  "0000" & burstlen(NextWriteBurstAddr,"00" & X"000" & OutCol_Xmax); 
				if (NextWriteBurstAddr <= "00" & X"000" & OutCol_Xmax) then 
					compute_axi_awvalid <= '1';
					write_row_state <= wrLoadNextBurst;
				else
					compute_axi_awvalid <= '0';
					write_row_state <= wrIdle;
				end if;
			end if;
		end if;
	end process;


	-- wlast count
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				wlast_count <= X"00";
			elsif (WriteAddrTransfer) then
				wlast_count <= WriteBurstLen + 1;
			elsif (WriteTransfer and wlast_count/=0) then
				wlast_count <= wlast_count - 1; 
			end if;
		end if;
	end process;
	compute_axi_wlast <= '1' when wlast_count=1 else '0';

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				WriteBurst_Active <= false;
			elsif (WriteAddrTransfer) then
				WriteBurst_Active <= true;
			elsif (LastWriteTransfer) then
				WriteBurst_Active <= false; 
			end if;
		end if;
	end process;
	LastWriteTransfer <= true when (compute_axi_wvalid='1' and compute_axi_wready='1' and compute_axi_wlast='1') else false;
	WriteTransfer     <= true when (compute_axi_wvalid='1' and compute_axi_wready='1') else false;
	WriteAddrTransfer <= true when (compute_axi_awvalid='1' and compute_axi_awready='1') else false;

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Signal_Rows_Processed <= false;
			elsif (RowWrite_Finished) then
				Signal_Rows_Processed <= true;
			elsif (RowRead_Start or read_col_state=rcWaitColumnEnd or read_col_state=rcWaitForSync) then
				Signal_Rows_Processed <= false; 
			end if;
		end if;
	end process;

	RowsExist <= ExistsA and ExistsB and ExistsC;

	ReadRows <= '1' when (RowsExist='1' and write_possible) else '0';  
	--((write_col_state=wcRowStart) or (write_col_state=wcRowEnd)) else '0';
	--and WriteTransfer)) else '0';

	Reset_ReadA <= '1' when RowWrite_Start else '0';
	Reset_ReadB <= Reset_ReadA;
	Reset_ReadC <= Reset_ReadA;


	compute_axi_wvalid <= '1' when (Pole4SumValid='1' and WriteBurst_Active) else '0';
	write_possible <= true when (WriteBurst_Active and compute_axi_wready='1') else false;


	-- STENCIL DATA
	process(clock) -- stage 1,2,3
	begin
		if (rising_edge(clock)) then
			if (reset='1' or RowWrite_Finished) then
				A1 <= (others => '0'); A2 <= (others => '0'); A3 <= (others => '0');
				B1 <= (others => '0'); B2 <= (others => '0'); B3 <= (others => '0');
				C1 <= (others => '0'); C2 <= (others => '0'); C3 <= (others => '0');
				StCol1valid <= '0';
				StCol2valid <= '0';
				StCol3valid <= '0';
			elsif (write_possible) then --ReadRows='1' or WriteTransfer) then
				A1 <= DataOutA;  A2 <= A1;  A3 <= A2;
				B1 <= DataOutB;  B2 <= B1;  B3 <= B2;
				C1 <= DataOutC;  C2 <= C1;  C3 <= C2;
				StCol1valid <= RowsExist;
				StCol2valid <= StCol1valid;
				StCol3valid <= StCol2valid;
			end if;
		end if;
	end process;

	-- WRITE COORDINATES
	process(clock) -- stage 1,2,3
	begin
		if (rising_edge(clock)) then
			if (reset='1' or RowWrite_Finished) then
				X_stage123 <= (others => '0');
			elsif (write_possible and StCol1valid='1' and StCol3valid='1') then
				X_stage123 <= X_stage123 + 1;
			end if;
			-------
			if (reset='1' or leave_wcWaitForSync) then
				Y_stage123 <= (others => '0');
			elsif (RowWrite_Finished) then
				Y_stage123 <= Y_stage123 + 1;
			end if;
		end if;
	end process;

	-- STENCIL OPERATION
	process(clock) -- stage 4
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				JacobiSum <= (others => '0');
				JacobiSumValid <= '0';
				--
				B2_stage4 <= (others => '0');
				X_stage4 <= (others => '0');
				Y_stage4 <= (others => '0');
			elsif (write_possible) then
				JacobiSum <= StencilSum(31) & StencilSum(31) & StencilSum(31 downto 2); -- div by 4, sign-extended
				JacobiSumValid <= StCol1valid and StCol3valid;
				--
				B2_stage4 <= B2;
				X_stage4 <= X_stage123;
				Y_stage4 <= Y_stage123;
			end if;
		end if;
	end process;
	StencilSum <= A2+B1+B3+C2;

	-- POLE CHECKING
	process(clock) -- stage 5 - pole 1 comparison
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Pole1Sum <= (others => '0');
				Pole1SumValid <= '0';
				-- 
				B2_stage5 <= (others => '0');
				X_stage5 <= (others => '0');
				Y_stage5 <= (others => '0');
			elsif (write_possible) then
				if (Pole1XY=(X_stage4 & Y_stage4)) then
					Pole1Sum <= JacobiSum + Pole1Value; -- check math again: substitute or add???
				else
					Pole1Sum <= JacobiSum;	
				end if;
				Pole1SumValid <= JacobiSumValid;
				-- 
				B2_stage5 <= B2_stage4;
				X_stage5 <= X_stage4;
				Y_stage5 <= Y_stage4;
			end if;
		end if;
	end process;

	process(clock) -- stage 6 - pole 2 comparison
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Pole2Sum <= (others => '0');
				Pole2SumValid <= '0';
				-- 
				B2_stage6 <= (others => '0');
				X_stage6 <= (others => '0');
				Y_stage6 <= (others => '0');
			elsif (write_possible) then
				if (Pole2XY=(X_stage5 & Y_stage5)) then
					Pole2Sum <= Pole1Sum + Pole2Value; -- check math again: substitute or add???
				else
					Pole2Sum <= Pole1Sum;	
				end if;
				Pole2SumValid <= Pole1SumValid;
				-- 
				B2_stage6 <= B2_stage5;
				X_stage6 <= X_stage5;
				Y_stage6 <= Y_stage5;
			end if;
		end if;
	end process;

	process(clock) -- stage 7 - pole 3 comparison
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Pole3Sum <= (others => '0');
				Pole3SumValid <= '0';
				-- 
				B2_stage7 <= (others => '0');
				X_stage7 <= (others => '0');
				Y_stage7 <= (others => '0');
			elsif (write_possible) then
				if (Pole3XY=(X_stage6 & Y_stage6)) then
					Pole3Sum <= Pole2Sum + Pole3Value; -- check math again: substitute or add???
				else
					Pole3Sum <= Pole2Sum;	
				end if;
				Pole3SumValid <= Pole2SumValid;
				-- 
				B2_stage7 <= B2_stage6;
				X_stage7 <= X_stage6;
				Y_stage7 <= Y_stage6;
			end if;
		end if;
	end process;

	process(clock) -- stage 8 - pole 4 comparison
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Pole4Sum <= (others => '0');
				Pole4SumValid <= '0';
				-- 
				B2_stage8 <= (others => '0');
				X_stage8 <= (others => '0');
				Y_stage8 <= (others => '0');
			elsif (write_possible) then
				if (Pole4XY=(X_stage7 & Y_stage7)) then
					Pole4Sum <= Pole3Sum + Pole4Value; -- check math again: substitute or add???
				else
					Pole4Sum <= Pole3Sum;	
				end if;
				Pole4SumValid <= Pole3SumValid;
				-- 
				B2_stage8 <= B2_stage7;
				X_stage8 <= X_stage7;
				Y_stage8 <= Y_stage7;
			end if;
		end if;
	end process;

	compute_axi_wdata <= Pole4Sum; 
	
	-- ERROR ACCUMULATION
	cell_change <= B2_stage8 - Pole4Sum;
		
	errormult : multiplier
	port map (
		clk  => clock,
		a    => cell_change,
		b    => cell_change,
		ce   => '1',--ce,
		sclr => reset,--sclr,
		p    => cell_change_squared
		);
	
	errormult_in_valid  <= '1' when WriteTransfer else '0';
	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				errormult_latency <= (others => '0');
			else
				errormult_latency <= errormult_latency(4 downto 0) & errormult_in_valid;
			end if;
		end if;
	end process;	
	errormult_out_valid <= errormult_latency(5);

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1' or leave_wcWaitForSync) then
				cell_change_squared_acc <= (others => '0');
			elsif (errormult_out_valid='1') then
				cell_change_squared_acc <= cell_change_squared_acc + cell_change_squared; 
			end if;
		end if;
	end process;	

	-- GRADIENT
	process(clock) -- stage 4
	begin
		if (rising_edge(clock)) then
			if (reset='1' or leave_wcWaitForSync) then -- clear every iteration
				GradientN <= (others => '0'); GradientS <= (others => '0');
				GradientW <= (others => '0'); GradientE <= (others => '0');
				Gradients1valid <= '0';
			elsif (write_possible) then -- TODO: do earlier in read?
				GradientN <= abs_diff(A2,B2); GradientE <= abs_diff(B1,B2);
				GradientW <= abs_diff(B3,B2); GradientS <= abs_diff(C2,B2);
				Gradients1valid <= StCol1valid and StCol3valid;
			end if;
		end if;
	end process;

	process(clock) -- stage 5							-- TODO: VALID SIGNALS
	begin
		if (rising_edge(clock)) then
			if (reset='1' or leave_wcWaitForSync) then -- clear every iteration
				GradientNS <= (others => '0'); GradientWE <= (others => '0');
				Gradients2valid <= '0';
			elsif (write_possible) then
				GradientNS <= pick_larger(GradientN,GradientS); GradientWE <= pick_larger(GradientW,GradientE);
				Gradients2valid <= Gradients1valid;
			end if;
		end if;
	end process;

	process(clock) -- stage 6
	begin
		if (rising_edge(clock)) then
			if (reset='1' or leave_wcWaitForSync) then -- clear every iteration
				MaxLocalGradient <= (others => '0');
				Gradients3valid <= '0';
			elsif (write_possible) then
				MaxLocalGradient <= pick_larger(GradientNS,GradientWE);
				Gradients3valid <= Gradients2valid;
			end if;
		end if;
	end process;

	process(clock) -- stage 7
	begin
		if (rising_edge(clock)) then
			if (reset='1' or leave_wcWaitForSync) then -- clear every iteration
				MaxGradient <= (others => '0');
				MaxGradientValid <= '0';
			elsif (write_possible) then
				MaxGradient <= pick_larger(MaxGradient,MaxLocalGradient);
				MaxGradientValid <= Gradients3valid;
			end if;
		end if;
	end process;


	-- STENCIL FUNCTION (Game of Life)
--	GoL_cellsum <= 	std_logic_vector(
--					unsigned("00"& A1(0 downto 0)) + unsigned("00"& A2(0 downto 0)) + unsigned("00"& A3(0 downto 0)) +  
--					unsigned("00"& B1(0 downto 0)) +                                  unsigned("00"& B3(0 downto 0)) +  
--					unsigned("00"& C1(0 downto 0)) + unsigned("00"& C2(0 downto 0)) + unsigned("00"& C3(0 downto 0)));
--					-- if sum is 8, 3-bit vector overflows to 0 -> both have the same effect to GoL
--					
--	GoL_newcell <= '1' when ((GoL_cellsum="011") or (GoL_cellsum="010" and B2(0)='1')) else '0';
	
--- SYNC

	SyncReceived <= Signal_SyncDone='1';
	Signal_SyncDone_ACK <= Signal_SyncDone;

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Signal_CoreDone <= '0';
			elsif (go_rcWaitForSync) then
				Signal_CoreDone <= '1';
			elsif (Signal_CoreDone_ACK='1') then
				Signal_CoreDone <= '0';
			end if;
		end if;
	end process;

--- PSM etc.

	with data_to_pams_count select
	data_to_pams <=	cell_change_squared_acc(31 downto 0) when "00",
					cell_change_squared_acc(63 downto 32) when "01",
					MaxGradient when others;


	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				data_to_pams_count <= "00";
			elsif (data_to_pams_read='1') then
				if (data_to_pams_count="10") then
					data_to_pams_count <= "00";
				else
					data_to_pams_count <= data_to_pams_count + 1;
				end if;
			end if;
		end if;
	end process;


	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				data_from_pams_count <= "0000";
			elsif (data_from_pams_valid='1') then
				if (data_from_pams_count="1001") then
					data_from_pams_count <= "0000";
				else
					data_from_pams_count <= data_from_pams_count + 1;
				end if;
			end if;
		end if;
	end process;


	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
					XSIZEsig   <= (others => '0');--CONV_STD_LOGIC_VECTOR(XSIZE,16);
					YSIZEsig   <= (others => '0');--CONV_STD_LOGIC_VECTOR(YSIZE,16);
					Pole1XY    <= (others => '1');
					Pole1Value <= (others => '0');
					Pole2XY    <= (others => '1');
					Pole2Value <= (others => '0');
					Pole3XY    <= (others => '1');
					Pole3Value <= (others => '0');
					Pole4XY    <= (others => '1');
					Pole4Value <= (others => '0');
			elsif (data_from_pams_valid='1') then
				if (data_from_pams_count="0000") then
					XSIZEsig <= data_from_pams(15 downto 0); end if;
				if (data_from_pams_count="0001") then
					YSIZEsig <= data_from_pams(15 downto 0); end if;
				if (data_from_pams_count="0010") then
					Pole1XY <= data_from_pams; end if;
				if (data_from_pams_count="0011") then
					Pole1Value <= data_from_pams; end if;
				if (data_from_pams_count="0100") then
					Pole2XY <= data_from_pams; end if;
				if (data_from_pams_count="0101") then
					Pole2Value <= data_from_pams; end if;
				if (data_from_pams_count="0110") then
					Pole3XY <= data_from_pams; end if;
				if (data_from_pams_count="0111") then
					Pole3Value <= data_from_pams; end if;
				if (data_from_pams_count="1000") then
					Pole4XY <= data_from_pams; end if;
				if (data_from_pams_count="1001") then
					Pole4Value <= data_from_pams; end if;
			end if;
		end if;
	end process;

	process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then
				Row_Xmax      <= X"0000";--CONV_STD_LOGIC_VECTOR(XSIZE,16);
				Row_Xmax_aura <= X"0001";---CONV_STD_LOGIC_VECTOR(XSIZE+1,16);
				Yoffset       <= X"0002";---CONV_STD_LOGIC_VECTOR(XSIZE+2,16);
				Ymax          <= X"0000";---CONV_STD_LOGIC_VECTOR(YSIZE,16);
				Ymax_aura     <= X"0001";---CONV_STD_LOGIC_VECTOR(YSIZE+1,16);
			else
				Row_Xmax      <= XSIZEsig;
				Row_Xmax_aura <= XSIZEsig + 1;
				Yoffset       <= XSIZEsig + 2;
				Ymax          <= YSIZEsig;
				Ymax_aura     <= YSIZEsig + 1;
			end if;
		end if;
	end process;

--process(clock) -- DEBUG ONLY
--	begin
--		if (rising_edge(clock)) then
--			if (reset='1') then
--				data_to_pams <= X"BEEF0001";
--			elsif (data_to_pams_read='1') then
--				data_to_pams <= data_to_pams + 1;
--			end if;
--		end if;
--end process;

	Inst_psm: psm
	generic map
	(
		ctrl_inputs => 4,--pams_ctrl_inputs,
		ctrl_outputs => 16--pams_ctrl_outputs
	)
	port map
	(
		clock => clock,
		reset => reset, 
		ctrl_in => pams_ctrl_inputs,          --Signal_CoreDone,
		ctrl_in_ack => pams_ctrl_inputs_ack,  --Signal_CoreDone_ACK,
		ctrl_out => pams_ctrl_outputs,        --Signal_SyncDone,
		ctrl_out_ack => pams_ctrl_outputs_ack, --Signal_SyncDone_ACK,
		core_data_in_valid => '1', 
		core_data_in_read => data_to_pams_read,
		core_data_in => data_to_pams,
		core_data_out_valid  =>  data_from_pams_valid,
		core_data_out_read  => data_from_pams_valid,
		core_data_out  => data_from_pams,		
		FROM_GC_Clk => FROM_GC_Clk,
		FROM_GC_Read => FROM_GC_Read,
		FROM_GC_Data => FROM_GC_Data,
		FROM_GC_Control => FROM_GC_Control,
		FROM_GC_Exists => FROM_GC_Exists,
		TO_GC_Clk => TO_GC_Clk,
		TO_GC_Write => TO_GC_Write,
		TO_GC_Data => TO_GC_Data,
		TO_GC_Control => TO_GC_Control,
		TO_GC_Full => TO_GC_Full,
		HANDLER_RETURN_Clk => HANDLER_RETURN_Clk,
		HANDLER_RETURN_Write => HANDLER_RETURN_Write,
		HANDLER_RETURN_Data => HANDLER_RETURN_Data,
		HANDLER_RETURN_Control => HANDLER_RETURN_Control,
		HANDLER_RETURN_Full => HANDLER_RETURN_Full,
		AM_RETURN_Clk => AM_RETURN_Clk,
		AM_RETURN_Read => AM_RETURN_Read,
		AM_RETURN_Data => AM_RETURN_Data,
		AM_RETURN_Control => AM_RETURN_Control,
		AM_RETURN_Exists => AM_RETURN_Exists
	);

		pams_ctrl_inputs(0) <= Signal_CoreDone;
		Signal_CoreDone_ACK <= pams_ctrl_inputs_ack(0);
		pams_ctrl_inputs(1) <= Signal_ClearDone;
		Signal_ClearDone_ACK <= pams_ctrl_inputs_ack(1);
		Signal_SyncDone <= pams_ctrl_outputs(0);
		pams_ctrl_outputs_ack(0) <= Signal_SyncDone_ACK;

end architecture archie;
