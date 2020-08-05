library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

entity stencil_core_tb is
end entity stencil_core_tb;

architecture RTL of stencil_core_tb is
	
	component stencil_core
		generic(
		        C_M_AXI_ADDR_WIDTH : integer := 32;
		        C_M_AXI_DATA_WIDTH : integer := 32;
				C_MAX_BURST_LEN   : integer := 16;
				C_BRAMS_PER_ROW: integer := 8;
				C_MEM_BASEADDR: std_logic_vector(31 downto 0) := X"80000000";
				C_MEM_SIZE: std_logic_vector(31 downto 0)     := X"80000000";--X"00000400";--
				C_CLEAR_MEM : integer := 0
				);
		port(HOST_Clk               : in  std_logic;
			 HOST_Rst               : in  std_logic;
			 FROM_GC_Clk            : out std_logic;
			 FROM_GC_Read           : out std_logic;
			 FROM_GC_Data           : in  std_logic_vector(0 to 31);
			 FROM_GC_Control        : in  std_logic;
			 FROM_GC_Exists         : in  std_logic;
			 TO_GC_Clk              : out std_logic;
			 TO_GC_Write            : out std_logic;
			 TO_GC_Data             : out std_logic_vector(0 to 31);
			 TO_GC_Control          : out std_logic;
			 TO_GC_Full             : in  std_logic;
			 AM_RETURN_Clk          : out std_logic;
			 AM_RETURN_Read         : out std_logic;
			 AM_RETURN_Data         : in  std_logic_vector(0 to 31);
			 AM_RETURN_Control      : in  std_logic;
			 AM_RETURN_Exists       : in  std_logic;
			 HANDLER_RETURN_Clk     : out std_logic;
			 HANDLER_RETURN_Write   : out std_logic;
			 HANDLER_RETURN_Data    : out std_logic_vector(0 to 31);
			 HANDLER_RETURN_Control : out std_logic;
			 HANDLER_RETURN_Full    : in  std_logic;
			 m_axi_aclk             : in  std_logic;
			 m_axi_aresetn          : in  std_logic;
			 md_error               : out std_logic;
			 m_axi_awready          : in  std_logic;
			 m_axi_awvalid          : out std_logic;
			 m_axi_awaddr           : out std_logic_vector(C_M_AXI_ADDR_WIDTH - 1 downto 0);
			 m_axi_awlen            : out std_logic_vector(7 downto 0);
			 m_axi_awsize           : out std_logic_vector(2 downto 0);
			 m_axi_awburst          : out std_logic_vector(1 downto 0);
			 m_axi_awprot           : out std_logic_vector(2 downto 0);
			 m_axi_awcache          : out std_logic_vector(3 downto 0);
			 m_axi_wready           : in  std_logic;
			 m_axi_wvalid           : out std_logic;
			 m_axi_wdata            : out std_logic_vector(C_M_AXI_DATA_WIDTH - 1 downto 0);
			 m_axi_wstrb            : out std_logic_vector((C_M_AXI_DATA_WIDTH / 8) - 1 downto 0);
			 m_axi_wlast            : out std_logic;
			 m_axi_bready           : out std_logic;
			 m_axi_bvalid           : in  std_logic;
			 m_axi_bresp            : in  std_logic_vector(1 downto 0);
			 m_axi_arready          : in  std_logic;
			 m_axi_arvalid          : out std_logic;
			 m_axi_araddr           : out std_logic_vector(C_M_AXI_ADDR_WIDTH - 1 downto 0);
			 m_axi_arlen            : out std_logic_vector(7 downto 0);
			 m_axi_arsize           : out std_logic_vector(2 downto 0);
			 m_axi_arburst          : out std_logic_vector(1 downto 0);
			 m_axi_arprot           : out std_logic_vector(2 downto 0);
			 m_axi_arcache          : out std_logic_vector(3 downto 0);
			 m_axi_rready           : out std_logic;
			 m_axi_rvalid           : in  std_logic;
			 m_axi_rdata            : in  std_logic_vector(C_M_AXI_DATA_WIDTH - 1 downto 0);
			 m_axi_rresp            : in  std_logic_vector(1 downto 0);
			 m_axi_rlast            : in  std_logic);
	end component stencil_core;
	
	-- data size-dependent constants
	
	
--	constant BurstSize: integer := 16;
--	constant RowBRAMs: integer := 1;
--	constant RowBufferDepth: integer := RowBRAMs*1024;-------1024; -- 1 BRAM = 4kbyte = 1kWords
	
--	constant Xsize_aura: integer := Xsize+2; -- 2002
--	constant Ysize_aura: integer := Ysize+2; -- 2002
	
--	constant Yoffset: integer := (1 + ((Xsize) / BurstSize) + 1) * BurstSize; -- 2032 
	
--	constant Row_Xmin: integer :=  BurstSize; -- 16
--	constant Row_Xmax: integer :=  Row_Xmin + Xsize - 1; -- 2015  

--	constant Row_Xmin_aura: integer :=  BurstSize-1; -- 15 
--	constant Row_Xmax_aura: integer :=  Row_Xmin_aura + Xsize_aura - 1; -- 2017  

--	constant Ymin: integer :=  1;
--	constant Ymax: integer :=  Ymin + Ysize - 1; -- 2000  

--	constant Ymin_aura: integer :=  0;
--	constant Ymax_aura: integer :=  Ymin_aura + Ysize_aura - 1; -- 2001  
	
	signal Reset : std_logic;
	signal Clock : std_logic;
	
	signal count : std_logic_vector(31 downto 0);
	signal WEreq : std_logic;
	signal RDreq : std_logic;
	signal FROM_GC_Clk : std_logic;
	signal FROM_GC_Read : std_logic;
	signal FROM_GC_Data : std_logic_vector(0 to 31);
	signal FROM_GC_Control : std_logic;
	signal FROM_GC_Exists : std_logic;
	signal TO_GC_Clk : std_logic;
	signal TO_GC_Write : std_logic;
	signal TO_GC_Data : std_logic_vector(0 to 31);
	signal TO_GC_Control : std_logic;
	signal TO_GC_Full : std_logic;
	signal AM_RETURN_Clk : std_logic;
	signal AM_RETURN_Read : std_logic;
	signal AM_RETURN_Data : std_logic_vector(0 to 31);
	signal AM_RETURN_Control : std_logic;
	signal AM_RETURN_Exists : std_logic;
	signal HANDLER_RETURN_Clk : std_logic;
	signal HANDLER_RETURN_Write : std_logic;
	signal HANDLER_RETURN_Data : std_logic_vector(0 to 31);
	signal HANDLER_RETURN_Control : std_logic;
	signal HANDLER_RETURN_Full : std_logic;
	signal m_axi_aclk : std_logic;
	signal m_axi_aresetn : std_logic;
	signal md_error : std_logic;
	signal m_axi_awready : std_logic;
	signal m_axi_awvalid : std_logic;
	signal m_axi_awaddr : std_logic_vector(31 downto 0);
	signal m_axi_awlen : std_logic_vector(7 downto 0);
	signal m_axi_awsize : std_logic_vector(2 downto 0);
	signal m_axi_awburst : std_logic_vector(1 downto 0);
	signal m_axi_awprot : std_logic_vector(2 downto 0);
	signal m_axi_awcache : std_logic_vector(3 downto 0);
	signal m_axi_wready : std_logic;
	signal m_axi_wvalid : std_logic;
	signal m_axi_wdata : std_logic_vector(31 downto 0);
	signal m_axi_wstrb : std_logic_vector(3 downto 0);
	signal m_axi_wlast : std_logic;
	signal m_axi_bready : std_logic;
	signal m_axi_bvalid : std_logic;
	signal m_axi_bresp : std_logic_vector(1 downto 0);
	signal m_axi_arready : std_logic;
	signal m_axi_arvalid : std_logic;
	signal m_axi_araddr : std_logic_vector(31 downto 0);
	signal m_axi_arlen : std_logic_vector(7 downto 0);
	signal m_axi_arsize : std_logic_vector(2 downto 0);
	signal m_axi_arburst : std_logic_vector(1 downto 0);
	signal m_axi_arprot : std_logic_vector(2 downto 0);
	signal m_axi_arcache : std_logic_vector(3 downto 0);
	signal m_axi_rready : std_logic;
	signal m_axi_rvalid : std_logic;
	signal m_axi_rdata : std_logic_vector(31 downto 0);
	signal m_axi_rresp : std_logic_vector(1 downto 0);
	signal m_axi_rlast : std_logic;
	signal lencount : std_logic_vector(7 downto 0);
	
	signal X: std_logic_vector(15 downto 0);
	signal Y: std_logic_vector(15 downto 0);
	
	
	   
	subtype std32 is std_logic_vector(31 downto 0);
	type stimuli is array (0 to 128) of std32;
	
	signal AM_data : stimuli;
	signal AM_numwords : integer := 0;
	signal AM_count : integer := 0;
	signal AM_transfer : std_logic;
	
	
	function PAMS_NOP return std_logic_vector is
	begin return X"00000000"; end function PAMS_NOP;
	
	function PAMS_WAITFOR(	ctrlinputs : in std_logic_vector(3 downto 0) := X"0";
							msgctrs    : in std_logic_vector(3 downto 0) := X"0";
							trnsfrctrs : in std_logic_vector(3 downto 0) := X"0";
							timers     : in std_logic_vector(1 downto 0) := "00";
							AMreturn   : in std_logic_vector(0 downto 0) := "0";
							codeloaded : in std_logic_vector(0 downto 0) := "0"
							) return std_logic_vector is
	begin
		return "00000" & "000" & X"0" & X"0" & ctrlinputs & msgctrs & trnsfrctrs & timers & AMreturn & codeloaded;
	end function PAMS_WAITFOR;
	
	constant flag_POS: std_logic_vector(3 downto 0) := "1000";	constant flag_NEG: std_logic_vector(3 downto 0) := "0100";
	constant flag_ZERO: std_logic_vector(3 downto 0) := "0010";	constant flag_NOTZERO: std_logic_vector(3 downto 0) := "0001";
	constant flag_ALWAYS: std_logic_vector(3 downto 0) := "0000";
	function PAMS_BRANCH(	flagtests  : in std_logic_vector(3 downto 0) := X"0";
							address    : in integer := 0
							) return std_logic_vector is
	begin
		return "00010" & "000" & X"00" & flagtests & CONV_STD_LOGIC_VECTOR(address,12);
	end function PAMS_BRANCH;

	function PAMS_SETNODEID(	newid  : in std_logic_vector(15 downto 0) := X"0000"
								) return std_logic_vector is
	begin
		return "00011" & "000" & X"00" & newid;
	end function PAMS_SETNODEID;

	function PAMS_ADJ_TIMER return std_logic_vector is
	begin return X"24000000"; end function PAMS_ADJ_TIMER;

	function PAMS_TIMER_THR(	threshold_mask  : in std_logic_vector(1 downto 0) := "00"
								) return std_logic_vector is
	begin
		return "00101" & "100" & X"00000" & "00" & threshold_mask;
	end function PAMS_TIMER_THR;

	function PAMS_DATA_THR( handlerID    : in std_logic_vector(7 downto 0) := X"00";
							countermask : in std_logic_vector(3 downto 0) := X"0"
							) return std_logic_vector is
	begin
		return "00110" & "100" & X"000" & handlerID & countermask ;
	end function PAMS_DATA_THR;

	function PAMS_MSG_THR(  handlerID    : in std_logic_vector(7 downto 0) := X"00";
							countermask : in std_logic_vector(3 downto 0) := X"0"
							) return std_logic_vector is
	begin
		return "00111" & "100" & X"000" & handlerID & countermask ;
	end function PAMS_MSG_THR;

	function PAMS_TO_AMSEND(endAM  : in std_logic_vector(0 downto 0) := "0";
							source : in integer := 0;
							words  : in integer := 0
							) return std_logic_vector is
	begin
		return "00001" & "01" & endAM & CONV_STD_LOGIC_VECTOR(source,4) & X"00" & CONV_STD_LOGIC_VECTOR(words,12);
	end function PAMS_TO_AMSEND;
	
	function PAMS_CTRLOUT(	ctrloutputs : in std_logic_vector(15 downto 0) := X"0000") return std_logic_vector is
	begin
		return "01000" & "000" & X"0" & X"0" & ctrloutputs;
	end function PAMS_CTRLOUT;
	
	function PAMS_TO_CORE(	source : in integer := 0;
							words : in integer := 0
						) return std_logic_vector is
	begin
		return "01001" & "010" & CONV_STD_LOGIC_VECTOR(source,4) & X"00" & CONV_STD_LOGIC_VECTOR(words,12);
	end function PAMS_TO_CORE;

	type alu_op is (op_LOAD,op_ADD,op_SUB,op_AND);
	function PAMS_ALU(	source : in integer := 0;
						op : in alu_op;
						accumulator : in std_logic_vector(1 downto 0) := "00";
						words : in integer := 0
						) return std_logic_vector is
		variable opbits: std_logic_vector(1 downto 0);
	begin
		case op is
			when op_LOAD => opbits := "00";	when op_ADD  => opbits := "01";
			when op_SUB  => opbits := "10";	when op_AND  => opbits := "11";
		end case;
		return "01010" & "010" & CONV_STD_LOGIC_VECTOR(source,4) & X"0" & opbits & accumulator & CONV_STD_LOGIC_VECTOR(words,12);
	end function PAMS_ALU;

	function PAMS_RESETWAIT(ctrlinputs : in std_logic_vector(11 downto 0) := X"000";
							msgctrs    : in std_logic_vector(3 downto 0) := X"0";
							trnsfrctrs : in std_logic_vector(3 downto 0) := X"0";
							timers     : in std_logic_vector(1 downto 0) := "00";
							AMreturn   : in std_logic_vector(0 downto 0) := "0";
							codeloaded : in std_logic_vector(0 downto 0) := "0"
							) return std_logic_vector is
	begin
		return "01011" & "000" & X"0" & X"0" & ctrlinputs & msgctrs & trnsfrctrs & timers & AMreturn & codeloaded;
	end function PAMS_RESETWAIT;
		
		
		
		
	constant XSIZE: integer := 1024;
	constant YSIZE: integer := 16;
	
begin
	
		host_stimuli:process
		begin
			AM_data(0) <= X"00000000";
			AM_numwords <= 0;
			wait for 100 ns;
			AM_data( 0) <= X"12000040";  -- MediumFIFO AM, 0 args, handler code 64 -> load PSM code at start
			AM_data( 1) <= X"00000000";  -- token = 0 
			AM_data( 2) <= X"FFFFFFFF";  -- bsize: replaced later
			--
			AM_data( 3) <= PAMS_NOP;  -- NOP
			AM_data( 4) <= PAMS_TO_CORE(source => 0, words => 10);  -- Write 4 words from code to core
			AM_data( 5) <= CONV_STD_LOGIC_VECTOR(XSIZE,32);  -- XSIZE 80 
			AM_data( 6) <= CONV_STD_LOGIC_VECTOR(YSIZE,32);  -- YSIZE 5
			AM_data( 7) <= X"FFFFFFFF";  -- Pole 1 coordinates - no pole
			AM_data( 8) <= X"AAAAAAAA";  -- Pole 1 value
			AM_data( 9) <= X"FFFFFFFF";  -- Pole 2 coordinates - no pole
			AM_data(10) <= X"BBBBBBBB";  -- Pole 2 value
			AM_data(11) <= X"FFFFFFFF";  -- Pole 3 coordinates - no pole
			AM_data(12) <= X"CCCCCCCC";  -- Pole 3 value
			AM_data(13) <= X"FFFFFFFF";  -- Pole 4 coordinates - no pole
			AM_data(14) <= X"DDDDDDDD";  -- Pole 4 value
			
			AM_data(15) <= PAMS_WAITFOR(ctrlinputs => X"2"); -- Wait for Control_In(1)--; 
			AM_data(16) <= PAMS_CTRLOUT(ctrloutputs => X"0001");				-- Assert Control_Out(0) 
			AM_data(17) <= PAMS_NOP;							-- NOP 
			AM_data(18) <= PAMS_WAITFOR(ctrlinputs => X"1"); -- Wait for Control_In(0) 
			AM_data(19) <= PAMS_TO_AMSEND(endAM => "1", source => 3, words => 3);  -- Write 4 words from core data feed to AM pipe, End AM request

			AM_data(20)  <= PAMS_ALU(source => 0, op => op_LOAD, accumulator => "00", words => 1); -- Load A with 0x00
			AM_data(21)  <= X"00000000";  --  
			AM_data(22) <= PAMS_BRANCH(flagtests => (flag_ZERO), address => 11);  -- Branch to 1 if loaded zero
			AM_numwords <= 23;
			AM_data(2) <= CONV_STD_LOGIC_VECTOR(4*(23-3),32);


--			wait for 17390 ns;
--			AM_data(22) <= X"010700F0";  -- Short AM, 07 args, HandlerCode 240 (->DirectReply) 
--			AM_data(23) <= X"00000033";  -- Token 33
--			AM_data(24) <= X"00000000";  -- Arg 0: Reply Header - kept 0 here to see Request Word 0 modifications 
--			AM_data(25) <= X"AA000001";  -- Arg 1: 
--			AM_data(26) <= X"AA000002";  -- Arg 2: 
--			AM_data(27) <= X"AA000003";  -- Arg 3: 
--			AM_data(28) <= X"AA000004";  -- Arg 4: 
--			AM_data(29) <= X"AA000005";  -- Arg 5: 
--			AM_data(30) <= X"AA000006";  -- Arg 6: 
--			AM_numwords <= 31;
			
			wait;
		end process;
--		AM_data(3)  <= PAMS_NOP;  -- NOP
--		AM_data(4)  <= PAMS_CTRLOUT(ctrloutputs => X"0001");				-- Assert Control_Out(0) 
--		AM_data(5)  <= PAMS_NOP;							-- NOP 
--		AM_data(6)  <= PAMS_WAITFOR(ctrlinputs => X"1"); -- Wait for Control_In(0) 
--		AM_data(7)  <= PAMS_ALU(source => 0, op => op_LOAD, accumulator => "00", words => 1); -- Load A with 0x42
--		AM_data(8)  <= X"00000042";  --  
--		AM_data(9)  <= PAMS_ALU(source => 0, op => op_ADD, accumulator => "00", words => 1);  -- Add 0x11 to A
--		AM_data(10) <= X"00000011";  -- 
--		AM_data(11) <= PAMS_ALU(source => 0, op => op_SUB, accumulator => "00", words => 1);  -- Sub 0x53 from A
--		AM_data(12) <= X"00000053";  -- 
--		AM_data(13) <= PAMS_ALU(source => 0, op => op_SUB, accumulator => "00", words => 1);  -- Sub 0x1 from A
--		AM_data(14) <= X"00000001";  -- 
--		AM_data(15) <= PAMS_TO_AMSEND(endAM => "1", source => 3, words => 3);  -- Write 4 words from core data feed to AM pipe, End AM request
--		AM_data(16) <= PAMS_TO_CORE(source => 0, words => 10);  -- Write 4 words from code to core
--		AM_data(17) <= X"00000050";  -- XSIZE 80 
--		AM_data(18) <= X"00000005";  -- YSIZE 5
--		AM_data(19) <= X"FFFFFFFF";  -- Pole 1 coordinates - no pole
--		AM_data(20) <= X"AAAAAAAA";  -- Pole 1 value
--		AM_data(21) <= X"FFFFFFFF";  -- Pole 2 coordinates - no pole
--		AM_data(22) <= X"BBBBBBBB";  -- Pole 2 value
--		AM_data(23) <= X"FFFFFFFF";  -- Pole 3 coordinates - no pole
--		AM_data(24) <= X"CCCCCCCC";  -- Pole 3 value
--		AM_data(25) <= X"FFFFFFFF";  -- Pole 4 coordinates - no pole
--		AM_data(26) <= X"DDDDDDDD";  -- Pole 4 value
--		AM_data(27) <= PAMS_BRANCH(flagtests => (flag_ALWAYS), address => 0);  -- Branch to 0 if last op not zero
--		AM_numwords <= 28; AM_data(2) <= CONV_STD_LOGIC_VECTOR(4*(28-3),32);
		
		
	host_proc: process(clock)
	begin
		if (rising_edge(clock)) then
			if (reset='1') then-- or AM_RETURN_Exists='1') then
				AM_count <= 0;
			elsif (AM_transfer='1') then
				AM_count <= AM_count + 1;
			end if;
		end if;
	end process;
	AM_transfer <= FROM_GC_Exists and FROM_GC_Read;

	
	FROM_GC_Exists <= '1' when (AM_count < AM_numwords) else '0';
	FROM_GC_Control <= '0';
	FROM_GC_Data <= AM_data(AM_count);
	
	
	TO_GC_Full <= '0';
	
	AM_RETURN_Exists <= '0';
	AM_RETURN_Control <= '0';
	AM_RETURN_Data <= (others => '0');
	
	HANDLER_RETURN_Full <= '0';

	dut: stencil_core
		generic map(C_M_AXI_ADDR_WIDTH => 32,
			        C_M_AXI_DATA_WIDTH => 32,
			        C_MAX_BURST_LEN    => 16,
			        C_BRAMS_PER_ROW => 8,
			        C_MEM_BASEADDR => X"80000000",
					C_MEM_SIZE => X"80000000",--X"00000400",--
					C_CLEAR_MEM => 0
					)
		port map(HOST_Clk               => clock,
			     HOST_Rst               => reset,
			     FROM_GC_Clk            => FROM_GC_Clk,
			     FROM_GC_Read           => FROM_GC_Read,
			     FROM_GC_Data           => FROM_GC_Data,
			     FROM_GC_Control        => FROM_GC_Control,
			     FROM_GC_Exists         => FROM_GC_Exists,
			     TO_GC_Clk              => TO_GC_Clk,
			     TO_GC_Write            => TO_GC_Write,
			     TO_GC_Data             => TO_GC_Data,
			     TO_GC_Control          => TO_GC_Control,
			     TO_GC_Full             => TO_GC_Full,
			     AM_RETURN_Clk          => AM_RETURN_Clk,
			     AM_RETURN_Read         => AM_RETURN_Read,
			     AM_RETURN_Data         => AM_RETURN_Data,
			     AM_RETURN_Control      => AM_RETURN_Control,
			     AM_RETURN_Exists       => AM_RETURN_Exists,
			     HANDLER_RETURN_Clk     => HANDLER_RETURN_Clk,
			     HANDLER_RETURN_Write   => HANDLER_RETURN_Write,
			     HANDLER_RETURN_Data    => HANDLER_RETURN_Data,
			     HANDLER_RETURN_Control => HANDLER_RETURN_Control,
			     HANDLER_RETURN_Full    => HANDLER_RETURN_Full,
			     m_axi_aclk             => m_axi_aclk,
			     m_axi_aresetn          => m_axi_aresetn,
			     md_error               => md_error,
			     m_axi_awready          => m_axi_awready,
			     m_axi_awvalid          => m_axi_awvalid,
			     m_axi_awaddr           => m_axi_awaddr,
			     m_axi_awlen            => m_axi_awlen,
			     m_axi_awsize           => m_axi_awsize,
			     m_axi_awburst          => m_axi_awburst,
			     m_axi_awprot           => m_axi_awprot,
			     m_axi_awcache          => m_axi_awcache,
			     m_axi_wready           => m_axi_wready,
			     m_axi_wvalid           => m_axi_wvalid,
			     m_axi_wdata            => m_axi_wdata,
			     m_axi_wstrb            => m_axi_wstrb,
			     m_axi_wlast            => m_axi_wlast,
			     m_axi_bready           => m_axi_bready,
			     m_axi_bvalid           => m_axi_bvalid,
			     m_axi_bresp            => m_axi_bresp,
			     m_axi_arready          => m_axi_arready,
			     m_axi_arvalid          => m_axi_arvalid,
			     m_axi_araddr           => m_axi_araddr,
			     m_axi_arlen            => m_axi_arlen,
			     m_axi_arsize           => m_axi_arsize,
			     m_axi_arburst          => m_axi_arburst,
			     m_axi_arprot           => m_axi_arprot,
			     m_axi_arcache          => m_axi_arcache,
			     m_axi_rready           => m_axi_rready,
			     m_axi_rvalid           => m_axi_rvalid,
			     m_axi_rdata            => m_axi_rdata,
			     m_axi_rresp            => m_axi_rresp,
			     m_axi_rlast            => m_axi_rlast);

	clkproc:process
	begin
		clock <= '1';
		wait for 5 ns;
		clock <= '0';
		wait for 5 ns;
	end process;
	
	rstproc:process
	begin
		reset <= '1';
		wait for 23 ns;
		reset <= '0';
		wait;
	end process;

	
	m_axi_aclk <= clock;
	m_axi_aresetn <= not reset;
	
	------ RD AXI model
	
	m_axi_arready <= '1' when (lencount=0) else '0';
	
	lenproc:process(clock)
	begin
		if (rising_edge(Clock)) then
			if (reset='1') then
				lencount <= "00000000";
			elsif (m_axi_arvalid='1' and m_axi_arready='1') then
				lencount <= m_axi_arlen + 1;
			elsif (m_axi_rvalid='1' and m_axi_rready='1' and lencount/=0) then
				lencount <= lencount - 1;
			end if;	
		end if;
	end process;
	m_axi_rvalid <= '1' when (lencount/=0) else '0';
	m_axi_rlast <= '1' when (lencount=1) else '0';
	
	m_axi_rresp <= (others => '0');
	
	cntproc:process(clock)
	begin
		if (rising_edge(Clock)) then
			if (reset='1') then
				count <= X"00000001";
			elsif (m_axi_rvalid='1' and m_axi_rready='1') then
				count <= count + X"00000001";
			end if;	
		end if;
	end process;
	--m_axi_rdata <= count;
	
	------ WR AXI model
	m_axi_awready <= '1';
	m_axi_wready <= '1';
	m_axi_bvalid <= (m_axi_wready and m_axi_wvalid and m_axi_wlast) when (rising_edge(clock)) else m_axi_bvalid;
	m_axi_bresp <= (others => '0');
	
	
	-- X/Y coordinates
	Xproc:process(clock)
	begin
		if (rising_edge(Clock)) then
			if (reset='1') then
				X <= (others => '0');
			elsif (m_axi_rvalid='1' and m_axi_rready='1') then
				if (X=Xsize+1) then
					X <= (others => '0');
				else
					X <= X + 1;
				end if;
			end if;	
		end if;
	end process;

	Yproc:process(clock)
	begin
		if (rising_edge(Clock)) then
			if (reset='1') then
				Y <= (others => '0');
			elsif (m_axi_rvalid='1' and m_axi_rready='1') then
				if (X=Xsize+1) then
					Y <= Y + 1;
				end if;
			end if;	
		end if;
	end process;
	
	m_axi_rdata <= X"FFFFFFFF" when false--X & Y;-- when ((X=40) and (Y=2 or Y=3 or Y=4)) else X"00000000";
					else CONV_STD_LOGIC_VECTOR(1024,32) when (X=4 and Y=4)
	
	
					else X"00000000";
	
	
	
	
	
	
	
	
	
	
	
	
	
	
end architecture RTL;
