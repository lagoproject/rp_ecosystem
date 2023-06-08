#include "zynq_io.h"

int main(int argc, char *argv[])
{
	//int fd;
	//unsigned int size;
	uint32_t i,val=0;
	uint32_t wo;
	int16_t ch[2];

	printf("ADC direct project\n");

	//initialize devices. TODO: add error checking 
	cfg_init();    
	sts_init();    
	cma_init();

	printf("Reseting FIFO...\n");
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val &= ~1,0);
	printf("Reseting tlast_gen core...\n");
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val &=~2,0);
	printf("Reseting writer...\n");
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val &= ~4,0);

	printf("Set writer address...\n");
	val=rd_reg_value(1, CFG_WR_ADDR_OFFSET,0);
	printf("val es: %d ...\n",val);
	printf("dev_size es: %d ...\n",dev_size);
	wr_reg_value(1, CFG_WR_ADDR_OFFSET, dev_size,0);

	printf("Set number of samples...\n");
	val=rd_reg_value(1, CFG_NSAMPLES_OFFSET,0);
	wr_reg_value(1, CFG_NSAMPLES_OFFSET, 1024 * 1024 - 1,0);

	// enter normal mode
	// tlast_gen
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val |= 2,0);
  //writer
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val |= 4,0);
	// wait 1 second
	sleep(1);
	//counters
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val |= 1,0);

	// wait 1 second
	//sleep(1);

	// print IN1 and IN2 samples
	for(i = 0; i < 2 * 1024 * 1024; ++i){
		ch[0] = cma_ptr[2*i + 0];
		ch[1] = cma_ptr[2*i + 1]; 
		wo = *((uint32_t *)(cma_ptr + 2*i));
		printf("%5d %5d %10d\n", ch[0], ch[1], wo);
	}

	//reset FIFO again
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val &= ~1,0);
	// unmap and close the devices 
	munmap(cfg_ptr, sysconf(_SC_PAGESIZE));
	munmap(sts_ptr, sysconf(_SC_PAGESIZE));

	close(cfg_fd);
	close(sts_fd);

	return 0;

}
