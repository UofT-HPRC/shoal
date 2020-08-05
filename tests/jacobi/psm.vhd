library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

Library UNISIM;
use UNISIM.vcomponents.all;

Library UNIMACRO;
use UNIMACRO.vcomponents.all;


entity psm is
	generic (
		ctrl_inputs: integer := 4;
		ctrl_outputs: integer := 16;
		coderam_size: integer := 1024
	);
	port (
		clock : in std_logic; 
		reset : in std_logic;
		ctrl_in : 			in std_logic_vector( ctrl_inputs-1 downto 0) := (others => '0');
		ctrl_in_ack : 		out std_logic_vector( ctrl_inputs-1 downto 0) := (others => '0');
		ctrl_out : 			out std_logic_vector( ctrl_outputs-1 downto 0) := (others => '0');
		ctrl_out_ack : 		in std_logic_vector( ctrl_outputs-1 downto 0) := (others => '0');
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
		AM_RETURN_Exists	: in	std_logic;
		--DEBUG
		wr_addr_dg	:	out std_logic_vector(9 downto 0);
		wr_data_dg	:	out std_logic_vector(31 downto 0);
		rd_addr_dg	:	out std_logic_vector(9 downto 0);
		rd_data_dg	:	out std_logic_vector(31 downto 0);
		wr_en_dg		: 	out std_logic;
		rd_en_dg		: 	out std_logic;
		instr_valid_dg	:	out std_logic;
		instr_state_dg	: 	out std_logic_vector(1 downto 0)
		);
end entity psm;
	
architecture archie of psm is
	
	function log2(x : natural) return integer is
		variable i : integer := 0;
	begin
		-- coverage off
		if x = 0 then return 0;
		-- coverage on
		else
			while 2**i < x loop
				i := i+1;
			end loop;
			return i;
		end if;
	end function log2;
	
	component gc_handler_interface is
	port 
	(
		clock	: in	std_logic;
		reset	: in	std_logic;
		FROM_GC_Clk	: out	std_logic;
		FROM_GC_Read	: out	std_logic;
		FROM_GC_Data	: in	std_logic_vector(0 to 31);
		FROM_GC_Control	: in	std_logic;
		FROM_GC_Exists	: in	std_logic;
		HANDLER_RETURN_Clk	: out	std_logic;
		HANDLER_RETURN_Write	: out	std_logic;
		HANDLER_RETURN_Data	: out	std_logic_vector(0 to 31);
		HANDLER_RETURN_Control	: out	std_logic;
		HANDLER_RETURN_Full	: in	std_logic;
		rx_func: 			out std_logic_vector( 6 downto 0);
		rx_num_args: 		out std_logic_vector( 7 downto 0);
		rx_hcode: 			out std_logic_vector(15 downto 0);
		rx_token: 			out std_logic_vector(9 downto 0);
		rx_bsize: 			out std_logic_vector(31 downto 0);
		rx_bufaddrH:	 	out std_logic_vector(31 downto 0);
		rx_bufaddrL: 		out std_logic_vector(31 downto 0);
		rx_ParamsReady:		out std_logic;
		rx_ReadParams:		in std_logic;
		rx_Done:			out std_logic;
		CodeLoad_Exists:	out std_logic;
		CodeLoad_Read:		in std_logic;
		CodeLoad_OutData:	out std_logic_vector(31 downto 0);
		DirectReply_Exists:	out std_logic;
		DirectReply_Read:	in std_logic;
		DirectReply_OutData:out std_logic_vector(31 downto 0);
		DirectReply_OutCtrl:out std_logic;
		ArgFIFO_Exists:		out std_logic;
		ArgFIFO_Read:		in std_logic;
		ArgFIFO_OutData:	out std_logic_vector(31 downto 0);
		MemFIFO_Exists:		out std_logic;
		MemFIFO_Read:		in std_logic;
		MemFIFO_OutData:	out std_logic_vector(31 downto 0)
	);
	end component gc_handler_interface;


	-- INCOMING MESSAGE DATA
	signal rx_func: 		std_logic_vector( 6 downto 0);
	signal rx_num_args: 	std_logic_vector( 7 downto 0);
	signal rx_hcode: 		std_logic_vector(15 downto 0);
	signal rx_token: 		std_logic_vector(9 downto 0);
	signal rx_bsize: 		std_logic_vector(31 downto 0);
	signal rx_bufaddrH:	 	std_logic_vector(31 downto 0);
	signal rx_bufaddrL: 	std_logic_vector(31 downto 0);
	signal rx_ParamsReady:	std_logic;
	signal rx_ReadParams:	std_logic;
	signal rx_Done:			std_logic;
	signal ArgFIFO_Exists:	std_logic;
	signal ArgFIFO_Read:	std_logic;
	signal ArgFIFO_OutData:	std_logic_vector(31 downto 0);
	signal MemFIFO_Exists:	std_logic;
	signal MemFIFO_Read:	std_logic;
	signal MemFIFO_OutData:	std_logic_vector(31 downto 0);

	signal code_written : std_logic;
	signal brx_ReadParams : std_logic; --buffer
	signal drx_ParamsReady:	std_logic := '0'; -- delay
	signal prx_ParamsReady:	std_logic := '0'; -- pulse


	-- IMPLICIT MESSAGE PROCESSING
	constant transfer_counters: integer := 4;
	constant message_counters: integer := 4;
	constant timer_thresholds: integer := 2;
	
	constant waitvector_width: integer := (ctrl_inputs+message_counters+transfer_counters+timer_thresholds+2);

	-- transfer counters
	signal add_transfer_ctr: std_logic_vector((transfer_counters-1) downto 0);
	signal load_transfer_ctr_threshold: std_logic_vector((transfer_counters-1) downto 0);
	signal transfer_ctr_threshold_reached: std_logic_vector((transfer_counters-1) downto 0);
	signal reset_transfer_ctr: std_logic_vector((transfer_counters-1) downto 0);
	
	-- message counters
	signal incr_message_ctr: std_logic_vector((message_counters-1) downto 0);
	signal load_message_ctr_threshold: std_logic_vector((message_counters-1) downto 0);
	signal message_ctr_threshold_reached: std_logic_vector((message_counters-1) downto 0);
	signal reset_message_ctr: std_logic_vector((message_counters-1) downto 0);

	-- timer
	signal correct_timer: std_logic;
	signal timer, arrival_time: std_logic_vector(31 downto 0);
	signal reset_timer_threshold: std_logic_vector((timer_thresholds-1) downto 0);
	signal load_timer_threshold: std_logic_vector((timer_thresholds-1) downto 0);
	signal timer_threshold_reached: std_logic_vector((timer_thresholds-1) downto 0);

	-- code loading
	type code_load_state_type is (cls_idle,cls_0,cls_load,cls_addr);
	signal next_code_load_state,code_load_state,last_code_load_state: code_load_state_type := cls_idle;
	signal go_cls_0,go_cls_load,go_cls_addr,go_cls_idle : boolean;
	signal code_load_cnt: std_logic_vector(log2(coderam_size)-1 downto 0);
	signal CodeLoad_Exists : std_logic;
	signal CodeLoad_Read : std_logic;
	signal CodeLoad_OutData : std_logic_vector(31 downto 0);


	-- CODE RAM
	type ibram_type is array(0 to (coderam_size-1)) of std_logic_vector(31 downto 0);
	signal ibram: ibram_type := (others=>X"00000000");
	signal wr_addr: std_logic_vector(log2(coderam_size)-1 downto 0);
	signal wr_en: std_logic;
	signal wr_data: std_logic_vector(31 downto 0);
	signal rd_addr: std_logic_vector(log2(coderam_size)-1 downto 0);
	signal drd_addr : std_logic_vector(log2(coderam_size)-1 downto 0);
	signal incr_rd_addr : std_logic;
	signal rd_data: std_logic_vector(31 downto 0);
	signal rd_en : std_logic;
	signal addr_reset_pulse : std_logic;
	
	
	-- INSTRUCTION PROCESSING
	type instr_state_type is (ins_opcode_load, ins_opcode_valid, ins_xparams, ins_wait);
	signal instr_state: instr_state_type := ins_opcode_load;

	signal instr: std_logic_vector(31 downto 0);
	signal instr_rd, instr_valid: std_logic;
	signal load_1param : boolean;
	signal load_opcode : std_logic;
	signal instr_param_count : std_logic_vector(11 downto 0);
	signal instr_opcode : std_logic_vector(31 downto 0);

	signal go_ins_xparams : std_logic;
	signal go_ins_wait : std_logic;

	signal op_source_select : std_logic_vector(3 downto 0);
	signal op_WriteAMRequest : boolean;
	signal op_WaitFor : boolean;
	signal op_Branch : boolean;
	signal op_SetNodeID : boolean;
	signal op_WriteToCore : boolean;
	signal op_CorrectTimer : boolean;
	signal op_SetTimerThr : boolean;
	signal op_SetTransferThr : boolean;
	signal op_SetMsgThr : boolean;
	signal op_SetCtrlOut : boolean;
	signal op_Accumulate : boolean;
	signal op_ResetWaitFlags : boolean;

	signal op_accuop : std_logic_vector(1 downto 0);
	signal op_flagtestmask : std_logic_vector(3 downto 0);
	signal op_accusel : std_logic_vector(1 downto 0);
	signal op_1param : std_logic;
	signal op_xparams : std_logic;
	signal op_endAM : std_logic;
	signal op_waitflags : std_logic_vector((waitvector_width-1) downto 0);
	
	signal branch: std_logic;
	signal branch_addr: std_logic_vector(log2(coderam_size)-1 downto 0);


	-- NODE ID
	signal node_id : std_logic_vector(15 downto 0);


	-- CTRL INPUTS/OUTPUTS FROM/TO CORE
	signal load_outputs : std_logic;
	signal bctrl_in_ack : std_logic_vector((ctrl_inputs-1) downto 0);
	signal ctrl_in_reg : std_logic_vector((ctrl_inputs-1) downto 0);


	-- WAITING
	signal wait_complete : std_logic;
	signal wait_vector : std_logic_vector((waitvector_width-1) downto 0);
	signal wait_signals : std_logic_vector((waitvector_width-1) downto 0);
	signal waitsignals_fullword : std_logic_vector(31 downto 0);

	
	-- ARITHMETICS
	signal AccumulatorA : std_logic_vector(31 downto 0);
	signal AccumulatorB : std_logic_vector(31 downto 0);
	signal AccumulatorC : std_logic_vector(31 downto 0);
	signal AccumulatorD : std_logic_vector(31 downto 0);
	signal next_AccumulatorA : std_logic_vector(31 downto 0);
	signal next_AccumulatorB : std_logic_vector(31 downto 0);
	signal next_AccumulatorC : std_logic_vector(31 downto 0);
	signal next_AccumulatorD : std_logic_vector(31 downto 0);
	signal AccOperand_transfer : std_logic;
	signal AccOperand_valid : std_logic;
	signal AccOperand : std_logic_vector(31 downto 0);
	signal flagsA : std_logic_vector(3 downto 0);
	signal flagsB : std_logic_vector(3 downto 0);
	signal flagsC : std_logic_vector(3 downto 0);
	signal flagsD : std_logic_vector(3 downto 0);
	signal flags : std_logic_vector(3 downto 0);
	signal flags_tested : std_logic_vector(3 downto 0);

	type flag_literal_type is (pos_not0,pos_0,neg_not0);
	signal flags_literal: flag_literal_type;


	-- DATA TRANSFERS 
	type AMsource_state_type is (ams_idle,ams_directreply,ams_pams);
	signal next_AMsource_state,AMsource_state: AMsource_state_type := ams_idle;
	signal DirectReply_Exists : std_logic;
	signal DirectReply_Read : std_logic;
	signal DirectReply_OutData : std_logic_vector(31 downto 0);
	signal DirectReply_OutCtrl : std_logic;
	signal PAMS_GC_Data : std_logic_vector(31 downto 0);
	signal PAMS_GC_Full : std_logic;
	signal PAMS_GC_Exists : std_logic;

	signal transfer_to_GC : std_logic;
	signal transfer_to_core : std_logic;
	signal code_data_read : std_logic;
	signal bcore_data_out_valid : std_logic;
	signal DataSource : std_logic_vector(31 downto 0);
	signal DataSource_valid : std_logic;
	signal DataSource_Transfer : std_logic;
	signal final_AM_transfer : std_logic;
	
	
begin

	TO_GC_Control <= '0';
	TO_GC_CLK <= clock;
	
	AM_RETURN_Clk <= clock;
	AM_RETURN_Read <=  '1' when
							((instr_state=ins_opcode_valid and  (op_ResetWaitFlags)) or wait_complete='1')
							and instr_opcode(1)='1'
							else '0';

	TO_GC_Data			<= DirectReply_OutData when (AMsource_state=ams_directreply) else PAMS_GC_Data;
	TO_GC_Write			<= (DirectReply_Exists and not TO_GC_Full) when (AMsource_state=ams_directreply) else (PAMS_GC_Exists and not TO_GC_Full);
	DirectReply_Read	<= (DirectReply_Exists and not TO_GC_Full) when (AMsource_state=ams_directreply) else '0';
	PAMS_GC_Full		<= '1' when (AMsource_state=ams_directreply) else TO_GC_Full;
	
	-- TODO: CONSIDER SRL32 FIFO for PAMS AM requests; right now, a directReplys can block execution until its written
	ams_fsm:process(AMsource_state, reset, DirectReply_Exists, DirectReply_OutCtrl, TO_GC_Full, final_AM_transfer, PAMS_GC_Exists)
		variable next_state: AMsource_state_type;
	begin
		next_state := AMsource_state;
		if reset='1' then
			next_state := ams_idle;
		else
			case AMsource_state is
				when ams_idle =>
					if (PAMS_GC_Exists='1') then
						next_state := ams_pams;
					elsif (DirectReply_Exists='1') then
						next_state := ams_directreply;
					end if;
				when ams_directreply =>
					if (DirectReply_Exists='1' and TO_GC_Full='0' and DirectReply_OutCtrl='1') then
						next_state := ams_idle;
					end if;
				when ams_pams =>
					if (final_AM_transfer='1') then
						next_state := ams_idle;
					end if;
			end case;
		end if;
		next_AMsource_state <= next_state;		
	end process;
	AMsource_state <= next_AMsource_state when (rising_edge(clock)) else AMsource_state;
		
		
	
	Inst_gc_hndlr_if: gc_handler_interface
	port map
	(
		clock => clock,
		reset => reset,
		FROM_GC_Clk => FROM_GC_Clk,
		FROM_GC_Read => FROM_GC_Read,
		FROM_GC_Data => FROM_GC_Data,
		FROM_GC_Control => FROM_GC_Control,
		FROM_GC_Exists => FROM_GC_Exists,
		HANDLER_RETURN_Clk => HANDLER_RETURN_Clk,
		HANDLER_RETURN_Write => HANDLER_RETURN_Write,
		HANDLER_RETURN_Data => HANDLER_RETURN_Data,
		HANDLER_RETURN_Control => HANDLER_RETURN_Control,
		HANDLER_RETURN_Full => HANDLER_RETURN_Full,
		rx_func => rx_func,
		rx_num_args => rx_num_args,
		rx_hcode => rx_hcode,
		rx_token => rx_token,
		rx_bsize => rx_bsize,
		rx_bufaddrH => rx_bufaddrH,
		rx_bufaddrL => rx_bufaddrL,
		rx_ParamsReady => rx_ParamsReady,
		rx_ReadParams => rx_ReadParams,
		rx_Done => rx_Done,
		CodeLoad_Exists  => CodeLoad_Exists,
		CodeLoad_Read    => CodeLoad_Read,
		CodeLoad_OutData => CodeLoad_OutData,
		DirectReply_Exists => DirectReply_Exists,
		DirectReply_Read => DirectReply_Read,
		DirectReply_OutData => DirectReply_OutData,
		DirectReply_OutCtrl => DirectReply_OutCtrl,
		ArgFIFO_Exists => ArgFIFO_Exists,
		ArgFIFO_Read => ArgFIFO_Read,
		ArgFIFO_OutData => ArgFIFO_OutData,
		MemFIFO_Exists => MemFIFO_Exists,
		MemFIFO_Read => MemFIFO_Read,
		MemFIFO_OutData => MemFIFO_OutData
	);
	

	-- handler call data acknowledge
	rx_ReadParams <= brx_ReadParams;
	brx_ReadParams <= '1' when rx_ParamsReady='1' and (
						(add_transfer_ctr/=0) or
						(incr_message_ctr/=0) or
						(code_load_state=cls_idle and last_code_load_state/=cls_idle)
						) else '0';
						
	-- Parameters ready pulse
	drx_ParamsReady <= rx_ParamsReady when rising_edge(clock) else drx_ParamsReady;
	prx_ParamsReady <= not drx_ParamsReady and rx_ParamsReady;


	-- message counters
	msgctrs: for i in 0 to (message_counters-1) generate
		signal message_ctr_i: std_logic_vector(31 downto 0);
		signal message_ctr_hcode_i: std_logic_vector(7 downto 0);
		signal message_ctr_threshold_i: std_logic_vector(31 downto 0);
	begin
		incr_message_ctr(i) <= '1' when (rx_hcode(7 downto 0)=message_ctr_hcode_i and message_ctr_hcode_i/=0 and rx_ParamsReady='1') else '0';
		
		messagectr_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_message_ctr(i)='1' then
					message_ctr_i <= (others => '0');
				elsif incr_message_ctr(i)='1' then 
					message_ctr_i <= message_ctr_i + 1;
				end if;
			end if;
		end process;
		
		load_message_ctr_threshold(i) <= '1' when load_1param and op_SetMsgThr and instr_opcode(i)='1' else '0';
		reset_message_ctr(i) <= '1' when
								((instr_state=ins_opcode_valid and  (op_ResetWaitFlags)) or wait_complete='1')
									and instr_opcode(i+8)='1'
								else '0';

		msg_ctr_hcode_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_message_ctr(i)='1' then
					message_ctr_hcode_i <= (others => '0'); -- 0 ostensibly reserved; TODO: check w/original GASNet implementation
				elsif load_message_ctr_threshold(i)='1' then 
					message_ctr_hcode_i <= instr_opcode(11 downto 4);
				end if;
			end if;
		end process;

		msg_ctr_threshold_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_message_ctr(i)='1' then
					message_ctr_threshold_i <= (others => '1'); -- way up
				elsif load_message_ctr_threshold(i)='1' then 
					message_ctr_threshold_i <= instr;
				end if;
			end if;
		end process;

		message_ctr_threshold_reached(i) <= '1' when (message_ctr_i>=message_ctr_threshold_i) else '0';		
	end generate msgctrs;
	
	-- transfer counters
	transferctrs: for i in 0 to (transfer_counters-1) generate
		signal transfer_ctr_i:	std_logic_vector(31 downto 0);
		signal transfer_ctr_hcode_i: std_logic_vector(7 downto 0);
		signal transfer_ctr_threshold_i: std_logic_vector(31 downto 0);
	begin
		add_transfer_ctr(i) <= '1' when (rx_hcode(7 downto 0)=transfer_ctr_hcode_i and transfer_ctr_hcode_i/=0 and rx_ParamsReady='1') else '0';
		
		transferctr0: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_transfer_ctr(i)='1' then
					transfer_ctr_i <= (others => '0');
				elsif add_transfer_ctr(i)='1' then 
					transfer_ctr_i <= transfer_ctr_i + rx_bsize;
				end if;
			end if;
		end process;

		load_transfer_ctr_threshold(i) <= '1' when load_1param and op_SetTransferThr and instr_opcode(i)='1' else '0';
		reset_transfer_ctr(i) <= '1' when
								((instr_state=ins_opcode_valid and  (op_ResetWaitFlags)) or wait_complete='1') and
									instr_opcode(i+4)='1'
									else '0';

		trsnfr_ctr_hcode_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_transfer_ctr(i)='1' then
					transfer_ctr_hcode_i <= (others => '0'); -- 0 ostensibly reserved; TODO: check w/original GASNet implementation
				elsif load_transfer_ctr_threshold(i)='1' then 
					transfer_ctr_hcode_i <= instr_opcode(11 downto 4);
				end if;
			end if;
		end process;

		trnsfr_ctr_threshold_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_transfer_ctr(i)='1' then
					transfer_ctr_threshold_i <= (others => '1'); -- way up
				elsif load_transfer_ctr_threshold(i)='1' then 
					transfer_ctr_threshold_i <= instr;
				end if;
			end if;
		end process;

		transfer_ctr_threshold_reached(i) <= '1' when (transfer_ctr_i>=transfer_ctr_threshold_i) else '0';		
	end generate transferctrs;
	
	-- timer
	timerproc: process(clock)
	begin
		if rising_edge(clock) then
			if reset='1' then
				timer <= (others => '0');
			elsif correct_timer='1' then
				timer <= timer + instr + 1; 
			elsif (timer/=-2) then -- stop at overflow
				timer <= timer + 1;
			end if;
		end if;
	end process;
	correct_timer <= '1' when load_1param and op_CorrectTimer else '0';

	-- arrival time								
	arrivetime:process (clock) 
	begin
		if rising_edge(clock) then
			if reset = '1' then
				arrival_time <= (others => '0');
			elsif (prx_ParamsReady='1' and rx_hcode(7 downto 0)/=255) then -- if 255, no time is taken -> this way last arrival time can be requested
				arrival_time <= timer;
			end if;
		end if;
	end process arrivetime;
	
	timer_thresholds_proc: for i in 0 to (timer_thresholds-1) generate
		signal timer_threshold_i: std_logic_vector(31 downto 0);
	begin
		load_timer_threshold(i) <= '1' when load_1param and op_SetTimerThr and instr_opcode(i)='1' else '0';
		reset_timer_threshold(i) <= '1' when
									((instr_state=ins_opcode_valid and  (op_ResetWaitFlags)) or wait_complete='1')
									and instr_opcode(i+2)='1'
									else '0';
								
		tmr_threshold_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or reset_timer_threshold(i)='1' then
					timer_threshold_i <= (others => '1'); -- way up
				elsif load_timer_threshold(i)='1' then 
					timer_threshold_i <= instr;
				end if;
			end if;
		end process;
		
		timer_threshold_reached(i) <= '1' when (timer>=timer_threshold_i) else '0';		
	end generate timer_thresholds_proc;

	-- ACCUMULATOR ARITHMETICS AND BRANCHING
	with op_accuop select
	next_AccumulatorA <=	(AccOperand) when "00", -- load
							(AccumulatorA + AccOperand) when "01", -- add
							(AccumulatorA - AccOperand) when "10", -- sub
							(AccumulatorA and AccOperand) when others; --"11"

	flagsA(3) <= '1' when (next_AccumulatorA(31)='0')	else '0'; -- positive
	flagsA(2) <= '1' when (next_AccumulatorA(31)='1')	else '0'; -- negative
	flagsA(1) <= '1' when (next_AccumulatorA=0)			else '0'; -- zero
	flagsA(0) <= '1' when (next_AccumulatorA/=0)		else '0'; -- not zero
	
	with op_accuop select
	next_AccumulatorB <=	(AccOperand) when "00", -- load
							(AccumulatorB + AccOperand) when "01", -- add
							(AccumulatorB - AccOperand) when "10", -- sub
							(AccumulatorB and AccOperand) when others; --"11"
	
	flagsB(3) <= '1' when (next_AccumulatorB(31)='0')	else '0'; -- positive
	flagsB(2) <= '1' when (next_AccumulatorB(31)='1')	else '0'; -- negative
	flagsB(1) <= '1' when (next_AccumulatorB=0)			else '0'; -- zero
	flagsB(0) <= '1' when (next_AccumulatorB/=0)		else '0'; -- not zero
	
	with op_accuop select
	next_AccumulatorC <=	(AccOperand) when "00", -- load
							(AccumulatorC + AccOperand) when "01", -- add
							(AccumulatorC - AccOperand) when "10", -- sub
							(AccumulatorC and AccOperand) when others; --"11"

	flagsC(3) <= '1' when (next_AccumulatorC(31)='0')	else '0'; -- positive
	flagsC(2) <= '1' when (next_AccumulatorC(31)='1')	else '0'; -- negative
	flagsC(1) <= '1' when (next_AccumulatorC=0)			else '0'; -- zero
	flagsC(0) <= '1' when (next_AccumulatorC/=0)		else '0'; -- not zero
	
	with op_accuop select
	next_AccumulatorD <=	(AccOperand) when "00", -- load
							(AccumulatorD + AccOperand) when "01", -- add
							(AccumulatorD - AccOperand) when "10", -- sub
							(AccumulatorD and AccOperand) when others; --"11"
	
	flagsD(3) <= '1' when (next_AccumulatorD(31)='0')	else '0'; -- positive
	flagsD(2) <= '1' when (next_AccumulatorD(31)='1')	else '0'; -- negative
	flagsD(1) <= '1' when (next_AccumulatorD=0)			else '0'; -- zero
	flagsD(0) <= '1' when (next_AccumulatorD/=0)		else '0'; -- not zero
	
	Accproc: process(clock)
	begin
		if rising_edge(clock) then
			-- STAGE 1
			if reset='1' then
				AccOperand <= (others => '0');
				AccOperand_valid <= '0';
			elsif (op_Accumulate and AccOperand_transfer='1') then
				AccOperand    <= DataSource;
				AccOperand_valid <= '1';
			else
				AccOperand_valid <= '0';
			end if;
			-- STAGE 2
			if reset='1' then
				AccumulatorA <= (others => '0');
				AccumulatorB <= (others => '0');
				AccumulatorC <= (others => '0');
				AccumulatorD <= (others => '0');
				flags        <= "1010"; -- positive 0 
			elsif (op_Accumulate and AccOperand_valid='1') then
				case op_accusel is
					when "00" => 
						AccumulatorA <= next_AccumulatorA;
						flags        <= flagsA; 
					when "01" => 
						AccumulatorB <= next_AccumulatorB;
						flags        <= flagsB; 
					when "10" => 
						AccumulatorC <= next_AccumulatorC;
						flags        <= flagsC; 
					when "11" => 
						AccumulatorD <= next_AccumulatorD;
						flags        <= flagsD;
					when others => 
				end case; 
			end if;
		end if;
	end process;
	flags_literal <= pos_0 when flags(1)='1' else pos_not0 when flags(3)='1' else neg_not0; 
	
	flags_tested <= op_flagtestmask and not flags;

	branch <= '1' when instr_state=ins_opcode_valid and op_Branch and (flags_tested=0) else '0';
	branch_addr <= instr_opcode(log2(coderam_size)-1 downto 0);
	

	-- outputs
	load_outputs <= '1' when instr_state=ins_opcode_valid and op_SetCtrlOut else '0';

	outputs: for i in 0 to (ctrl_outputs-1) generate
		signal dctrl_out_ack_i:std_logic;
		signal pctrl_out_ack_i:std_logic;
		signal bctrl_out_i:std_logic;
	begin
		-- detect edge
		dctrl_out_ack_i <= ctrl_out_ack(i) when rising_edge(clock) else dctrl_out_ack_i; -- TODO: check if OK w/o reset
		pctrl_out_ack_i <= not dctrl_out_ack_i and ctrl_out_ack(i);
								
		ctrl_out_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' then
					bctrl_out_i <= '0';
				elsif load_outputs='1' then -- if load and ack at the same time, load wins (==re-load after ack) 
					bctrl_out_i <= instr_opcode(i) or bctrl_out_i;
				elsif pctrl_out_ack_i='1' then
					bctrl_out_i <= '0';
				end if;
			end if;
		end process;
		ctrl_out(i) <= bctrl_out_i;
	end generate outputs;
	
	--inputs
	inputs: for i in 0 to (ctrl_inputs-1) generate
		signal dctrl_in_i:std_logic;
		signal pctrl_in_i:std_logic;
	begin
		-- detect edge
		dctrl_in_i <= ctrl_in(i) when rising_edge(clock) else dctrl_in_i; -- TODO: check if OK w/o reset
		pctrl_in_i <= not dctrl_in_i and ctrl_in(i);
		
		ctrl_in_ack(i) <= bctrl_in_ack(i); 
		bctrl_in_ack(i) <=  '1' when
								((instr_state=ins_opcode_valid and  (op_ResetWaitFlags)) or wait_complete='1')
								 and instr_opcode(i+12)='1'
								 else '0';
								
		ctrl_in_i: process(clock)
		begin
			if rising_edge(clock) then
				if reset='1' or bctrl_in_ack(i)='1' then
					ctrl_in_reg(i) <= '0';
				elsif ctrl_in(i)='1' then--pctrl_in_i='1' then   -- TODO consider what pulse detection is good for
					ctrl_in_reg(i) <= '1';
				end if;
			end if;
		end process;
	end generate inputs;

	-- node id
	set_node_id:process (clock) 
	begin
		if rising_edge(clock) then
			if reset = '1' then
				node_id <= (others => '0');
			elsif (instr_state=ins_opcode_valid and op_SetNodeID) then
				node_id <= instr_opcode(15 downto 0);
			end if;
		end if;
	end process set_node_id;
		
		
	-- INSTRUCTION PROCESSING
	
	-- DEBUG
	instr_state_dg <= "00" when instr_state = ins_opcode_load else
                 "01" when instr_state = ins_opcode_valid else
                 "10" when instr_state = ins_xparams else
                 "11";
	
	instr_fsm:process (clock) 
	begin
		if rising_edge(clock) then
			if reset = '1' then
				instr_state <= ins_opcode_load;
			else
				case instr_state is
					when ins_opcode_load =>
						if (instr_valid='1') then instr_state <= ins_opcode_valid; end if;
					when ins_opcode_valid =>
						if (op_1param='1') then instr_state <= ins_opcode_load; -- next cycle spend re-loading opcode (this cycle 1 param)
						elsif (branch='1') then instr_state <= ins_opcode_load; -- next cycle spend re-loading address
						elsif (go_ins_xparams='1') then instr_state <= ins_xparams;
						elsif (go_ins_wait='1') then instr_state <= ins_wait;
						-- else: next one is  ins_opcode_valid, too  
 						end if;
					when ins_xparams =>
						if (DataSource_Transfer='1' and instr_param_count=1) then -- one transfer left
							--if (op_source_select=X"0") then -- source: code
								instr_state <= ins_opcode_load; -- needs extra cycle
							--else
								--instr_state <= ins_opcode_valid; -- can load opcode in parallel 
							--end if;
						end if;
					when ins_wait =>
						if (wait_complete='1') then instr_state <= ins_opcode_valid; end if;
				end case;
			end if;
		end if;
	end process instr_fsm;

	go_ins_xparams <= '1' when (instr_state=ins_opcode_valid and op_xparams='1') else '0';

	go_ins_wait <= '1' when (instr_state=ins_opcode_valid and op_WaitFor and wait_complete='0') else '0';
	
	load_opcode <= '1' when (instr_state=ins_opcode_load and instr_valid='1') or
							(instr_state=ins_opcode_valid and go_ins_wait='0' and op_xparams='0' and op_1param='0' and branch='0') or
							(instr_state=ins_wait and wait_complete='1') --or
							--(DataSource_Transfer='1' and instr_param_count=1 and op_source_select/=X"0")
							 else '0';

	instr_rd <= '1' when 	(instr_state=ins_opcode_load and instr_valid='1') or
							(instr_state=ins_opcode_valid and load_1param) or
							(instr_state=ins_opcode_valid and go_ins_wait='0' and op_xparams='0' and op_1param='0' and branch='0') or
							(instr_state=ins_wait and wait_complete='1') or
							(instr_state=ins_xparams and code_data_read='1') --or -- instr_state=ins_xparams implied
							--(DataSource_Transfer='1' and instr_param_count=1)   -- load param or load next op
							 else '0';

	load_1param <= (instr_state=ins_opcode_valid) and (op_1param='1');

	opcode_proc:process (clock) 
	begin
		if rising_edge(clock) then
			if reset = '1' then
				instr_opcode <= (others => '0');   
			elsif (load_opcode='1') then
				instr_opcode <= instr; 
			end if;
		end if;
	end process opcode_proc;

	
	params_proc:process (clock) 
	begin
		if rising_edge(clock) then
			if reset = '1' then
				instr_param_count <= (others => '0');   
			elsif go_ins_xparams='1' then
				instr_param_count <= instr_opcode(11 downto 0);
			elsif (DataSource_Transfer='1') then -- instr_state=ins_xparams  implied
			 	instr_param_count <= instr_param_count - 1; 
			end if;
		end if;
	end process params_proc;



	-- OPCODE DECODING
	op_WaitFor			<= true when instr_opcode(31 downto 27)="00000" else false;
	op_WriteAMRequest	<= true when instr_opcode(31 downto 27)="00001" else false;
	op_Branch			<= true when instr_opcode(31 downto 27)="00010" else false;
	op_SetNodeID		<= true when instr_opcode(31 downto 27)="00011" else false;
	op_CorrectTimer		<= true when instr_opcode(31 downto 27)="00100" else false;
	op_SetTimerThr		<= true when instr_opcode(31 downto 27)="00101" else false;
	op_SetTransferThr	<= true when instr_opcode(31 downto 27)="00110" else false;
	op_SetMsgThr		<= true when instr_opcode(31 downto 27)="00111" else false;
	op_SetCtrlOut		<= true when instr_opcode(31 downto 27)="01000" else false;
	op_WriteToCore		<= true when instr_opcode(31 downto 27)="01001" else false;
	op_Accumulate		<= true when instr_opcode(31 downto 27)="01010" else false;
	op_ResetWaitFlags	<= true when instr_opcode(31 downto 27)="01011" else false;
	op_1param  <= instr_opcode(26);
	op_xparams <= instr_opcode(25);
	op_endAM   <= instr_opcode(24);
	op_source_select <= instr_opcode(23 downto 20);
	op_flagtestmask <= instr_opcode(15 downto 12);
	op_accuop <= instr_opcode(15 downto 14);
	op_accusel <= instr_opcode(13 downto 12);
	op_waitflags <= instr_opcode((11+ctrl_inputs) downto 12) & instr_opcode((7+message_counters) downto 8) &  
					instr_opcode((3+transfer_counters) downto 4) & instr_opcode((1+timer_thresholds) downto 2) &
					instr_opcode(1) & instr_opcode(0);
	

	-- WAIT	
	wait_signals <=	ctrl_in_reg((ctrl_inputs-1) downto 0) &	message_ctr_threshold_reached &  
					transfer_ctr_threshold_reached & timer_threshold_reached &
					AM_RETURN_EXISTS & code_written;
	
	wait_vector <= op_waitflags and not wait_signals;	
	
						
	wait_complete <= '1' when wait_vector=0 else '0';
	 

	-- DATA TRANSFER
								 
	final_AM_transfer <=  op_endAM when (transfer_to_GC='1' and (instr_state=ins_xparams and instr_param_count=1)) else '0';

	transfer_to_core <= bcore_data_out_valid and core_data_out_read;
	core_data_out_valid <= bcore_data_out_valid;
	bcore_data_out_valid <= '1' when (instr_state=ins_xparams and instr_param_count/=0)
									 and op_WriteToCore and DataSource_Valid='1' else '0';

	transfer_to_GC <= PAMS_GC_Exists and not PAMS_GC_Full;
	PAMS_GC_Exists <= '1' when (instr_state=ins_xparams and instr_param_count/=0) and op_WriteAMRequest and DataSource_Valid='1' else '0';

	AccOperand_transfer <= '1' when (instr_state=ins_xparams and instr_param_count/=0) and op_Accumulate and DataSource_Valid='1' else '0';
	
	DataSource_Transfer <= transfer_to_core or transfer_to_GC or AccOperand_transfer;

	with op_source_select select
	DataSource_Valid <= instr_valid when X"0",
						'1' when X"1", -- timer always available
						'1' when X"2", -- timer arrival always available
						(core_data_in_valid) when X"3",
						'1' when X"4", -- waitvector always available
						'1' when X"5", -- id always available
						ArgFIFO_Exists when X"6", -- argFIFO
						MemFIFO_Exists when X"7", -- memFIFO
						'1' when X"8", -- accumulators always available
						'1' when X"9", -- accumulators always available
						'1' when X"A", -- accumulators always available
						'1' when X"B", -- accumulators always available
						'0' when others;
								
	
	with op_source_select select
	DataSource <=	instr			when X"0",
					timer			when X"1",
					arrival_time	when X"2",
					core_data_in	when X"3",
					waitsignals_fullword when X"4",
					X"0000" & node_id	 when X"5",
					ArgFIFO_OutData	when X"6",
					MemFIFO_OutData	when X"7",
					AccumulatorA	when X"8",
					AccumulatorB	when X"9",
					AccumulatorC	when X"A",
					AccumulatorD	when others;
					
	waitsignals_fullword((waitvector_width-1) downto 0) <= wait_signals;
	waitsignals_fullword(31 downto waitvector_width) <= (others => '0');

	PAMS_GC_Data  <= DataSource;
	core_data_out <= DataSource;

	code_data_read		<= '1' when (DataSource_Transfer='1' and op_source_select=X"0") else '0';
	core_data_in_read	<= '1' when (DataSource_Transfer='1' and op_source_select=X"3") else '0';
	ArgFIFO_Read		<= '1' when (DataSource_Transfer='1' and op_source_select=X"6") else '0';
	MemFIFO_Read		<= '1' when (DataSource_Transfer='1' and op_source_select=X"7") else '0';
	

	-- INSTRUCTIONS AND CODE LOADING
	
	process (clock) 
	begin 
	if (rising_edge(clock)) then 
		if (wr_en='1') then 
			ibram(conv_integer(wr_addr)) <= wr_data; 
		end if; 
		--read_a <= wr_addr;
		if (rd_en = '1') then
			drd_addr <= rd_addr;         
		end if;
	end if; 
	end process; 
	rd_data <= ibram(conv_integer(drd_addr)); 
	-- On reset, first instruction memory word is re-initialized with "wait for code load finished"
	wr_data <= X"00000001" when reset='1' else CodeLoad_OutData;
	
	-- code loading and reading
	go_cls_0 		<= (rx_hcode(7 downto 0)=64 and prx_ParamsReady='1');
	go_cls_load 	<= (rx_hcode(7 downto 0)=65 and prx_ParamsReady='1'); -- for attach mode, correct message sequence needs to be guaranteed (not through IP)
	go_cls_addr 	<= (rx_hcode(7 downto 0)=66 and prx_ParamsReady='1');
	go_cls_idle		<= (code_load_cnt=1 and CodeLoad_Exists='1');
	clsstate:process(reset,code_load_state,
					go_cls_0,go_cls_load,go_cls_addr,
					CodeLoad_Exists, go_cls_idle)
		variable next_state: code_load_state_type;
	begin
		next_state := code_load_state;
		if reset='1' then
			next_state := cls_idle;
		else
			case code_load_state is
				when cls_idle =>
					if go_cls_0 then next_state := cls_0;
					elsif go_cls_addr then next_state := cls_addr;
					elsif go_cls_load then next_state := cls_load;
					end if;
				when cls_0 =>
					next_state := cls_load;
				when cls_addr =>
					if (CodeLoad_Exists='1') then next_state := cls_load; end if;
				when cls_load =>
					if (go_cls_idle) then next_state := cls_idle; end if;
			end case;
		end if;
		next_code_load_state <= next_state;
	end process clsstate;

	code_load_state_proc:process (clock) 
	begin
		if rising_edge(clock) then
			if (reset = '1') then	
				code_load_state <= cls_idle;
				last_code_load_state <= cls_idle;
			else
				code_load_state <= next_code_load_state;
				last_code_load_state <= code_load_state;
			end if;
		end if;
	end process code_load_state_proc;
	
	CodeLoad_Read <= CodeLoad_Exists when (code_load_state=cls_addr) or (code_load_state=cls_load) else '0';
	
	code_load_cnt_proc:process (clock) 
	begin
		if rising_edge(clock) then
			if (reset = '1') then
				code_load_cnt <= (others => '0');
			elsif (next_code_load_state=cls_load and code_load_state/=cls_load) then
				code_load_cnt <= rx_bsize(11 downto 2); --bytes to words
			elsif (code_load_cnt/=0 and CodeLoad_Exists='1') then
				code_load_cnt <= code_load_cnt - 1;
			end if;
		end if;
	end process code_load_cnt_proc;
	
	codewritten_proc:process (clock) 
	begin
		if rising_edge(clock) then
			if (reset = '1' or 
				(((instr_state=ins_opcode_valid and op_ResetWaitFlags) or wait_complete='1') and instr_opcode(0)='1')
				) then
				code_written <= '0';
			elsif (rx_hcode(7 downto 2)="010000" and brx_readParams='1') then
				code_written <= '1';
			end if;
		end if;
	end process codewritten_proc;
						
	-- write new code to ibram
	code_wr_addr_proc:process (clock) 
	begin
		if rising_edge(clock) then
			if ((reset = '1') or (code_load_state=cls_0)) then
				wr_addr <= (others => '0');
			elsif (code_load_state=cls_addr and CodeLoad_Exists='1') then
				wr_addr <= CodeLoad_OutData(log2(coderam_size)-1 downto 0);
			elsif (code_load_state=cls_load and CodeLoad_Exists='1') then
				wr_addr <= wr_addr + 1;
			end if;
		end if;
	end process code_wr_addr_proc;
	wr_en <= '1' when (reset='1' or (code_load_state=cls_load and CodeLoad_Exists='1')) else '0';

	-- CODE READING
	code_rd_addr_proc:process (clock)
	begin
		if rising_edge(clock) then
			if reset = '1' then
				rd_addr				<= (others => '0');
				addr_reset_pulse	<= '1'; 
			elsif branch = '1' then
				rd_addr				<= branch_addr;
				addr_reset_pulse	<= '1'; 
			else
				addr_reset_pulse	<= '0';
				if (incr_rd_addr='1') then
					rd_addr <= rd_addr + 1;
				end if;

			end if;
		end if;
	end process;
	incr_rd_addr <= (not reset and addr_reset_pulse) or instr_rd;
	rd_en <= addr_reset_pulse or instr_rd;
		
	instr 		<= rd_data;
	instr_valid <= not addr_reset_pulse and not branch;
	
	
	-- DEBUG
	rd_addr_dg	<= rd_addr;
	rd_data_dg	<= rd_data;
	wr_addr_dg	<= wr_addr;
	wr_data_dg	<= wr_data;
	wr_en_dg <= wr_en;
	rd_en_dg <= rd_en;
	instr_valid_dg	<= instr_valid;
end architecture archie;
