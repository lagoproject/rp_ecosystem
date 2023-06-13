source projects/base_system/block_design.tcl

# Create xlconstant
cell xilinx.com:ip:xlconstant const_0

# Create proc_sys_reset
cell xilinx.com:ip:proc_sys_reset rst_0 {} {
  ext_reset_in const_0/dout
}

# Create axi_cfg_register
cell labdpr:user:axi_cfg_register cfg_0 {
  CFG_DATA_WIDTH 160
  AXI_ADDR_WIDTH 32
  AXI_DATA_WIDTH 32
}

# Create axi_sts_register
cell labdpr:user:axi_sts_register sts_0 {
  STS_DATA_WIDTH 32
  AXI_ADDR_WIDTH 32
  AXI_DATA_WIDTH 32
} {
}

# Create axis_rp_adc
cell labdpr:user:axis_rp_adc adc_0 {} {
  aclk pll_0/clk_out1
  adc_dat_a adc_dat_a_i
  adc_dat_b adc_dat_b_i
  adc_csn adc_csn_o
}

# Create counter
cell labdpr:user:axis_counter axis_counter_0 {
  AXIS_TDATA_WIDTH 16
  CNTR_WIDTH 16
} {
  aclk pll_0/clk_out1
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
 S00_AXIS axis_counter_0/M_AXIS
 S01_AXIS axis_counter_1/M_AXIS
}

module ram_writer_0 {
  source projects/counter_test/ram_wr.tcl
} {
				rst_1/din cfg_0/cfg_data
				rst_2/din cfg_0/cfg_data
				nsamples/din cfg_0/cfg_data
				writer_0/sts_data sts_0/sts_data
				tlast_gen_0/S_AXIS comb_0/M_AXIS
				rst_1/dout axis_counter_0/aresetn
        rst_1/dout axis_counter_1/aresetn
        rst_1/dout comb_0/aresetn
}

addr 0x40001000 4K cfg_0/S_AXI /ps_0/M_AXI_GP0
addr 0x40002000 4K sts_0/S_AXI /ps_0/M_AXI_GP0

assign_bd_address [get_bd_addr_segs ps_0/S_AXI_HP0/HP0_DDR_LOWOCM]
