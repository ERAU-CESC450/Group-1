//#pragma once

//void isr_signal(int latency);

#pragma once  
//Tyler M7 Change

// Creates the ISR-to-task semaphore.
bool InterruptSystemInit();

// Deletes the ISR-to-task semaphore.
void InterruptSystemShutdown();

// Independent simulated asynchronous event source.
void SimulatedEventSourceTask(void *params);

// Task that performs meaningful event processing.
void InterruptEventHandlerTask(void *params);
