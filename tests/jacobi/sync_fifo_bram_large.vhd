library IEEE;
use IEEE.Std_Logic_1164.all;
use IEEE.numeric_std.all;
use ieee.std_logic_unsigned.all;

Library UNISIM;
use UNISIM.vcomponents.all;

Library UNIMACRO;
use UNIMACRO.vcomponents.all;

entity Sync_FIFO_BRAM_LARGE is
  generic (
    		WordWidth      : integer := 32;
    		BRAMs          : integer := 16
    );
  port (
    Reset : in std_logic;
    Clk   : in std_logic;
    WE      : in  std_logic;
    DataIn  : in  std_logic_vector(WordWidth-1 downto 0);
    Full    : out std_logic;
    Reset_Read : in  std_logic;
    RD      : in  std_logic;
    DataOut : out std_logic_vector(WordWidth-1 downto 0);
    Exists  : out std_logic
    );
end Sync_FIFO_BRAM_LARGE;

architecture VHDL_RTL of Sync_FIFO_BRAM_LARGE is

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

  constant BRAM_SIZE     : integer := 32768;
  constant BRAM_DWIDTH  : integer := WordWidth / BRAMs;
  constant MemDepth     : integer := BRAM_SIZE / BRAM_DWIDTH;
  constant BRAM_AWIDTH  : integer := log2(MemDepth);
  constant BRAM_WEWIDTH : integer := ((BRAM_DWIDTH-1) / 8) + 1;

  type ram_type is array ((2**BRAM_AWIDTH)-1 downto 0) of std_logic_vector ((WordWidth-1) downto 0); 
  signal ram_mem : ram_type;
  signal read_a : std_logic_vector(BRAM_AWIDTH-1 downto 0); 
  signal read_dpra : std_logic_vector(BRAM_AWIDTH-1 downto 0); 
  
  signal Read_Address  : std_logic_vector(0 to BRAM_AWIDTH-1);
  signal Write_Address : std_logic_vector(0 to BRAM_AWIDTH-1);


  signal read_bram_enable : std_logic;
  signal DataOut_BRAM     : std_logic_vector(WordWidth-1 downto 0);
  signal fall_through_data  : std_logic_vector(WordWidth-1 downto 0);
  signal use_fall_through   : std_logic;

  signal read_addr_ptr  : natural range 0 to 2 ** BRAM_AWIDTH-1;
  signal write_addr_ptr : natural range 0 to 2 ** BRAM_AWIDTH-1;

  signal full_i                    : std_logic;
  signal exists_i                  : std_logic;
  signal read_addr_incr            : std_logic;
  signal first_write_on_empty_fifo : std_logic;
  signal last_word                 : std_logic;

  signal fifo_length : natural range 0 to MemDepth;
  
  ------------ Reset_Read extensions
  signal written_length : natural range 0 to MemDepth;
  signal dReset_Read : std_logic;
  signal p1Reset_Read : std_logic;
  signal p2Reset_Read : std_logic;
  
begin
	
    process (Clk)
    begin
      if (Clk'event and Clk = '1') then
        if (Reset = '1') then
        	dReset_Read <= '0';
       		p2Reset_Read <= '0'; -- 2nd cycle of Reset_Read
        else
        	dReset_Read <= Reset_Read;
        	p2Reset_Read <= p1Reset_Read;
        end if;
      end if;
  end process;
  p1Reset_Read <= '1' when (dReset_Read='0' and Reset_Read='1') else '0'; -- 1st cycle of Reset_Read;
	
	

    -- FIFO length handling
    Fifo_Length_Handle : process (Clk)
    begin
      if (Clk'event and Clk = '1') then
        if (Reset = '1') then
        	fifo_length <= 0;
        	written_length <= 0;
        elsif (p1Reset_Read='1') then
        	fifo_length <= written_length;
        else
          -- write and no read => increment length
          -- don't increment length when FULL
          if (WE = '1' and RD = '0' and full_i = '0') then
          	fifo_length <= fifo_length + 1;
          -- read and no write => decrement length
          -- don't decrement length when EMPTY
          elsif (WE = '0' and RD = '1' and exists_i = '1') then
          	fifo_length <= fifo_length - 1;
          end if;
          
          if (WE = '1' and full_i = '0') then
          	written_length <= written_length + 1;
          end if;
        end if;
      end if;
    end process Fifo_Length_Handle;

    ---------------------------------------------------------------------------
    -- Need special handling for BRAM based fifo since there is one extra delay
    -- reading out data from it.
    -- We are pipelining the reading by making read_addr be one read ahead and
    -- are holding the data on the BRAM output by enabling/disabling the BRAM
    -- enable signal
    ---------------------------------------------------------------------------
	
	  -------------------------------------------------------------------------
	  -- Need to detect when writing into an empty FIFO, 
	  -------------------------------------------------------------------------
	  First_Write : process (Clk) is
	  begin  -- process First_Write
	    if Clk'event and Clk = '1' then  -- rising clock edge
	      if Reset = '1' then            -- synchronous reset (active high)
	        first_write_on_empty_fifo <= '0';
	      else
	        first_write_on_empty_fifo <= WE and not exists_i;
	      end if;
	    end if;
	  end process First_Write;
	
	  -------------------------------------------------------------------------
	  -- Read out BRAM contents on the first word written in an empty FIFO and
	  -- all other FIFO read except when the last word is read since the "real"
	  -- FIFO is actually empty at this time since the last word is on the
	  -- output of the BRAM
	  -------------------------------------------------------------------------
	  last_word        <= '1' when (fifo_length = 1) else '0';
	  read_bram_enable <= first_write_on_empty_fifo or (RD and (not last_word or WE))  or p2Reset_Read;
	
	  read_addr_incr <= read_bram_enable;
	
	  -------------------------------------------------------------------------
	  -- The exists flag is now if the BRAM output has valid data and not the
	  -- content of the FIFO
	  -------------------------------------------------------------------------
	  FIFO_Exists_DFF : process (Clk) is
	  begin  -- process FIFO_Exists_DFF
	    if Clk'event and Clk = '1' then  -- rising clock edge
	      if Reset = '1' then            -- synchronous reset (active high)
	        Exists <= '0';
	      else
	        if (first_write_on_empty_fifo = '1' or p2Reset_Read='1') then
	          Exists <= '1';
	        elsif ((RD = '1') and (WE = '0') and (last_word = '1')) then
	          Exists <= '0';
	        end if;
	      end if;
	    end if;
	  end process FIFO_Exists_DFF;
	  
	  -------------------------------------------------------------------------
	  -- Data output with fallthrough
	  -------------------------------------------------------------------------
	  use_fall_through_DFF : process (Clk) is
	  begin  -- process FIFO_Exists_DFF
	    if Clk'event and Clk = '1' then  -- rising clock edge
	      if ((RD and (not WE)) = '1') or (Reset = '1') then            -- synchronous reset (active high)
	        use_fall_through <= '0';
	      elsif (RD and not last_word) = '1' then
	        use_fall_through <= '0';
	      elsif (RD = '1') then
	--          The equation (RD and WE and last_word) = '1' can be reduced to (RD = '1')
	        use_fall_through <= '1';
	      end if;
	    end if;
	  end process use_fall_through_DFF;
	
	  fall_through_data_DFF : process (Clk) is
	  begin  -- process FIFO_Exists_DFF
	    if Clk'event and Clk = '1' then  -- rising clock edge
	      if (RD and WE and last_word) = '1' then
	        fall_through_data <= DataIn;
	      end if;
	    end if;
	  end process fall_through_data_DFF;
	        
	  DataOut <= fall_through_data when (use_fall_through = '1') else DataOut_BRAM;
	  


    -- Set Full and empty flags
    --full_i   <= '1' when (fifo_length = MemDepth) else '0';
    full_i   <= '1' when (written_length = MemDepth) else '0';
    exists_i <= '1' when (fifo_length /= 0)      else '0'; -- no wraparound anymore after #MemDepth reads

    Full <= full_i;

    -- Increment Read Address Pointer
    Read_Addr_Handle : process (Clk)
    begin
      if (Clk'event and Clk = '1') then
        if (Reset = '1' or p1Reset_Read='1') then
          read_addr_ptr <= 0;
        elsif (read_addr_incr = '1') then
          read_addr_ptr <= (read_addr_ptr + 1) mod (2 ** BRAM_AWIDTH);
        end if;
      end if;
    end process Read_Addr_Handle;

    -- Increment Write Address Pointer
    Write_Addr_Handle : process (Clk)
    begin
      if (Clk'event and Clk = '1') then
        if (Reset = '1') then
          write_addr_ptr <= 0;
        elsif (WE = '1') then
          write_addr_ptr <= (write_addr_ptr + 1) mod (2 ** BRAM_AWIDTH);
        end if;
      end if;
    end process Write_Addr_Handle;

    Write_Address <= std_logic_vector(to_unsigned(write_addr_ptr, BRAM_AWIDTH));
    Read_Address  <= std_logic_vector(to_unsigned(read_addr_ptr, BRAM_AWIDTH));
    
--  BRAM:process (Clk) 
--  begin 
--    if (Clk'event and Clk = '1') then 
--      if (WE = '1') then 
--        ram_mem(conv_integer(Write_Address)) <= DataIn; 
--      end if; 
--      read_a <= Write_Address;
--      if (read_bram_enable = '1') then
--        read_dpra <= Read_Address;         
--      end if;
--    end if; 
--  end process; 
--  DataOut_BRAM <= ram_mem(conv_integer(read_dpra)); 

	gen_mkstrb_loop: for k in BRAMS-1 downto 0 generate
		signal DO: std_logic_vector(BRAM_DWIDTH-1 downto 0);
		signal DI: std_logic_vector(BRAM_DWIDTH-1 downto 0);
		signal RDADDR: std_logic_vector(BRAM_AWIDTH-1 downto 0);
		signal WRADDR: std_logic_vector(BRAM_AWIDTH-1 downto 0);
		signal RDEN: std_logic;
		signal WREN: std_logic;
		signal WSTRB: std_logic_vector(BRAM_WEWIDTH-1 downto 0);
	begin
		
		BRAM_SDP_MACRO_k : BRAM_SDP_MACRO
		   generic map (
		      BRAM_SIZE => "36Kb", 
		      DEVICE => "VIRTEX6", 
		      WRITE_WIDTH => BRAM_DWIDTH,
		      READ_WIDTH => BRAM_DWIDTH,
		      DO_REG => 0,
		      INIT_FILE => "NONE",
		      SIM_COLLISION_CHECK => "ALL") -- Collision check enable "ALL", "WARNING_ONLY", "GENERATE_X_ONLY" or "NONE"       
		    port map (
		      DO => DO,
		      DI => DI,
		      RDADDR => RDADDR,
		      RDCLK => Clk,
		      RDEN => RDEN,     -- 1-bit input read port enable
		      REGCE => '1',
		      RST => Reset, 
		      WE => WSTRB,         -- byte strobe vector
		      WRADDR => WRADDR,
		      WRCLK => Clk,
		      WREN => WREN      -- 1-bit input write port enable
		   );
	    WSTRB <= (others => WREN);
	    
	    RDADDR <= Read_Address;
	    WRADDR <= Write_Address;
	    RDEN   <= read_bram_enable;
	    WREN   <= WE;
	    
	    DI(BRAM_DWIDTH-1 downto 0) <= DataIn(((k+1)*BRAM_DWIDTH-1) downto (k*BRAM_DWIDTH));  
	    DataOut_BRAM(((k+1)*BRAM_DWIDTH-1) downto (k*BRAM_DWIDTH)) <= DO(BRAM_DWIDTH-1 downto 0);  
	end generate gen_mkstrb_loop;
   
end VHDL_RTL;


