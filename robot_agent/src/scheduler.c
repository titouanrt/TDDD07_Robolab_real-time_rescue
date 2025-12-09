/**
 * @file	scheduler.c
 * @author  Eriks Zaharans and Massimiiliano Raciti
 * @date    1 Jul 2013
 *
 * @section DESCRIPTION
 *
 * Cyclic executive scheduler library.
 */

/* -- Includes -- */
/* system libraries */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
/* project libraries */
#include "scheduler.h"
#include "task.h"
#include "timelib.h"

/* -- Defines -- */

/* -- Functions -- */

/**
 * Initialize cyclic executive scheduler
 * @param Void
 * @return Pointer to scheduler structure
 */
scheduler_t *scheduler_init(void)
{
	// Allocate memory for Scheduler structure
	scheduler_t *ces = (scheduler_t *)malloc(sizeof(scheduler_t));

	return ces;
}

/**
 * Deinitialize cyclic executive scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_destroy(scheduler_t *ces)
{
	// Free memory
	free(ces);
}

/**
 * Start scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_start(scheduler_t *ces)
{
	// Set timers
	timelib_timer_set(&ces->tv_started);
	timelib_timer_set(&ces->tv_cycle);
}

/**
 * Wait (sleep) till end of minor cycle
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_wait_for_timer(scheduler_t *ces)
{
	int sleep_time; // Sleep time in microseconds

	// Calculate time till end of the minor cycle
	sleep_time = (ces->minor * 1000) - (int)(timelib_timer_get(ces->tv_cycle) * 1000);

	// Add minor cycle period to timer
	timelib_timer_add_ms(&ces->tv_cycle, ces->minor);

	// Check for overrun and execute sleep only if there is no
	if (sleep_time > 0)
	{
		// Go to sleep (multipy with 1000 to get miliseconds)
		usleep(sleep_time);
	}
}

/**
 * Execute task
 * @param ces Pointer to scheduler structure
 * @param task_id Task ID
 * @return Void
 */
void scheduler_exec_task(scheduler_t *ces, int task_id)
{
	switch (task_id)
	{
	// Mission
	case s_TASK_MISSION_ID:
		task_mission();
		break;
	// Navigate
	case s_TASK_NAVIGATE_ID:
		task_navigate();
		break;
	// Control
	case s_TASK_CONTROL_ID:
		task_control();
		break;
	// Refine
	case s_TASK_REFINE_ID:
		task_refine();
		break;
	// Report
	case s_TASK_REPORT_ID:
		task_report();
		break;
	// Communicate
	case s_TASK_COMMUNICATE_ID:
		task_communicate();
		break;
	// Collision detection
	case s_TASK_AVOID_ID:
		task_avoid();
		break;
	// Other
	default:
		// Do nothing
		break;
	}
}

/**
 * Run scheduler
 * @param ces Pointer to scheduler structure
 * @return Void
 */
void scheduler_run(scheduler_t *ces)
{
	/* --- Local variables (define variables here) --- */

	/* --- Set minor cycle period --- */
	// ces->minor = ...;

	/* --- Write your code here --- */

	/* --- Local variables (define variables here) --- */
	double time[8] = {0,0,0,0,0,0,0,0};
	int cycle = 0;
	int cpt = 0;
	/* --- Set minor cycle period --- */
	ces->minor = 125;

	/* --- Write your code here --- */
	scheduler_start(ces);
	timelib_timer_set(&ces->tv_cycle);
	while (1)
	{
		switch (cycle)
		{
		case 125:
			timelib_timer_set(&ces->tv_cycle);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			scheduler_exec_task(ces, s_TASK_MISSION_ID);
			//if(cpt > 0) time[0] += timelib_timer_get(ces->tv_cycle);
			printf("125 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			break;

		case 250:
			timelib_timer_set(&ces->tv_cycle);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			scheduler_exec_task(ces, s_TASK_REFINE_ID);
			scheduler_exec_task(ces, s_TASK_REPORT_ID);
			scheduler_exec_task(ces, s_TASK_COMMUNICATE_ID);
			//if(cpt > 0) time[1] += timelib_timer_get(ces->tv_cycle);

			printf("250 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			// time[1]+= timelib_timer_reset(ces);

			break;

		case 375:
			timelib_timer_set(&ces->tv_cycle);
			scheduler_exec_task(ces, s_TASK_CONTROL_ID);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			if(cpt > 0) time[2] += timelib_timer_get(ces->tv_cycle);

			printf("375 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			// time[2]+= timelib_timer_reset(ces);

			break;

		case 500:
			timelib_timer_set(&ces->tv_cycle);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			scheduler_exec_task(ces, s_TASK_REFINE_ID);
			scheduler_exec_task(ces, s_TASK_REPORT_ID);
			if(cpt > 0) time[3] += timelib_timer_get(ces->tv_cycle);

			printf("500 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			// time[3]+= timelib_timer_reset(ces);
			break;

		case 625:
			timelib_timer_set(&ces->tv_cycle);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			scheduler_exec_task(ces, s_TASK_MISSION_ID);
			if(cpt > 0) time[4] += timelib_timer_get(ces->tv_cycle);
			printf("625 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			// time[4]+= timelib_timer_reset(ces);
			break;

		case 750:
			timelib_timer_set(&ces->tv_cycle);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			scheduler_exec_task(ces, s_TASK_REFINE_ID);
			scheduler_exec_task(ces, s_TASK_REPORT_ID);
			scheduler_exec_task(ces, s_TASK_COMMUNICATE_ID);
			if(cpt > 0) time[5] += timelib_timer_get(ces->tv_cycle);
			printf("750 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			// time[5]+= timelib_timer_reset(ces);

			break;

		case 875:
			timelib_timer_set(&ces->tv_cycle);

			scheduler_exec_task(ces, s_TASK_NAVIGATE_ID);
			scheduler_exec_task(ces, s_TASK_CONTROL_ID);
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			if(cpt > 0) time[6] += timelib_timer_get(ces->tv_cycle);
			printf("875 stop : %f\n", timelib_timer_get(ces->tv_cycle));
			// time[6]+= timelib_timer_reset(ces);

			break;

		case 1000:
			timelib_timer_set(&ces->tv_cycle);

			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			scheduler_exec_task(ces, s_TASK_REFINE_ID);
			scheduler_exec_task(ces, s_TASK_REPORT_ID);
			//if(cpt > 0) time[7] += timelib_timer_get(ces->tv_cycle);
			// time[7]+=timelib_timer_reset(ces);
			cycle = 0;
			printf("1000 stop : %f\n", timelib_timer_get(ces->tv_cycle));

			break;

		default:
			scheduler_exec_task(ces, s_TASK_AVOID_ID);
			break;
		}
		cycle += 125;

		cpt++;
		if (cpt == 50)
		{
			// mean calulation output
			/*
			for (int i = 0; i < 8; i++)
			{
				printf("Mean time for step %d: %f ms\n", (i + 1) * 125, time[i]);
			}
			*/
		}
		scheduler_wait_for_timer(ces);
	}
}