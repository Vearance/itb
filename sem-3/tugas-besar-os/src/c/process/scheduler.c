#include "header/process/scheduler.h"
#include "header/interrupt/interrupt.h"
#include "header/cpu/portio.h"
#include "header/process/process.h"

uint32_t current_running_process_index = 0;
static bool timer_interrupt_activated = false;

void activate_timer_interrupt(void)
{
    if (timer_interrupt_activated) return;
    timer_interrupt_activated = true;
    
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t)(pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t)((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}

void scheduler_init(void)
{
    current_running_process_index = 0;
    bool process_found = false;
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (_process_list[i].metadata.state == PROCESS_STATE_READY && 
            process_manager._process_pid_used[_process_list[i].metadata.pid])
        {
            _process_list[i].metadata.state = PROCESS_STATE_RUNNING;
            current_running_process_index = i;
            process_found = true;
            break;
        }
    }

    if (!process_found)
    {
        __asm__("cli; hlt");
        while (1)
        {
        };
    }

    // NOTE: Don't activate timer interrupt here!
    // It will be activated in scheduler_switch_to_next_process
    // to ensure no timer interrupt occurs before the first process starts
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx)
{
    struct ProcessControlBlock *current_pcb = &_process_list[current_running_process_index];
    uint32_t current_pid = current_pcb->metadata.pid;

    if (current_pcb->metadata.state != PROCESS_STATE_RUNNING || 
        !process_manager._process_pid_used[current_pid])
    {
        return;
    }

    current_pcb->context = ctx;
    current_pcb->metadata.state = PROCESS_STATE_READY;
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void)
{
    uint32_t starting_index = current_running_process_index;
    uint32_t next_index = (current_running_process_index + 1) % PROCESS_COUNT_MAX;
    bool process_found = false;

    // Search for the next ready process
    while (next_index != starting_index)
    {
        uint32_t next_pid = _process_list[next_index].metadata.pid;
        if (_process_list[next_index].metadata.state == PROCESS_STATE_READY && 
            process_manager._process_pid_used[next_pid])
        {
            process_found = true;
            break;
        }
        next_index = (next_index + 1) % PROCESS_COUNT_MAX;
    }

    if (!process_found)
    {
        // No other process found, continue running the current process
        next_index = starting_index;
    }

    // Update process states
    if (_process_list[starting_index].metadata.state == PROCESS_STATE_RUNNING)
    {
        _process_list[starting_index].metadata.state = PROCESS_STATE_READY;
    }

    current_running_process_index = next_index;
    _process_list[current_running_process_index].metadata.state = PROCESS_STATE_RUNNING;

    // Switch page directory to the new process
    paging_use_page_directory(_process_list[current_running_process_index].context.page_directory_virtual_addr);

    // Activate timer interrupt (safe to do here, right before context switch to user mode)
    activate_timer_interrupt();

    // Perform context switch
    process_context_switch(_process_list[current_running_process_index].context);
}

void scheduler_handle_timer_interrupt(struct InterruptFrame frame)
{
    struct ProcessControlBlock *current_pcb = &_process_list[current_running_process_index];
    uint32_t current_pid = current_pcb->metadata.pid;

    // Only save context if we have a valid running process
    if (current_pcb->metadata.state == PROCESS_STATE_RUNNING && 
        process_manager._process_pid_used[current_pid])
    {
        // Build Context from InterruptFrame
        // The InterruptFrame contains the CPU state at the time of interrupt
        struct Context ctx;
        
        // Copy CPU registers (esp is already set correctly by intsetup.s for interprivilege)
        ctx.cpu = frame.cpu;
        
        // EIP and EFLAGS from interrupt stack
        ctx.eip = frame.int_stack.eip;
        ctx.eflags = frame.int_stack.eflags;
        
        // Keep current page directory
        ctx.page_directory_virtual_addr = current_pcb->context.page_directory_virtual_addr;
        
        // Save context to PCB
        current_pcb->context = ctx;
        current_pcb->metadata.state = PROCESS_STATE_READY;
    }

    // Switch to next process
    scheduler_switch_to_next_process();
}