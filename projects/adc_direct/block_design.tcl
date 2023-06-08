source projects/base_system/block_design.tcl

# Create xlconstant
cell xilinx.com:ip:xlconstant const_0

# Create proc_sys_reset
cell xilinx.com:ip:proc_sys_reset rst_0 {} {
  ext_reset_in const_0/dout
}

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

# Create axi_sts_register
cell labdpr:user:axi_sts_register sts_0 {
  STS_DATA_WIDTH 32
  AXI_ADDR_WIDTH 32
  AXI_DATA_WIDTH 32
} {}

module ram_writer_0 {
  source projects/adc_direct/ram_wr.tcl
} {
        slice_1/din cfg_0/cfg_data
        slice_2/din cfg_0/cfg_data
        slice_3/din cfg_0/cfg_data
        slice_4/din cfg_0/cfg_data
        writer_0/sts_data sts_0/sts_data
        tlast_gen_0/S_AXIS adc_0/M_AXIS
}

addr 0x40001000 4K cfg_0/S_AXI /ps_0/M_AXI_GP0
addr 0x40002000 4K sts_0/S_AXI /ps_0/M_AXI_GP0

assign_bd_address [get_bd_addr_segs ps_0/S_AXI_ACP/ACP_DDR_LOWOCM]
