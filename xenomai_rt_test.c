#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <native/task.h>
#include <native/timer.h>
//#include <bcm2835.h>
#include <mcp23s17.h>

RT_TASK demo_task;
/* NOTE: error handling omitted. */

void demo(void *arg)
{
	
	
	RTIME now, previous;
	/*
	* Arguments: &task (NULL=self),
	* start time,
	* period (here: 1 s)
	*/
	rt_task_set_periodic(NULL, TM_NOW, 1000000);
	previous = rt_timer_read();
	
	uint16_t registro;
		
	registro =0xFFFF;
	while (1) {
		rt_task_wait_period(NULL);
		now = rt_timer_read();
		/*
		* NOTE: printf may have unexpected impact on the timing of
		* your program. It is used here in the critical loop
		* only for demonstration purposes.
		*/
		printf("Time since last turn: %ld.%06ld us Register=%x Latency= %d ns\n",
		(long)(now - previous) / 1000000,
		(long)(now - previous) % 1000000, registro, (long)((now - previous)-1000000000));
		previous = now;
		registro=~registro;
		digitalWrite_word(registro); // set pin 16 to "HIGH;		
	}

}
void catch_signal(int sig)
{
}

int main(int argc, char* argv[])
{
	if (!bcm2835_init())
		return 1;
  
	// Default configuration
	configMCP23s17(0x00000000);
	pinMode_word(0x0000); // sets pin 16 as an output
	pullupMode_word(0xFFFF); // enable the pull-up on pin 4
	inputInvert_word(0xFFFF); // disable inversion on pin 4
	
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	
	/* Avoids memory swapping for this program */
	mlockall(MCL_CURRENT|MCL_FUTURE);
	/*
	* Arguments: &task,
	* name,
	* stack size (0=default),
	* priority,
	* mode (FPU, start suspended, ...)
	*/
	rt_task_create(&demo_task, "trivial", 0, 99, 0);
	/*
	* Arguments: &task,
	* task function,
	* function argument
	*/
	rt_task_start(&demo_task, &demo, NULL);
	pause();
	rt_task_delete(&demo_task);
	return 0;
}
