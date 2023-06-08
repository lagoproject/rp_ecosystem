set proj_dir [file normalize "[pwd]"]

# Create xlslice for reset modules. off=0
cell labdpr:user:port_slicer rst_4 {
  DIN_WIDTH 160 DIN_FROM 4 DIN_TO 4
} {}

# Create data memory
cell labdpr:user:gen_tonos gen_tonos_0 {
} {
  aclk /pll_0/clk_out1
	aresetn rst_4/dout
}

# Create distributed memory generator
# coefficient_file [file join $proj_dir projects dft_rxc16 tx_source_real_cosine_16.coe]
cell xilinx.com:ip:dist_mem_gen coef_mem_r {
  depth 16384
  data_width 16
  memory_type rom
  coefficient_file [file join $proj_dir projects dft_rxc16 ant_source_real_cosine_16.coe]
} {
  a gen_tonos_0/a_r
  spo gen_tonos_0/spo_r
}

# Create distributed memory generator
# coefficient_file [file join $proj_dir projects dft_rxc16 tx_source_imag_cosine_16.coe]
cell xilinx.com:ip:dist_mem_gen coef_mem_i {
  depth 16384
  data_width 16
  memory_type rom
  coefficient_file [file join $proj_dir projects dft_rxc16 ant_source_imag_cosine_16.coe]
} {
  a gen_tonos_0/a_i
  spo gen_tonos_0/spo_i
}

