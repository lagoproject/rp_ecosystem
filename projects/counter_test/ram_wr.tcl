# Create xlslice for reset modules. off=0
cell labdpr:user:port_slicer rst_1 {
  DIN_WIDTH 160 DIN_FROM 1 DIN_TO 1
} {}

# Create xlslice for reset tlast_gen. off=0
cell labdpr:user:port_slicer rst_2 {
  DIN_WIDTH 160 DIN_FROM 2 DIN_TO 2 
} {}

# Create xlslice for set the # of samples to get. off=1
cell labdpr:user:port_slicer nsamples {
  DIN_WIDTH 160 DIN_FROM 63 DIN_TO 32 
} {}

# Create xlconstant
cell xilinx.com:ip:xlconstant wr_offset {
  CONST_WIDTH 32
  CONST_VAL 503316480
}

# Create the tlast generator
cell labdpr:user:axis_tlast_gen tlast_gen_0 {
  AXIS_TDATA_WIDTH 32
  PKT_CNTR_BITS 32
} {
  aclk /pll_0/clk_out1
  aresetn rst_1/Dout
  pkt_length nsamples/Dout
}

# Create axis_dwidth_converter
cell xilinx.com:ip:axis_dwidth_converter conv_0 {
  S_TDATA_NUM_BYTES.VALUE_SRC USER
  S_TDATA_NUM_BYTES 4
  M_TDATA_NUM_BYTES 8
} {
  aclk /pll_0/clk_out1
  aresetn rst_2/Dout
  S_AXIS tlast_gen_0/M_AXIS
}

# Create axis_ram_writer
cell labdpr:user:axis_ram_writer writer_0 {
  ADDR_WIDTH 20
} {
  aclk /pll_0/clk_out1
  aresetn rst_2/Dout
  S_AXIS conv_0/M_AXIS
  M_AXI /ps_0/S_AXI_HP0
  cfg_data wr_offset/dout
}

