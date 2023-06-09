
# Create port_slicer for reset modules. pktzr rst. off=0
cell labdpr:user:port_slicer slice_1 {
  DIN_WIDTH 128 DIN_FROM 1 DIN_TO 1
} {}

# Create port_slicer for rst converter and ram writer. off=0
cell labdpr:user:port_slicer slice_2 {
  DIN_WIDTH 128 DIN_FROM 2 DIN_TO 2 
} {}

# Create port_slicer for ram writer offset. off=4
cell labdpr:user:port_slicer slice_3 {
  DIN_WIDTH 128 DIN_FROM 63 DIN_TO 32
} {}

# Create port_slicer for pkt_length size. off=8
cell labdpr:user:port_slicer slice_4 {
  DIN_WIDTH 128 DIN_FROM 95 DIN_TO 64
} {}

# Create axis_packetizer
cell labdpr:user:axis_packetizer pktzr_0 {
  AXIS_TDATA_WIDTH 32
  CNTR_WIDTH 32
  CONTINUOUS TRUE
} {
  aclk /pll_0/clk_out1
  aresetn slice_1/dout
  cfg_data slice_4/dout
}

## Create the tlast generator
#cell labdpr:user:axis_tlast_gen tlast_gen_0 {
#  AXIS_TDATA_WIDTH 32
#  PKT_CNTR_BITS 32
#} {
#  aclk /pll_0/clk_out1
#  aresetn slice_1/dout
#  pkt_length slice_4/dout
#}

# Create axis_dwidth_converter
#cell xilinx.com:ip:axis_dwidth_converter conv_0 {
#  S_TDATA_NUM_BYTES.VALUE_SRC USER
#  S_TDATA_NUM_BYTES 4
#  M_TDATA_NUM_BYTES 8
#} {
#  aclk /pll_0/clk_out1
#  aresetn slice_2/dout
#  S_AXIS pktzr_0/M_AXIS
#}

# Create axis_ram_writer
cell labdpr:user:axis_ram_writer writer_0 {
  ADDR_WIDTH 20
  AXI_ID_WIDTH 3
  AXIS_TDATA_WIDTH 32
  FIFO_WRITE_DEPTH 1024
} {
  aclk /pll_0/clk_out1
  aresetn slice_2/dout
  S_AXIS pktzr_0/M_AXIS
  M_AXI /ps_0/S_AXI_ACP
  cfg_data slice_3/dout
}

