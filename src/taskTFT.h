#ifndef _TASKTFT_H_
#define _TASKTFT_H_
/* system includes */

#include "FreeRTOS.h"
#include "task.h"
/* local includes */


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

namespace TFT {
	void init();

	// this will wakeup a running tft task causing update
	void update();

	TaskHandle_t getTask();
}

#endif

#endif /* _TASKTFT_H_ */

