library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity gc_handler_interface is
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
		DirectReply_Exists:	out std_logic;
		DirectReply_Read:	in std_logic;
		DirectReply_OutData:out std_logic_vector(31 downto 0);
		DirectReply_OutCtrl:out std_logic;
		CodeLoad_Exists:	out std_logic;
		CodeLoad_Read:		in std_logic;
		CodeLoad_OutData:	out std_logic_vector(31 downto 0);
		ArgFIFO_Exists:		out std_logic;
		ArgFIFO_Read:		in std_logic;
		ArgFIFO_OutData:	out std_logic_vector(31 downto 0);
		MemFIFO_Exists:		out std_logic;
		MemFIFO_Read:		in std_logic;
		MemFIFO_OutData:	out std_logic_vector(31 downto 0)
	);

end gc_handler_interface;

architecture archie of gc_handler_interface is

	--	Func structure (6 downto 0) :   Reply | Async | FIFO | X | Long | Medium/Long_Vector | Short/Long_Strided  
		
	function isShortAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(2 downto 0)="001");
	end function isShortAM;

	function isMediumAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(2 downto 0)="010");
	end function isMediumAM;

	function isLongxAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(2)='1');
	end function isLongxAM;

	function isLongAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(2 downto 0)="100");
	end function isLongAM;

	function isLongVectoredAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(2 downto 0)="110");
	end function isLongVectoredAM;

	function isLongStridedAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(2 downto 0)="101");
	end function isLongStridedAM;

	function isFIFOnotMemData(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(4)='1');
	end function isFIFOnotMemData;

	function isAsyncAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(5)='1');
	end function isAsyncAM;

	function isReplyAM(func: in std_logic_vector(6 downto 0)) return boolean is
	begin
		return (func(6)='1');
	end function isReplyAM;
	
	function stdlogic(B: boolean) return std_ulogic is
		begin if B then return('1'); else return('0'); end if;
		end function stdlogic;

	component SRL_FIFO is
	generic (
		C_DATA_BITS : integer := 8;
		C_DEPTH     : integer := 16
		);
	port (
		Clk             : in  std_logic;
		Reset           : in  std_logic;
		FIFO_Write      : in  std_logic;
		Data_In         : in  std_logic_vector(0 to C_DATA_BITS-1);
		FIFO_Read       : in  std_logic;
		Data_Out        : out std_logic_vector(0 to C_DATA_BITS-1);
		FIFO_Full       : out std_logic;
		FIFO_almost_Full       : out std_logic;
		-- FIFO_Half_Full  : out std_logic;
		-- FIFO_Half_Empty : out std_logic;
		Data_Exists     : out std_logic
		);
	end component SRL_FIFO;
	
	component SRL_FIFO32
		generic(C_DATA_BITS : integer := 8;
			    C_DEPTH     : integer := 32);
		port(Clk         : in  std_logic;
			 Reset       : in  std_logic;
			 FIFO_Write  : in  std_logic;
			 Data_In     : in  std_logic_vector(0 to C_DATA_BITS - 1);
			 FIFO_Read   : in  std_logic;
			 Data_Out    : out std_logic_vector(0 to C_DATA_BITS - 1);
			 FIFO_Full   : out std_logic;
			 Data_Exists : out std_logic);
	end component SRL_FIFO32;

	signal RXfunc: 			std_logic_vector( 6 downto 0);
		signal RXfunc_valid: 	std_logic;
		signal RXfunc_write: 	std_logic;
	signal RXnum_args: 		 std_logic_vector( 7 downto 0);
		signal RXnum_args_valid: std_logic;
		signal RXnum_args_write: std_logic;
	signal RXhcode: 		std_logic_vector(15 downto 0);
		signal RXhcode_valid: std_logic;
		signal RXhcode_write: std_logic;
	signal RXtoken: 		std_logic_vector(9 downto 0);
		signal RXtoken_valid: std_logic;
		signal RXtoken_write: std_logic;
	signal RXbsize: 		std_logic_vector(31 downto 0);
		signal RXbsize_valid: std_logic;
		signal RXbsize_write: std_logic;
	signal RXbufaddrH:	 	std_logic_vector(31 downto 0);
		signal RXbufaddrH_valid: std_logic;
		signal RXbufaddrH_write: std_logic;
	signal RXbufaddrL: 		std_logic_vector(31 downto 0);
		signal RXbufaddrL_valid: std_logic;
		signal RXbufaddrL_write: std_logic;

	signal RXdummy: 		std_logic_vector(17 downto 0);
	signal RXmemwords_packed: 	std_logic_vector(15 downto 0);
		signal RXmemwords_packed_valid: std_logic;
		signal RXmemwords_packed_read: std_logic;
		signal RXmemwords_packed_write: std_logic;

	signal RXargwrite:		std_logic;
	signal RXarg_cnt: 		std_logic_vector( 7 downto 0);
	signal ArgFIFO_InData:	std_logic_vector(31 downto 0);
	signal ArgFIFO_Write:		std_logic;
	signal ArgFIFO_Full:		std_logic;

	signal RXmemFwrite:		std_logic;
	signal RXmemF_cnt: 		std_logic_vector( 7 downto 0);
	signal MemFIFO_InData:	std_logic_vector(31 downto 0);
	signal MemFIFO_Write:		std_logic;
	signal MemFIFO_Full:		std_logic;

	signal RXreadparams_done: 	std_logic;
	signal RXargs_done: 	std_logic;
	signal RXmemF_done: 	std_logic;

	signal bFROM_GC_Read: std_logic;
	signal FROM_GC_Done: std_logic;

	signal FROM_GC_set		: std_logic_vector(0 to 8);
	signal FROM_GC_shreg	: std_logic_vector(0 to 8);
	signal FROM_GC_valid	: std_logic_vector(0 to 8);

	-- state machine
	type rx_read_state is  (rrIDLE,rrParams,rrArgs,rrMemF,rrDone,rrDirectReply,rrCodeLoad);
	signal rr_state, next_rr_state: rx_read_state;
	signal DirectReply : std_logic;
	
	signal DirectReply_Write : std_logic;
	signal DirectReply_InDataCtrl : std_logic_vector(32 downto 0);
	signal DirectReply_OutDataCtrl : std_logic_vector(32 downto 0);
	signal DirectReply_InData : std_logic_vector(31 downto 0);
	signal DirectReply_InCtrl : std_logic;
	signal DirectReply_Full : std_logic;
	signal RXdirectReply_done : std_logic;
	signal RXdirectReply_cnt : std_logic_vector(4 downto 0);
	signal empty_slots : std_logic;

	signal CodeLoad : std_logic;

begin
	
	FROM_GC_Clk <= clock;

	-- Read preparation
	FROMGC_set:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or FROM_GC_Done='1') then
				FROM_GC_set   <= (others=>'0');
			elsif (bFROM_GC_Read='1') then
				FROM_GC_set   <= '1' & FROM_GC_set  (0 to 7);
			end if;
		end if;
	end process;

	FROMGC_valid_shreg:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or FROM_GC_Done='1') then
				FROM_GC_shreg <= (0=>'1',others=>'0');
			elsif (bFROM_GC_Read='1') then
				FROM_GC_shreg <= '0' & FROM_GC_shreg(0 to 7);
			end if;
		end if;
	end process;
	FROM_GC_valid <= FROM_GC_shreg when (FROM_GC_Exists='1') else (others=>'0');

	bFROM_GC_Read <= '1' when (FROM_GC_valid(0 to 1)/="00") -- always
						or ((FROM_GC_valid(2 to 2)/="0") and isMediumAM(RXfunc))
						or ((FROM_GC_valid(2 to 4)/="000") and isLongAM(RXfunc))
						or ((FROM_GC_valid(2 to 7)/="000000") and isLongStridedAM(RXfunc))
						or RXargwrite='1'
						or DirectReply_Write='1'
						or RXmemFwrite='1'
						or (CodeLoad_Read='1')
						else '0';
	FROM_GC_Read <= bFROM_GC_Read;

	RXfunc_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXfunc_valid <= '0';
				RXfunc <= (others=>'0');
			elsif (RXfunc_write='1') then          RXfunc_valid <= '1';
				RXfunc <= FROM_GC_Data(1 to 7); end if; end if;
	end process;
	RXfunc_write <= FROM_GC_valid(0);

	RXnum_args_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXnum_args_valid <= '0';
				RXnum_args <= (others=>'0');
			elsif (RXnum_args_write='1') then          RXnum_args_valid <= '1';
				RXnum_args <= FROM_GC_Data(8 to 15); end if; end if;
	end process;
	RXnum_args_write <= FROM_GC_valid(0);

	RXhcode_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXhcode_valid <= '0';
				RXhcode <= (others=>'0');
			elsif (RXhcode_write='1') then          RXhcode_valid <= '1';
				RXhcode <= FROM_GC_Data(16 to 31); end if; end if;
	end process;
	RXhcode_write <= FROM_GC_valid(0);

	RXtoken_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXtoken_valid <= '0';
				RXtoken <= (others=>'0');
			elsif (RXtoken_write='1') then          RXtoken_valid <= '1';
				RXtoken <= FROM_GC_Data(22 to 31); end if; end if;
	end process;
	RXtoken_write <= FROM_GC_valid(1);

	-- variable positions
	RXbsize_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXbsize_valid <= '0';
				RXbsize <= (others=>'0');
			elsif (RXbsize_write='1') then          RXbsize_valid <= '1';
				RXbsize <= FROM_GC_Data; 
			end if;
		end if;
	end process;


	RXbsize_write <= (RXfunc_valid) when isShortAM(RXfunc)
					else (FROM_GC_valid(2)) when (isMediumAM(RXfunc) or isLongxAM(RXfunc))
					else '0';

	-- compute number of 32-bit memwords in the network packet (no byte offset)
	RXdummy <= FROM_GC_Data(14 to 31)+"000000000000000011"; -- bsize + 3 for overhang
	RXmempack_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or RXmemwords_packed_read='1') then RXmemwords_packed_valid <= '0';
				RXmemwords_packed <= (others=>'0');
			elsif (RXmemwords_packed_write='1') then          RXmemwords_packed_valid <= '1';
				RXmemwords_packed <= RXdummy(17 downto 2); -- div 4
			end if;
		end if;
	end process;
	RXmemwords_packed_write <= RXbsize_write;
	RXmemwords_packed_read <= '1' when rr_state = rrDone else '0';

	RXbufaddrL_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXbufaddrL_valid <= '0';
				RXbufaddrL <= (others=>'0');
			elsif (RXbufaddrL_write='1') then          RXbufaddrL_valid <= '1';
				RXbufaddrL <= FROM_GC_Data; end if;	end if;
	end process;

	RXbufaddrL_write <= (RXbufaddrL_valid and FROM_GC_valid(3)) when isLongAM(RXfunc) else
						(RXbufaddrL_valid and FROM_GC_valid(5)) when isLongStridedAM(RXfunc) else
						 '0';		
	
	RXbufaddrH_proc:process(clock) begin
		if rising_edge(clock) then
			if (reset='1' or empty_slots='1') then RXbufaddrH_valid <= '0';
				RXbufaddrH <= (others=>'0');
			elsif (RXbufaddrH_write='1') then          RXbufaddrH_valid <= '1';
				RXbufaddrH <= FROM_GC_Data; end if;	end if;
	end process;

	RXbufaddrH_write <= (RXbufaddrH_valid and FROM_GC_valid(4)) when isLongAM(RXfunc) else
						(RXbufaddrH_valid and FROM_GC_valid(6)) when isLongStridedAM(RXfunc) else
						 '0';		
	
	--TODO: Outputs for SizeX,SizeY,StrideY in case of LongStridedAM
	--TODO: LongV handling

	-- argument transfer
	RXargwrite <= '1' when (rr_state=rrArgs and FROM_GC_Exists='1' and ArgFIFO_Full='0') else '0';

	rx_argument_cnt:process(clock)
	begin
		if rising_edge(clock) then
			if (reset='1' or rr_state=rrIDLE) then
				RXarg_cnt <= (others=>'0');
			elsif (RXargwrite='1') then
				RXarg_cnt <= RXarg_cnt + 1;
			end if;
		end if;
	end process;
	RXargs_done <= '1' when (RXargwrite='1' and (RXarg_cnt+1=RXnum_args)) else '0';

	-- DirectReply transfer
	DirectReply_Write <= '1' when (rr_state=rrDirectReply and FROM_GC_Exists='1' and DirectReply_Full='0') else '0';

	rx_directReply_cnt:process(clock)
	begin
		if rising_edge(clock) then
			if (reset='1' or rr_state=rrIDLE) then
				RXdirectReply_cnt <= (others=>'0');
			elsif (DirectReply_Write='1') then
				RXdirectReply_cnt <= RXdirectReply_cnt + 1;
			end if;
		end if;
	end process;
	RXdirectReply_done <= '1' when (DirectReply_Write='1' and (RXdirectReply_cnt+1=RXnum_args)) else '0';

	Arg_FIFO: srl_fifo
	GENERIC MAP(C_DATA_BITS => 32)
	PORT MAP(
		Clk => clock,
		Reset => reset,
		FIFO_Write => ArgFIFO_Write,
		Data_In => ArgFIFO_InData(31 downto 0),
		FIFO_Read =>  ArgFIFO_Read,
		Data_Out => ArgFIFO_OutData(31 downto 0),
		FIFO_Full => ArgFIFO_Full,
		Data_Exists => ArgFIFO_Exists
	);
	ArgFIFO_InData <= FROM_GC_Data;
	ArgFIFO_Write <= RXargwrite;

	-- Medium Message MemFIFO transfer
	RXmemFwrite <= '1' when (rr_state=rrMemF and FROM_GC_Exists='1' and MemFIFO_Full='0') else '0';

	rx_medFIFOmem_cnt:process(clock)
	begin
		if rising_edge(clock) then
			if (reset='1' or rr_state=rrIDLE) then
				RXmemF_cnt <= (others=>'0');
			elsif (RXmemFwrite='1') then
				RXmemF_cnt <= RXmemF_cnt + 1;
			end if;
		end if;
	end process;
	RXmemF_done <= '1' when (RXmemFwrite='1' and (RXmemF_cnt+1=RXmemwords_packed)) else '0';

	Mem_FIFO: srl_fifo32
	GENERIC MAP(C_DATA_BITS => 32)
	PORT MAP(
		Clk => clock,
		Reset => reset,
		FIFO_Write => MemFIFO_Write,
		Data_In => MemFIFO_InData(31 downto 0),
		FIFO_Read =>  MemFIFO_Read,
		Data_Out => MemFIFO_OutData(31 downto 0),
		FIFO_Full => MemFIFO_Full,
		Data_Exists => MemFIFO_Exists
	);
	MemFIFO_InData <= FROM_GC_Data;
	MemFIFO_Write <= RXmemFwrite;

	-- output signals
	rx_func <= RXfunc;
	rx_num_args <= RXnum_args;
	rx_hcode <= RXhcode;
	rx_token <= RXtoken;
	rx_bsize <= RXbsize;
	rx_bufaddrL <= RXbufaddrL;
	rx_bufaddrH <= RXbufaddrH;

	-- Code Load mechanism
	CodeLoad <= '1' when (RXhcode(7 downto 2)="010000" and RXhcode_valid='1')
						else '0'; --Handler codes 64..67
	CodeLoad_OutData <= FROM_GC_Data;
	CodeLoad_Exists  <= FROM_GC_Exists when (rr_state=rrCodeLoad) else '0';
	

	-- Direct Reply Mechanism
	DirectReply <= '1' when (RXhcode(7 downto 3)="11110" and RXhcode_valid='1')
						else '0'; --Handler codes 240..247

	DirectReply_FIFO: srl_fifo32
	GENERIC MAP(C_DATA_BITS => 33)
	PORT MAP(
		Clk => clock,
		Reset => reset,
		FIFO_Write =>	DirectReply_Write,
		Data_In =>		DirectReply_InDataCtrl,
		FIFO_Read =>	DirectReply_Read,
		Data_Out =>		DirectReply_OutDataCtrl,
		FIFO_Full =>	DirectReply_Full,
		Data_Exists =>	DirectReply_Exists
	);
	DirectReply_InCtrl <= RXdirectReply_done;
	DirectReply_InData <= ("011" & FROM_GC_Data(3 to 15) & "000000" & RXToken)--forcing Reply, Async bits; using received Token
							when (RXdirectReply_cnt=0)
							else FROM_GC_Data;
	DirectReply_InDataCtrl <= DirectReply_InCtrl & DirectReply_InData;
	
	DirectReply_OutData <= DirectReply_OutDataCtrl(31 downto 0);
	DirectReply_OutCtrl <= DirectReply_OutDataCtrl(32);
	
	-- State Machine for reading from NetIF pipe
	rxread:process (rr_state, reset,
					FROM_GC_Exists, rx_ReadParams,
					RXnum_args, RXfunc, RXfunc_valid,
					RXreadparams_done, RXargs_done, RXmemF_done, DirectReply, RXdirectReply_done, CodeLoad)
	variable next_state:rx_read_state;
	begin
		next_state := rr_state;
		if (reset='1') then
			next_state := rrIDLE;
		else
			case rr_state is
				when rrIDLE => -- wait for new data, finished tx_write
					if FROM_GC_Exists='1' then
						next_state := rrParams; end if;
				when rrParams => -- Destptr Low
					if (RXreadparams_done='1') then
						if (DirectReply='1') then
							next_state := rrDirectReply;
						elsif (CodeLoad='1') then
							next_state := rrCodeLoad;
						elsif ((RXnum_args/=0)) then -- arguments exist
							next_state := rrArgs;
						elsif (isMediumAM(RXfunc)) then
							next_state := rrMemF;
						else					-- no args, no mem transfer
							next_state := rrDone;
						end if;
					end if;
				when rrArgs => -- argument transfer
					if (RXargs_done='1') then
						if (isMediumAM(RXfunc)) then
							next_state := rrMemF;
						else					-- no mem transfer
							next_state := rrDone;
						end if;
					end if;
				when rrMemF => -- memory transfer into Handler Call FIFO
					if (RXmemF_done='1') then
						next_state := rrDone; end if;
				when rrDirectReply => -- DirectReply argument transfer
					if (RXdirectReply_done='1') then
						next_state := rrIDLE;
					end if;
				when rrCodeLoad =>
					if (RXfunc_valid='0' or rx_ReadParams='1') then -- Params read
						next_state := rrIDLE; end if;
				when rrDone =>
					if (RXfunc_valid='0' or rx_ReadParams='1') then -- Params read
						next_state := rrIDLE; end if;
--				when others =>
			end case;
		end if;
		next_rr_state <= next_state;
	end process;
	rr_state <= next_rr_state when rising_edge(clock) else rr_state;

	FROM_GC_Done <= '1' when next_rr_state=rrIDLE else '0';
	rx_Done <= '1' when rr_state=rrDone else '0';

	RXreadparams_done <= (FROM_GC_valid(1) or FROM_GC_set(1)) when isShortAM(RXfunc)
					else (FROM_GC_valid(2) or FROM_GC_set(2)) when isMediumAM(RXfunc)
					else (FROM_GC_valid(4) or FROM_GC_set(4)) when isLongAM(RXfunc)
					else (FROM_GC_valid(7) or FROM_GC_set(7)) when isLongStridedAM(RXfunc)
					else '0';

	empty_slots <= '1' when (rx_ReadParams='1' or next_rr_state=rrIDLE) else '0';

	rx_ParamsReady <= '1' when (rr_state=rrArgs or rr_state=rrMemF or 
								rr_state=rrDone or rr_state=rrCodeLoad) else '0';

	-- generic token return 
	-- assumption HANDLER RETURN FSL never full -> GAScore processes return tokens instantly
	HANDLER_RETURN_Write <= '1' when (next_rr_state=rrIDLE) else '0';
	HANDLER_RETURN_Data <= X"00000" & "00" & RXtoken;
	HANDLER_RETURN_Control <= '0';
	HANDLER_RETURN_Clk <= clock;
	
end architecture archie;
