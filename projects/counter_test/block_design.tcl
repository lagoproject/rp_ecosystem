source projects/base_system/block_design.tcl

# Create xlconstant
cell xilinx.com:ip:xlconstant const_0

# Create proc_sys_reset
cell xilinx.com:ip:proc_sys_reset rst_0 {} {
  ext_reset_in const_0/dout
}


# ADC

# Create axis_red_pitaya_adc
cell labdpr:user:axis_rp_adc adc_0 {
  ADC_DATA_WIDTH 14
} {
  aclk pll_0/clk_out1
  adc_dat_a adc_dat_a_i
  adc_dat_b adc_dat_b_i
  adc_csn adc_csn_o
}

# Create axi_cfg_register
cell labdpr:user:axi_cfg_register cfg_0 {
  CFG_DATA_WIDTH 128
  AXI_ADDR_WIDTH 32
  AXI_DATA_WIDTH 32
}

# Create port_slicer
cell labdpr:user:port_slicer slice_0 {
  DIN_WIDTH 128 DIN_FROM 0 DIN_TO 0
} {
  din cfg_0/cfg_data
}

# Create port_slicer
cell labdpr:user:port_slicer slice_1 {
  DIN_WIDTH 128 DIN_FROM 1 DIN_TO 1
} {
  din cfg_0/cfg_data
}

# Create port_slicer
cell labdpr:user:port_slicer slice_2 {
  DIN_WIDTH 128 DIN_FROM 2 DIN_TO 2
} {
  din cfg_0/cfg_data
}

# Create port_slicer
cell labdpr:user:port_slicer slice_3 {
  DIN_WIDTH 128 DIN_FROM 63 DIN_TO 32
} {
  din cfg_0/cfg_data
}

# Create port_slicer
cell labdpr:user:port_slicer slice_4 {
  DIN_WIDTH 128 DIN_FROM 95 DIN_TO 64
} {
  din cfg_0/cfg_data
}

# Create axis_clock_converter
cell xilinx.com:ip:axis_clock_converter fifo_0 {} {
  S_AXIS adc_0/M_AXIS
  s_axis_aclk pll_0/clk_out1
  s_axis_aresetn const_0/dout
  m_axis_aclk ps_0/FCLK_CLK0
  m_axis_aresetn slice_0/Dout
}

# Create counter
cell labdpr:user:axis_counter axis_counter_0 {
  AXIS_TDATA_WIDTH 16
  CNTR_WIDTH 16
} {
  aclk pll_0/clk_out1
  aresetn slice_0/dout
}

# Create xlconstant
cell xilinx.com:ip:xlconstant const_2 {
 CONST_VAL 65535
 CONST_WIDTH 16
} {
 dout axis_counter_0/cfg_data
}

# Create counter
cell labdpr:user:axis_counter axis_counter_1 {
  AXIS_TDATA_WIDTH 16
  CNTR_WIDTH 16
} {
  aclk pll_0/clk_out1
  aresetn slice_0/dout
}

# Create xlconstant
cell xilinx.com:ip:xlconstant const_3 {
 CONST_VAL 65535
 CONST_WIDTH 16
} {
 dout axis_counter_1/cfg_data
}

#Create combiner
cell xilinx.com:ip:axis_combiner comb_0 {} {
  aclk pll_0/clk_out1
 aresetn slice_1/Dout
 S00_AXIS axis_counter_0/M_AXIS
 S01_AXIS axis_counter_1/M_AXIS
}

# Create the tlast generator
cell labdpr:user:axis_tlast_gen tlast_gen_0 {
  AXIS_TDATA_WIDTH 32
  PKT_CNTR_BITS 32
} {
  S_AXIS comb_0/M_AXIS
  pkt_length slice_4/dout
  aclk pll_0/clk_out1
  aresetn slice_1/dout
}

# Create axis_dwidth_converter
cell xilinx.com:ip:axis_dwidth_converter conv_0 {
  S_TDATA_NUM_BYTES.VALUE_SRC USER
  S_TDATA_NUM_BYTES 4
  M_TDATA_NUM_BYTES 8
} {
  S_AXIS tlast_gen_0/M_AXIS
  aclk pll_0/clk_out1
  aresetn slice_2/dout
}

# Create axis_ram_writer
cell labdpr:user:axis_ram_writer writer_0 {
  ADDR_WIDTH 16
  AXI_ID_WIDTH 3
} {
  aclk pll_0/clk_out1
  aresetn slice_2/dout
  S_AXIS conv_0/M_AXIS
  M_AXI ps_0/S_AXI_ACP
  cfg_data slice_3/dout
}

# Create axi_sts_register
cell labdpr:user:axi_sts_register sts_0 {
  STS_DATA_WIDTH 32
  AXI_ADDR_WIDTH 32
  AXI_DATA_WIDTH 32
} {
  sts_data writer_0/sts_data
}

addr 0x40001000 4K cfg_0/S_AXI /ps_0/M_AXI_GP0
addr 0x40002000 4K sts_0/S_AXI /ps_0/M_AXI_GP0

