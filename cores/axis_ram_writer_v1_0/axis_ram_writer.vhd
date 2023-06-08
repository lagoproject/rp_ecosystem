library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library xpm;
use xpm.vcomponents.all;
library unisim;
use unisim.vcomponents.all;

entity axis_ram_writer is
  generic (
            ADDR_WIDTH       : natural := 20;
            AXI_ID_WIDTH     : natural := 6;
            AXI_ADDR_WIDTH   : natural := 32;
            AXI_DATA_WIDTH   : natural := 64;
            AXIS_TDATA_WIDTH : natural := 64;
            FIFO_WRITE_DEPTH : natural := 512
          );
  port (
         -- System signals
         aclk             : in std_logic;
         aresetn          : in std_logic;

         -- Configuration bits
         cfg_data         : in std_logic_vector(AXI_ADDR_WIDTH-1 downto 0);
         sts_data         : out std_logic_vector(ADDR_WIDTH-1 downto 0);

         -- Master side
         m_axi_awid       : out std_logic_vector(AXI_ID_WIDTH-1 downto 0);     -- AXI master: Write address ID
         m_axi_awaddr     : out std_logic_vector(AXI_ADDR_WIDTH-1 downto 0);   -- AXI master: Write address
         m_axi_awlen      : out std_logic_vector(3 downto 0);                  -- AXI master: Write burst length
         m_axi_awsize     : out std_logic_vector(2 downto 0);                  -- AXI master: Write burst size
         m_axi_awburst    : out std_logic_vector(1 downto 0);                  -- AXI master: Write burst type
         m_axi_awcache    : out std_logic_vector(3 downto 0);                  -- AXI master: Write memory type
         m_axi_awvalid    : out std_logic;                                     -- AXI master: Write address valid
         m_axi_awready    : in std_logic;                                      -- AXI master: Write address ready
         m_axi_wid        : out std_logic_vector(AXI_ID_WIDTH-1 downto 0);     -- AXI master: Write data ID
         m_axi_wdata      : out std_logic_vector(AXI_DATA_WIDTH-1 downto 0);   -- AXI master: Write data
         m_axi_wstrb      : out std_logic_vector(AXI_DATA_WIDTH/8-1 downto 0); -- AXI master: Write strobes
         m_axi_wlast      : out std_logic;                                     -- AXI master: Write last
         m_axi_wvalid     : out std_logic;                                     -- AXI master: Write valid
         m_axi_wready     : in std_logic;                                      -- AXI master: Write ready
         m_axi_bvalid     : in std_logic;                                      -- AXI master: Write response valid
         m_axi_bready     : out std_logic;                                     -- AXI master: Write response ready

         -- Slave side
         s_axis_tready    : out std_logic;
         s_axis_tdata     : in std_logic_vector(AXIS_TDATA_WIDTH-1 downto 0);
         s_axis_tvalid    : in std_logic
       );
end axis_ram_writer;

architecture rtl of axis_ram_writer is

  function clogb2 (value: natural) return natural is
  variable temp    : natural := value;
  variable ret_val : natural := 1;
  begin
    while temp > 1 loop
      ret_val := ret_val + 1;
      temp    := temp / 2;
    end loop;
    return ret_val;
  end function;

  constant ADDR_SIZE : natural := clogb2((AXI_DATA_WIDTH/8) - 1);
  constant COUNT_SIZE: natural := clogb2((FIFO_WRITE_DEPTH*AXIS_TDATA_WIDTH/AXI_DATA_WIDTH) - 1) + 1;

  signal int_awvalid_reg, int_awvalid_next : std_logic;
  signal int_wvalid_reg, int_wvalid_next   : std_logic;
  signal int_addr_reg, int_addr_next       : unsigned(ADDR_WIDTH-1 downto 0);
  signal int_awid_reg, int_awid_next       : unsigned(AXI_ID_WIDTH-1 downto 0);

  signal int_full_wire, int_empty_wire, int_rden_wire : std_logic;
  signal int_wlast_wire, int_tready_wire   : std_logic;
  signal int_count_wire                    : std_logic_vector(COUNT_SIZE-1 downto 0);
  signal int_wdata_wire                    : std_logic_vector(AXI_DATA_WIDTH-1 downto 0);

  signal tmp_s2                            : std_logic;
  signal reset                             : std_logic;

begin

  int_tready_wire <= not int_full_wire;
  int_wlast_wire <= '1' when (int_addr_reg(3 downto 0) = "1111") else '0';
  int_rden_wire <= m_axi_wready and int_wvalid_reg;
  tmp_s2 <= int_tready_wire and s_axis_tvalid;

  reset <= not aresetn;

  fifo_0 : xpm_fifo_sync
   generic map (
      WRITE_DATA_WIDTH => AXIS_TDATA_WIDTH,   -- DECIMAL
      FIFO_WRITE_DEPTH => FIFO_WRITE_DEPTH,   -- DECIMAL
      READ_DATA_WIDTH  => AXI_DATA_WIDTH,     -- DECIMAL
      READ_MODE        => "fwft",             -- String
      FIFO_READ_LATENCY => 0,                 -- DECIMAL
      USE_ADV_FEATURES => "0400",             -- String
      RD_DATA_COUNT_WIDTH => COUNT_SIZE       -- DECIMAL
   )
   port map (
      full          => int_full_wire,
      rd_data_count => int_count_wire, 
      rst           => reset,
      wr_clk        => aclk,
      wr_en         => tmp_s2,
      din           => s_axis_tdata,
      rd_en         => int_rden_wire,
      dout          => int_wdata_wire,
      sleep         => '0',
      injectdbiterr => '0',
      injectsbiterr => '0' 
   );

  process(aclk)
  begin
    if (rising_edge(aclk)) then
      if (reset = '1') then
        int_awvalid_reg <= '0';
        int_wvalid_reg <= '0';
        int_addr_reg <= (others => '0');
        int_awid_reg <= (others => '0');
      else
        int_awvalid_reg <= int_awvalid_next;
        int_wvalid_reg <= int_wvalid_next;
        int_addr_reg <= int_addr_next;
        int_awid_reg <= int_awid_next;
      end if;
    end if;
  end process;

  int_awvalid_next <= '1' when ((unsigned(int_count_wire) >  15) and (int_awvalid_reg = '0') and (int_wvalid_reg = '0')) or 
                      ((m_axi_wready = '1') and (int_wlast_wire = '1') and (unsigned(int_count_wire) > 16)) else 
                      '0' when ((m_axi_awready = '1') and (int_awvalid_reg = '1')) else
                      int_awvalid_reg;

  int_wvalid_next <= '1' when ((unsigned(int_count_wire) > 15) and (int_awvalid_reg = '0') and (int_wvalid_reg = '0')) else 
                     '0' when (m_axi_wready = '1') and (int_wlast_wire = '1') and (unsigned(int_count_wire) <= 16) else
                     int_wvalid_reg;

  int_addr_next <= int_addr_reg + 1 when (int_rden_wire = '1') else
                   int_addr_reg;

  int_awid_next <= int_awid_reg + 1 when (m_axi_wready = '1') and (int_wlast_wire = '1') else
                   int_awid_reg;

  sts_data      <= std_logic_vector(int_addr_reg);

  m_axi_awid    <= std_logic_vector(int_awid_reg);
  m_axi_awaddr  <= std_logic_vector(unsigned(cfg_data) + (int_addr_reg & (ADDR_SIZE-1 downto 0 => '0')));
  m_axi_awlen   <= std_logic_vector(to_unsigned(15, m_axi_awlen'length));
  m_axi_awsize  <= std_logic_vector(to_unsigned(ADDR_SIZE, m_axi_awsize'length));
  m_axi_awburst <= "01";
  m_axi_awcache <= "0110";
  m_axi_awvalid <= int_awvalid_reg;
  m_axi_wid     <= std_logic_vector(int_awid_reg);
  m_axi_wdata   <= int_wdata_wire;
  m_axi_wstrb   <= ((AXI_DATA_WIDTH/8-1) downto 0 => '1');
  m_axi_wlast   <= int_wlast_wire;
  m_axi_wvalid  <= int_wvalid_reg;
  m_axi_bready  <= '1';

  s_axis_tready <= int_tready_wire;

end rtl;
