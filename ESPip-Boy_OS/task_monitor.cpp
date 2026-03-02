#include "log.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TASK_MONITOR_INTERVAL_MS 2000
#define MAX_TASKS 20

void TaskMonitor(void *pvParameters)
{
    while (true)
    {
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("===== FreeRTOS Task Monitor =====");

        UBaseType_t taskCount = uxTaskGetNumberOfTasks();
        TaskStatus_t *taskStatusArray;

        taskStatusArray = (TaskStatus_t *)pvPortMalloc(taskCount * sizeof(TaskStatus_t));

        if (taskStatusArray != NULL)
        {
            uint32_t totalRunTime;

            taskCount = uxTaskGetSystemState(
                taskStatusArray,
                taskCount,
                &totalRunTime
            );

            if (totalRunTime == 0)
                totalRunTime = 1;

            DEBUG_PRINTF("%-16s %-6s %-6s %-8s %-6s %-6s\n",
                          "Name", "Core", "Prio", "CPU %", "Stack", "State");

            for (UBaseType_t i = 0; i < taskCount; i++)
            {
                uint32_t cpuPercent =
                    (taskStatusArray[i].ulRunTimeCounter * 100UL) / totalRunTime;

                const char *stateStr;
                switch (taskStatusArray[i].eCurrentState)
                {
                case eRunning:
                    stateStr = "Run";
                    break;
                case eReady:
                    stateStr = "Ready";
                    break;
                case eBlocked:
                    stateStr = "Block";
                    break;
                case eSuspended:
                    stateStr = "Susp";
                    break;
                case eDeleted:
                    stateStr = "Del";
                    break;
                default:
                    stateStr = "?";
                    break;
                }

#if CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
                BaseType_t core = taskStatusArray[i].xCoreID;
#else
                BaseType_t core = -1;
#endif

                DEBUG_PRINTF("%-16s %-6d %-6d %-8lu %-6u %-6s\n",
                              taskStatusArray[i].pcTaskName,
                              core,
                              taskStatusArray[i].uxCurrentPriority,
                              cpuPercent,
                              taskStatusArray[i].usStackHighWaterMark,
                              stateStr);
            }

            vPortFree(taskStatusArray);
        }
        else
        {
            DEBUG_PRINTLN("Failed to allocate memory for task stats.");
        }

        DEBUG_PRINTLN("=================================\n");

        vTaskDelay(pdMS_TO_TICKS(TASK_MONITOR_INTERVAL_MS));
    }
}

void init_task_monitor() {
      xTaskCreatePinnedToCore(
        TaskMonitor,
        "TaskMonitor",
        4096,
        NULL,
        1,          // Low priority
        NULL,
        1           // Run on core 1
    );
}
