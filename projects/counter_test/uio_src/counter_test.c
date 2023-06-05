#include "zynq_io.h"

int main(int argc, char *argv[])
{
	//int fd;
	//unsigned int size;
	uint32_t i,val=0;
	uint32_t wo,wo2;
	int16_t ch[4];

	printf("ADC RECORDER test\n");

	//initialize devices. TODO: add error checking 
	cfg_init();    
	sts_init();    
	cma_init();

	printf("Reseting counters...\n");
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
	wr_reg_value(1, CFG_NSAMPLES_OFFSET, 65536,0);

	// wait 1 second
	//sleep(1);

	// enter normal mode
	// tlast_gen
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val |= 2,0);
  //writer
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val |= 4,0);
	// wait 1 second
	sleep(2);
	//counters
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val |= 1,0);

	//val=rd_reg_value(1, STS_STATUS_OFFSET,1);
	//printf("%5d\n", val);
	// wait 1 second
	//sleep(1);

	// print IN1 and IN2 samples
	for(i = 0; i < 2*65536; ++i){
	//for(i = 0; i < 65536; i+=2){
		//ch[0] = *(cma_ptr + 2*i + 0);
		//ch[1] = *(cma_ptr + 2*i + 1);
		//ch[0] = cma_ptr[2*i + 0];
		//ch[1] = cma_ptr[2*i + 1]; 
		//ch[2] = *((uint16_t *)(cma_ptr2 + i));
		//ch[3] = *((uint16_t *)(cma_ptr2 + 2*i));
		//wo = *((uint32_t *)(cma_ptr + 2*i));
		//wo2 = *((uint32_t *)(cma_ptr2 + i));
		//printf("%5d %5d %10d\n", ch[0], ch[1], wo);
		//printf("%5d %5d %10d\n", ch[2], ch[3], wo2);
		//printf("%5d %5d %10d %5d %5d %10d\n", ch[0], ch[1], wo, (int16_t)(wo&0xFFFF), (int16_t)(wo>>16), wo2);
	//	printf("%5d %5d %10d %5d %5d %10d\n", ch[0], ch[1], wo, ch[2], ch[3], wo2);
		//printf("%5d %5d %10d\n", ch[2], ch[3], wo2);
	//val=rd_reg_value(1, STS_STATUS_OFFSET,1);
	//printf("#%5d\n", val);
		ch[0] = cma_ptr[2 * i + 0];
		ch[1] = cma_ptr[2 * i + 1];
		//wo    = cma_ptr[i] 
		printf("%5d %5d\n", ch[0], ch[1]);
	}
	//val=rd_reg_value(1, STS_STATUS_OFFSET,1);
	//printf("%5d\n", val);

	//reset counters again
	val=rd_reg_value(1, CFG_RESET_GRAL_OFFSET,0);
	wr_reg_value(1, CFG_RESET_GRAL_OFFSET, val &= ~1,0);
	// unmap and close the devices 
	munmap(cfg_ptr, sysconf(_SC_PAGESIZE));
	munmap(sts_ptr, sysconf(_SC_PAGESIZE));

	close(cfg_fd);
	close(sts_fd);
	//printf("Saliendo ...\n");

	return 0;

}
