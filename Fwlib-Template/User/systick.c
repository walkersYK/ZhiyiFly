#include "systick.h"
void SystickConfig(u32 us){
	SysTick_Config(us*72);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}	
