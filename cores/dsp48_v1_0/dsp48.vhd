library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library unisim;
use unisim.vcomponents.all;

entity dsp48 is
generic (
  A_WIDTH : natural := 24;
  B_WIDTH : natural := 16;
  P_WIDTH : natural := 24
);
port (
  CLK       : in std_logic;

  A         : in std_logic_vector(A_WIDTH-1 downto 0);
  B         : in std_logic_vector(B_WIDTH-1 downto 0);

  P         : out std_logic_vector(P_WIDTH-1 downto 0)
);
end dsp48;

architecture rtl of dsp48 is

  constant SHIFT : integer := A_WIDTH + B_WIDTH - P_WIDTH - 1;
  constant ONES  : integer := SHIFT - 1;

  signal int_p_wire   : std_logic_vector(48-1 downto 0);
  signal int_pbd_wire : std_logic;
  signal tmp1         : std_logic;

begin

   dsp_0 : DSP48E1
   generic map (
      ALUMODEREG => 0,                   -- Number of pipeline stages for ALUMODE (0 or 1)
      CARRYINSELREG => 0,                -- Number of pipeline stages for CARRYINSEL (0 or 1)
      INMODEREG => 0,                    -- Number of pipeline stages for INMODE (0 or 1)
      OPMODEREG => 0,                    -- Number of pipeline stages for OPMODE (0 or 1)
      CREG => 0,                         -- Number of pipeline stages for C (0 or 1)
      CARRYINREG => 0,                   -- Number of pipeline stages for CARRYIN (0 or 1)
      MREG => 1,                         -- Number of multiplier pipeline stages (0 or 1)
      PREG => 1,                         -- Number of pipeline stages for P (0 or 1)
      USE_PATTERN_DETECT => "PATDET",    -- Enable pattern detect ("PATDET" or "NO_PATDET")
      SEL_MASK => "ROUNDING_MODE1"       -- "C", "MASK", "ROUNDING_MODE1", "ROUNDING_MODE2"
   )
   port map (
      CLK => CLK,                       -- 1-bit input: Clock input
      RSTA => '0',                     -- 1-bit input: Reset input for AREG
      RSTB => '0',                     -- 1-bit input: Reset input for BREG
      RSTM => '0',                     -- 1-bit input: Reset input for MREG
      RSTP => '0',                     -- 1-bit input: Reset input for PREG
      CEA1 => '1',                     -- 1-bit input: Clock enable input for 1st stage AREG
      CEA2 => '1',                     -- 1-bit input: Clock enable input for 2nd stage AREG
      CEB1 => '1',                     -- 1-bit input: Clock enable input for 1st stage BREG
      CEB2 => '1',                     -- 1-bit input: Clock enable input for 2nd stage BREG
      CED  => '0',                       -- 1-bit input: Clock enable input for DREG
      CEAD => '0',                     -- 1-bit input: Clock enable input for ADREG
      CEM  => '1',                       -- 1-bit input: Clock enable input for MREG
      CEP  => '1',                       -- 1-bit input: Clock enable input for PREG
      CEC  => '0',                       -- 1-bit input: Clock enable input for CREG
      RSTINMODE => '0',           -- 1-bit input: Reset input for INMODEREG
      MULTSIGNIN => '0',         -- 1-bit input: Multiplier sign input
      ALUMODE => (others => '0'),
      CEALUMODE => '0',           -- 1-bit input: Clock enable input for ALUMODE
      CEINMODE => '0',             -- 1-bit input: Clock enable input for INMODEREG
      OPMODE => "0110101",
      A    => (29-A_WIDTH downto 0 => A(A_WIDTH-1)) & A,
      B    => (17-B_WIDTH downto 0 => B(B_WIDTH-1)) & B,
      C    => (47-ONES downto 0 => '0') & (ONES-1 downto 0 => '1'),
      D => (others => '0'),                           -- 25-bit input: D data input
      CECTRL => '0',                 -- 1-bit input: Clock enable input for OPMODEREG and CARRYINSELREG
      CARRYIN => '0',
      CARRYCASCIN => '0',
      CARRYINSEL => (others => '0'),
      CECARRYIN => '0',           -- 1-bit input: Clock enable input for CARRYINREG
      INMODE => (others => '0'),                 -- 5-bit input: INMODE control input
      PATTERNBDETECT => int_pbd_wire,
      RSTALLCARRYIN => '0',   -- 1-bit input: Reset input for CARRYINREG
      RSTALUMODE => '0',         -- 1-bit input: Reset input for ALUMODEREG
      PCIN => (others => '0'),                     -- 48-bit input: P cascade input
      P => int_p_wire,
      ACIN => (others => '0'),                     -- 30-bit input: A cascade data input
      RSTC => '0',                     -- 1-bit input: Reset input for CREG
      RSTCTRL => '0',               -- 1-bit input: Reset input for OPMODEREG and CARRYINSELREG
      RSTD => '0',                     -- 1-bit input: Reset input for DREG and ADREG
      BCIN => (others => '0')                      -- 18-bit input: B cascade input

      -- Control: 1-bit (each) output: Control Inputs/Status Bits
--      OVERFLOW => OVERFLOW,             -- 1-bit output: Overflow in add/acc output
--      PATTERNBDETECT => PATTERNBDETECT, -- 1-bit output: Pattern bar detect output
--      PATTERNDETECT => PATTERNDETECT,   -- 1-bit output: Pattern detect output
--      UNDERFLOW => UNDERFLOW,           -- 1-bit output: Underflow in add/acc output
--      -- Data: 4-bit (each) output: Data Ports
--      CARRYOUT => CARRYOUT,             -- 4-bit output: Carry output
--      P => P,                           -- 48-bit output: Primary data output
--      -- Cascade: 30-bit (each) input: Cascade Ports
--      ACIN => ACIN,                     -- 30-bit input: A cascade data input
--      BCIN => BCIN,                     -- 18-bit input: B cascade input
--      CARRYCASCIN => CARRYCASCIN,       -- 1-bit input: Cascade carry input
--      -- Control: 4-bit (each) input: Control Inputs/Status Bits
--      ALUMODE => ALUMODE,               -- 4-bit input: ALU control input
--      CARRYINSEL => CARRYINSEL,         -- 3-bit input: Carry select input
--      CLK => CLK,                       -- 1-bit input: Clock input
--      OPMODE => OPMODE,                 -- 7-bit input: Operation mode input
--      -- Data: 30-bit (each) input: Data Ports
--      A => A,                           -- 30-bit input: A data input
--      B => B,                           -- 18-bit input: B data input
--      C => C,                           -- 48-bit input: C data input
--      CARRYIN => CARRYIN,               -- 1-bit input: Carry input signal
--      -- Reset/Clock Enable: 1-bit (each) input: Reset/Clock Enable Inputs
--      CEA2 => CEA2,                     -- 1-bit input: Clock enable input for 2nd stage AREG
--      CEAD => CEAD,                     -- 1-bit input: Clock enable input for ADREG
--      CEB2 => CEB2,                     -- 1-bit input: Clock enable input for 2nd stage BREG
--      CED => CED,                       -- 1-bit input: Clock enable input for DREG
--      CEM => CEM,                       -- 1-bit input: Clock enable input for MREG
--      CEP => CEP,                       -- 1-bit input: Clock enable input for PREG
   );

  tmp1 <= int_p_wire(SHIFT) or int_pbd_wire;
  P    <= int_p_wire(SHIFT+P_WIDTH-1 downto SHIFT+1) & tmp1;

end rtl;

