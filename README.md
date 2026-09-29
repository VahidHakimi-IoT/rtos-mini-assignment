
RTOS Mini Assignment: Two FreeRTOS Tasks on ESP32-S3

1. Hardware and software

Board: ESP32-S3-DevKitC-1 v1.1
Onboard output: addressable RGB LED, data pin GPIO 38
Software: Arduino IDE 2.x, "esp32 by Espressif Systems" 
Serial Monitor: 115200 baud
Core: both tasks pinned to core 1 in every experiment

2. Baseline implementation

Two FreeRTOS tasks created with `xTaskCreatePinnedToCore()` on core 1:
Task A: prints "Task A alive" every 1000 ms using `vTaskDelay(pdMS_TO_TICKS(1000))`.
Task B: blinks the RGB LED (red, brightness 50) with 500 ms ON and 500 ms OFF using `vTaskDelay(pdMS_TO_TICKS(500))` for both waits.
Baseline priorities: Task A = 1, Task B = 1.
(images/serial-monitor.png)
Observation: "Task A alive" appeared repeatedly, about once per second, and the LED blinked periodically. 

3. Prediction and observation table

> Predictions were written before testing and were not changed afterwards. 

Scenario A	

A = 1, B = 1  Both tasks ran. "Task A alive" printed about once per second and the LED blinked 500 ms on / 500 ms off.	

 Scenario B	

A = 2, B = 1 Both tasks ran. "Task A alive" kept appearing and the LED kept blinking. No visible difference from baseline. Yes. Task A has higher priority but is Blocked almost all the time, so it cannot dominate. The delay sets the period, not the priority.
C	A = 1, B = 2	Both tasks ran. "Task A alive" kept appearing and the LED kept blinking on and off. No visible difference from baseline. 	

4. Priority experiments

Only the priority argument of `xTaskCreatePinnedToCore()` was changed. All `vTaskDelay()` calls stayed active and both tasks stayed on core 1.
Experiment	Task A priority	Task B priority	Both tasks kept running?
A	1	1	Yes
B	2	1	Yes
C	1	2	Yes
The observed periods matched the baseline in all three cases. A higher priority did not make a periodic task run more often, because the delay period sets how often the task runs. Priority only matters at the moments when both tasks are Ready at the same time, and those moments are very short.

5. Starvation experiment (temporary)

Test 1: delay removed, `Serial.println` kept
Priorities: Task A = 2, Task B = 1, both on core 1
Change: `vTaskDelay()` commented out in Task A only
Serial Monitor: "Task A alive" appeared much faster than once per second
LED: still blinking normally, about 500 ms on / 500 ms off
Interpretation: Task A never blocked in `vTaskDelay`, but `Serial.println` can still make it wait inside the serial driver. While it waits it is Blocked, so Task B can run when it becomes Ready. The loop was not truly non-blocking, so starvation did not appear.
Test 2: busy loop, no delay, no Serial output
Priorities: Task A = 2, Task B = 1, both on core 1
Change: Task A replaced with a loop that never blocks:
```cpp
void taskA(void *parameter) {
  for (;;) {
    volatile unsigned long counter = 0;
    for (unsigned long i = 0; i < 1000000UL; i++) {
      counter++;
    }
  }
}
```
LED: stayed on permanently
Serial Monitor: [describe exactly what you saw. Any "Task A alive" lines were probably left over from earlier runs. Note whether any watchdog message appeared.]
Interpretation: Task A stayed Running and never entered Blocked. Task B turned the LED on, entered Blocked, and became Ready when its delay expired, but the scheduler always chose the higher-priority Task A. Task B never ran its "LED off" line, so the LED stayed on.
After both tests the original code was restored and I confirmed that both tasks worked normally again.

6. Ready, Running and Blocked explanation

A task is Running when it is the one executing on core 1, and only one task can run at a time. When Task A or Task B calls `vTaskDelay()`, it moves from Running to Blocked, so it uses no CPU until its delay expires. When the delay expires, the task becomes Ready and waits for the scheduler, which always selects the highest-priority Ready task. Tasks of equal priority take turns.
In scenarios A, B and C both tasks kept running and the timing looked the same as the baseline: Task A printed about once per second and the LED blinked about 500 ms on and 500 ms off. Priority did not visibly matter because both tasks were Blocked almost all the time and were rarely Ready together. The delay, not the priority, set each period.
In the first starvation test, Task A (priority 2) had no delay but still called `Serial.println`. It printed much faster, yet the LED kept blinking, so Task A was probably still being blocked inside the serial driver, letting Task B run. In the second test, Task A was a busy loop with no delay and no Serial output. The LED stayed on permanently. Task A stayed Running and never Blocked, while Task B stayed Ready but was never selected because a higher-priority task was always available. This is starvation.

7. Final restored configuration

The starvation experiments were temporary. All `vTaskDelay()` calls were restored in the submitted code, and I confirmed on the board that both tasks work normally again.
Board: ESP32-S3-DevKitC-1 v1.1
RGB LED pin: GPIO 38
Final priorities: Task A = 1, Task B = 1
Core: both tasks pinned to core 1
Priorities per experiment: A (1/1), B (2/1), C (1/2), starvation tests (A = 2, B = 1)

Will both tasks keep running?

Will the periods change (1000 ms for A, 500 ms on/off for B)?

Will you see any visible difference from the baseline?

Scenario A: both priority 1

Both tasks keep running? Yes. Each task runs for a fraction of a millisecond, then blocks in vTaskDelay. The CPU is idle almost all the time.
Periods change? No. Task A stays at 1000 ms and Task B at 500 ms on / 500 ms off, the same as your baseline.
Visible difference? None. This is the baseline.

Scenario B: Task A = 2, Task B = 1

Both tasks keep running? Yes. Task A blocks after printing, so Task B still gets the CPU.
Periods change? No. The delay sets the period, not the priority. Task A doesn't print more often just because it has higher priority.
Visible difference? None you can see. The only effect is at instants when both tasks are Ready at the same time (for example at startup and every 1000 ms when both wake together). Then Task A goes first and Task B waits a very short time, likely under a few milliseconds, which you can't see.

Scenario C: Task A = 1, Task B = 2

Both tasks keep running? Yes, for the same reason.
Periods change? No.
Visible difference? None you can see. At the moments they are Ready together, Task B goes first and Task A is delayed by a tiny amount (the LED write is very quick, so it's negligible). The LED blinking and serial output look the same as the baseline.

Common reasoning for all three: a periodic task spends nearly all its time Blocked, so priority only matters when both are Ready simultaneously, and those overlaps last a very short time. Priority decides who runs first, not how often.

scenario B:

Priorities used: Task A = 2, Task B = 1, both on core 1
Task A: "Task A alive" appears repeatedly, about once per second
Task B: LED still blinking
Difference from baseline: none visible

scenario C:

Scenario C matches the prediction: both tasks kept running and nothing looks different from the baseline. Record it like this:

Priorities used: Task A = 1, Task B = 2, both on core 1
Both tasks running? Yes
Task A: "Task A alive" keeps appearing
Task B: LED still blinking on and off
Difference from baseline: none visible

Priorities used: Task A = 2, Task B = 1, both on core 1, delay removed from Task A only
Serial Monitor: "Task A alive" appeared much faster than once per second
RGB LED: still blinking normally, about 500 ms on and 500 ms off
Interpretation: Task A did not stay Running continuously. It was repeatedly Blocked inside the serial driver, which let Task B run when it became Ready. Starvation did not occur in my setup because the loop was not truly non-blocking.


starvation:

Test: Task A = 2, Task B = 1, both on core 1. Task A was a busy loop with no vTaskDelay and no Serial output.
Observation: the LED stayed on permanently and the Serial Monitor showed no new output (or a watchdog message, if you saw one).
Explanation: Task A stayed Running and never entered Blocked. Task B stayed Ready but was never selected because a higher-priority task was always available.

The starvation experiments were temporary. All vTaskDelay() calls were restored in the submitted code. Final configuration: ESP32-S3-DevKitC-1 v1.1, RGB LED on GPIO 38, Task A and Task B at priority 1, both pinned to core 1

 







