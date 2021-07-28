/// Routines for synchronizing threads.
///
/// This is the only implementation of a synchronizing primitive that comes
/// with base Nachos.
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "semaphore.hh"
#include "system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
    dependentTheads = new List<Thread *>;
    dependentTheadsOldPriorities = new List<int>;
    maxPriority = 0; // Inicializamos la mayor prioridad de un hilo
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;

    // We revert the priorities to their original values
    while (!dependentTheads->IsEmpty() && !dependentTheadsOldPriorities->IsEmpty()) {
        Thread * thread = dependentTheads->Pop();
        int priority = dependentTheadsOldPriorities->Pop();

        thread->SetPriority(priority);
    }

    // A sanity check that the lengths must be equal
    ASSERT(dependentTheads->IsEmpty() && dependentTheadsOldPriorities->IsEmpty())

    delete dependentTheads;
    delete dependentTheadsOldPriorities;
}

const char *
Semaphore::GetName() const
{
    return name;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.

        queue->Append(currentThread);  // So go to sleep.

        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    Thread *thread = queue->Pop();
    if (thread != nullptr) {
        // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    }
    value++;

    interrupt->SetLevel(oldLevel);
}

void 
Semaphore::ManageDependencyInversion(Thread * thread) 
{
    // Keep track of the highest priority 
    int threadPriority = currentThread->GetPriority();
    if (threadPriority < maxPriority) 
        maxPriority = threadPriority
    
    if (!dependentTheads.Has(thread)) {
        dependentTheads->Append(thread)
        dependentTheadsOldPriorities->Append(currentThreadPriority)
    }

    thread->SetPriority(maxPriority)
}
