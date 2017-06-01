/**
* @brief 
* @file dsp_out.cpp
* @author J.H. 
* @date 2017-05-18
*/

/* module header */
#include "dsp_out.h"
#include "math.h"

/* system includes C */
#include "mk20dx128.h"
#include "FreeRTOS.h"
#include "task.h"

/* system includes C++ */


/* local includes */
#include "pdb.h"
#include "hw.h"
#include "audio/utils.h"
#include "instruments/kick.h"

#include "rack.h"

namespace DSP {

#define BUFFER_SIZE 512
#define DMAMEM __attribute__ ((section(".dmabuffers"), used))
static volatile uint16_t __attribute__((aligned(16))) dac_buffer[BUFFER_SIZE];
//static DMAMEM volatile uint16_t __attribute__((aligned(16)))  dac_buffer[BUFFER_SIZE];
static KickDrum _osc;
static TaskHandle_t _bufFillTask = NULL;

volatile uint16_t *begin = NULL;
volatile uint16_t *end = NULL;

static void fillBufTask(void * params)
{

	while(1) {
		uint32_t ulNotificationValue = ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		if( ulNotificationValue == 1 )   {
			do {
				*begin++ = (Rack::update() + 0x7fff) >> 4;
			} while(begin < end);
		}  else  {
		}	
	}
}

// the interrupt is triggered either when the read pointer reaches half of the buffer, or at the end
//  in either cases, we will update the contents...
static void dma0_isr(void)
{
	uint32_t saddr = (uint32_t)DMA_TCD0_SADDR;
	DMA_CINT = 0; //clear interrupt for channel 0
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (saddr < ((uint32_t)dac_buffer + (sizeof(dac_buffer) / 2))) {
		begin = &dac_buffer[BUFFER_SIZE/2];
		end = &dac_buffer[BUFFER_SIZE];
	} else {
		begin = &dac_buffer[0];
		end = &dac_buffer[BUFFER_SIZE/2];
	}

	vTaskNotifyGiveFromISR( _bufFillTask, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void init()
{
	// DMA + PDB + DAC is somewhat tricky
	// this code is adapted from https://hackaday.io/project/12543-solid-state-flow-sensing-using-eis/log/41575-dac-with-dma-and-buffer-on-a-teensy-32
	// enable DAC
	SIM_SCGC2 |= SIM_SCGC2_DAC0;
	DAC0_C0 = DAC_C0_DACRFS | DAC_C0_DACEN;                  
	DAC0_C1 = 0;
	//
	// slowly ramp up to DC voltage
	for (int16_t i=0; i<0xfff; i++) {
		*(int16_t *)&(DAC0_DAT0L) = i;
	}
	for (int16_t i=0; i<16; i++) {
		*(int16_t *)(&DAC0_DAT0L + 2*i) = 0xfff;
	}
	for(int i = 0; i < BUFFER_SIZE; i++) {
		dac_buffer[i] = (0xfff / BUFFER_SIZE) * i;
	}
	

	// first, we need to init the dma and dma mux
	// to do this, we enable the clock to DMA and DMA MUX using the system timing registers
	SIM_SCGC6 |= SIM_SCGC6_DMAMUX; // enable DMA MUX clock
	SIM_SCGC7 |= SIM_SCGC7_DMA;    // enable DMA clock
	// then, we enable requests on our channel
	DMA_ERQ = DMA_ERQ_ERQ0; // Enable requests on DMA channel 0
	// next up, the channel in the DMA MUX needs to be configured
	DMAMUX0_CHCFG0 = DMAMUX_SOURCE_DAC0; //Select DAC as request source
	DMAMUX0_CHCFG0 |= DMAMUX_ENABLE;      //Enable DMA channel 0
	// then, we enable requests on our channel
	DMA_ERQ = DMA_ERQ_ERQ0; // Enable requests on DMA channel 0
	// Here we choose where our data is coming from, and where it is going
	DMA_TCD0_SADDR = dac_buffer;   // set the address of the first byte in our LUT as the source address
	DMA_TCD0_DADDR = (uint16_t*)(&DAC0_DAT0L); // set the first data register as the destination address
	// now we need to set the read and write offsets - kind of boring
	DMA_TCD0_SOFF = 16; // advance 32 bits, or 4 bytes per read
	DMA_TCD0_DOFF = 16; // advance 32 bits, or 4 bytes per write
	// this is the fun part! Now we get to set the data transfer size...
	DMA_TCD0_ATTR  = DMA_TCD_ATTR_SSIZE(DMA_TCD_ATTR_SIZE_16BYTE);
	DMA_TCD0_ATTR |= DMA_TCD_ATTR_DSIZE(DMA_TCD_ATTR_SIZE_16BYTE) | DMA_TCD_ATTR_DMOD(5); // set the data transfer size to 32 bit for both the source and the destination
//    DMA_TCD0_ATTR |= DMA_TCD_ATTR_DSIZE(DMA_TCD_ATTR_SIZE_8BIT) | DMA_TCD_ATTR_DMOD(31 - __builtin_clz(32)); // set the data transfer size to 32 bit for both the source and the destination
	// ...and the number of bytes to be transferred per request (or 'minor loop')...
	DMA_TCD0_NBYTES_MLNO = 16; // we want to fill half of the DAC buffer, which is 16 words in total, so we need 8 words - or 16 bytes - per transfer
	// set the number of minor loops (requests) in a major loop
	// the circularity of the buffer is handled by the modulus functionality in the TCD attributes
	DMA_TCD0_CITER_ELINKNO = DMA_TCD_CITER_ELINKYES_CITER(sizeof(dac_buffer) / 16);
	DMA_TCD0_BITER_ELINKNO = DMA_TCD_BITER_ELINKYES_BITER(sizeof(dac_buffer) / 16);
	// the address is adjusted by these values when a major loop completes
	// we don't need this for the destination, because the circularity of the buffer is already handled
	DMA_TCD0_SLAST    = -sizeof(dac_buffer);
	DMA_TCD0_DLASTSGA = 0;
	// do the final init of the channel
	DMA_TCD0_CSR = DMA_TCD_CSR_INTHALF |  DMA_TCD_CSR_INTMAJOR;
//    DMA_TCD0_CSR = 0;

	// enable interrupt for DMA (refilling the buffer)
	NVIC_SET_PRIORITY(IRQ_DMA_CH0, 80);
	NVIC_ENABLE_IRQ(IRQ_DMA_CH0);
	_VectorsRam[IRQ_DMA_CH0 + 16] = dma0_isr; // set the timer interrupt

	// enable DAC DMA
#define DAC0_SR_DACBFWMF 0x04 // buffer watermark flag
#define DAC0_SR_DACBFRTF 0x02 // buffer top flag
#define DAC0_SR_DACBFRBF 0x01 // buffer bottom flag (not used)
//    DAC0_C0 |= DAC_C0_DACTRGSEL;
	DAC0_C1 |=  DAC_C1_DACBFEN | DAC_C1_DACBFWM(3); // enable dma and buffer
	DAC0_C0 |= DAC_C0_DACBTIEN | DAC_C0_DACBWIEN  ; // enable read pointer bottom and waterwark interrupt
	DAC0_C2 = DAC_C2_DACBFRP(0) | DAC_C2_DACBFUP(15)  ;									// buffer pointer location
	DAC0_C1 |= DAC_C1_DMAEN;
	DAC0_C2 = DAC_C2_DACBFRP(11) | DAC_C2_DACBFUP(15)  ;									// buffer pointer location


	// PDB will periodically trigger DAC to read next sample from the buffer 
	//  once the watermark is reached, DMA is triggered and lower half of the buffer will be refilled by DMA channel
	// init the PDB for DAC interval generationV

	SIM_SCGC6 |= SIM_SCGC6_PDB; // turn on the PDB clock  
	PDB0_SC |= PDB_SC_PDBEN; // enable the PDB  
	PDB0_SC |= PDB_SC_TRGSEL(15); // trigger the PDB on software start (SWTRIG)  
	PDB0_SC |= PDB_SC_CONT; // run in continuous mode  
	PDB0_MOD = PDB_PERIOD; // modulus time for the PDB  
	PDB0_DACINT0 = (uint16_t)PDB_PERIOD; // we won't subdivide the clock... 
	PDB0_DACINTC0 |= 0x1; // enable the DAC interval trigger  
	PDB0_SC |= PDB_SC_LDOK; // update pdb registers  
	PDB0_SC |= PDB_SC_SWTRIG; // ...and start the PDB
	
	xTaskCreate( fillBufTask, "buf", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &_bufFillTask ); 
}


}
