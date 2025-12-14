#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/driver/ext2.h"

struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX] = {0};

struct ProcessManager process_manager = {
    .active_process_count = 0,
    ._process_pid_used = {false},
};

static uint32_t process_generate_new_pid(void)
{
    for (uint32_t pid = 0; pid < PROCESS_COUNT_MAX; pid++)
    {
        if (!process_manager._process_pid_used[pid])
        {
            process_manager._process_pid_used[pid] = true;
            return pid;
        }
    }
    return UINT32_MAX; // Indicate failure to generate PID
}

static int32_t process_list_get_inactive_index(void)
{
    for (int32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (_process_list[i].metadata.state == PROCESS_STATE_INACTIVE)
        {
            return i;
        }
    }
    return -1; // Indicate no inactive process found
}

static uint32_t ceil_div(uint32_t numerator, uint32_t denominator)
{
    return (numerator + denominator - 1) / denominator;
}

struct ProcessControlBlock *process_get_current_running_pcb_pointer(void)
{
    for (int32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        if (_process_list[i].metadata.state == PROCESS_STATE_RUNNING && process_manager._process_pid_used[_process_list[i].metadata.pid])
        {
            return &(_process_list[i]);
        }
    }
    return NULL;
}

int32_t process_create_user_process(struct EXT2DriverRequest request)
{
    int32_t retcode = PROCESS_CREATE_SUCCESS;
    if (process_manager.active_process_count >= PROCESS_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t)request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE)
    {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX)
    {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB
    int32_t p_index = process_list_get_inactive_index();
    if (p_index < 0)
    {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);

    // Initialize metadata
    new_pcb->metadata.pid = process_generate_new_pid();
    new_pcb->metadata.state = PROCESS_STATE_INACTIVE;
    memset(new_pcb->metadata.name, 0, PROCESS_NAME_LENGTH_MAX);
    
    // Copy process name from request filename
    uint32_t name_len = request.name_len;
    if (name_len > PROCESS_NAME_LENGTH_MAX - 1)
        name_len = PROCESS_NAME_LENGTH_MAX - 1;
    memcpy(new_pcb->metadata.name, request.name, name_len);
    new_pcb->metadata.name[name_len] = '\0';

    // Create new page directory for the user process
    struct PageDirectory *new_page_dir = paging_create_new_page_directory();
    if (new_page_dir == NULL)
    {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Allocate and map user memory pages
    uint32_t virtual_addr = 0;
    uint32_t pages_allocated = 0;

    for (uint32_t i = 0; i < page_frame_count_needed && i < PROCESS_PAGE_FRAME_COUNT_MAX; i++)
    {
        if (!paging_allocate_user_page_frame(new_page_dir, (void *)virtual_addr))
        {
            retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
            goto exit_cleanup_page_dir;
        }

        new_pcb->memory.virtual_addr_used[i] = (void *)virtual_addr;
        pages_allocated++;
        virtual_addr += PAGE_FRAME_SIZE;
    }

    new_pcb->memory.page_frame_used_count = pages_allocated;

    /*
     * Copy the executable into the allocated frames using a temporary kernel mapping.
     * We use kernel page directory entry 0x301 (virtual 0xC0400000) as a temporary mapping
     * for each destination frame, allowing us to access arbitrary physical frames from kernel mode.
     */
    uint32_t copied = 0;
    struct PageDirectoryEntryFlag temp_flag = {
        .present_bit = 1,
        .write_bit = 1,
        .user_supervisor = 0,
        .page_write_through = 0,
        .page_cache_disable = 0,
        .accessed_bit = 0,
        .dirty_bit = 0,
        .use_pagesize_4_mb = 1,
    };

    struct PageDirectory *kernel_page_dir = paging_get_current_page_directory_addr();

    for (uint32_t frame_idx = 0; frame_idx < pages_allocated && copied < request.buffer_size; frame_idx++)
    {
        uint32_t dest_virtual = frame_idx * PAGE_FRAME_SIZE;
        uint32_t page_index = (dest_virtual >> 22) & 0x3FF;
        uint32_t frame_index = new_page_dir->table[page_index].lower_address;

        // Verify the frame is valid
        if (frame_index >= PAGE_FRAME_MAX_COUNT)
        {
            retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
            goto exit_cleanup_page_dir;
        }

        // Temporarily map this physical frame at kernel virtual address 0xC0400000
        // (using page directory entry 0x301)
        void *phys_addr = (void *)(frame_index << 22);
        update_page_directory_entry(kernel_page_dir, phys_addr, (void *)0xC0400000, temp_flag);

        // Copy as much as fits in this frame or remaining data
        uint32_t remaining = request.buffer_size - copied;
        uint32_t to_copy = remaining > PAGE_FRAME_SIZE ? PAGE_FRAME_SIZE : remaining;
        memcpy((void *)0xC0400000, (void *)((uint32_t)request.buf + copied), to_copy);

        // Clear the temporary mapping
        struct PageDirectoryEntryFlag clear_flag = {0};
        update_page_directory_entry(kernel_page_dir, NULL, (void *)0xC0400000, clear_flag);

        copied += to_copy;
    }

    // Initialize process context
    new_pcb->context.eip = 0; // Entry point at address 0 in user space
    new_pcb->context.eflags = CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;
    new_pcb->context.page_directory_virtual_addr = new_page_dir;

    // Initialize CPU registers (all zero except for stack and segments)
    memset(&new_pcb->context.cpu, 0, sizeof(struct CPURegister));

    // Set user stack pointer to end of allocated memory
    new_pcb->context.cpu.stack.esp = virtual_addr - 4; // Leave 4 bytes of space from the end

    // Set segment registers to user data segment selector (0x20 | 0x3 = 0x23)
    new_pcb->context.cpu.segment.ds = 0x23;
    new_pcb->context.cpu.segment.es = 0x23;
    new_pcb->context.cpu.segment.fs = 0x23;
    new_pcb->context.cpu.segment.gs = 0x23;

    // Set process state to ready
    new_pcb->metadata.state = PROCESS_STATE_READY;
    process_manager.active_process_count++;

exit_cleanup_page_dir:
    if (retcode != PROCESS_CREATE_SUCCESS)
    {
        // Clean up allocated page directory on failure
        for (uint32_t i = 0; i < pages_allocated; i++)
        {
            paging_free_user_page_frame(new_page_dir, new_pcb->memory.virtual_addr_used[i]);
        }
        paging_free_page_directory(new_page_dir);
    }
    return retcode;

exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid)
{
    for (int32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        struct ProcessControlBlock *pcb = &(_process_list[i]);
        if (pcb->metadata.state != PROCESS_STATE_INACTIVE && pcb->metadata.pid == pid)
        {
            // Cannot destroy currently running process
            if (pcb->metadata.state == PROCESS_STATE_RUNNING)
            {
                return false;
            }
            
            // Free user page frames
            struct PageDirectory *page_dir = pcb->context.page_directory_virtual_addr;
            for (uint32_t j = 0; j < pcb->memory.page_frame_used_count; j++)
            {
                paging_free_user_page_frame(page_dir, pcb->memory.virtual_addr_used[j]);
            }

            // Free page directory
            paging_free_page_directory(page_dir);

            // Mark PID as unused
            process_manager._process_pid_used[pid] = false;

            // Reset PCB
            memset(pcb, 0, sizeof(struct ProcessControlBlock));

            process_manager.active_process_count--;
            return true;
        }
    }
    return false; // Process with given PID not found or already inactive
}

void process_exec(char *exec_filename, uint32_t parent_inode, int32_t *return_code)
{
    // Copy the filename to a local buffer BEFORE switching page directories
    // because exec_filename points to user space memory
    char filename_copy[128];
    uint32_t filename_len = strlen(exec_filename);
    if (filename_len >= 128)
    {
        *return_code = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        return;
    }
    memcpy(filename_copy, exec_filename, filename_len + 1);
    
    // We need to temporarily switch to kernel page directory to allocate
    // a buffer for reading the executable
    struct PageDirectory *kernel_page_dir = &_paging_kernel_page_directory;
    struct PageDirectory *current_page_dir = paging_get_current_page_directory_addr();
    
    // Switch to kernel page directory
    paging_use_page_directory(kernel_page_dir);
    
    // Allocate a user page frame in kernel's page directory at a safe address
    // Use virtual address 0 since we can control it
    bool alloc_success = paging_allocate_user_page_frame(kernel_page_dir, (void *)0);
    if (!alloc_success)
    {
        // Restore original page directory
        paging_use_page_directory(current_page_dir);
        *return_code = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        return;
    }
    
    // Now create the EXT2 request with buffer at address 0
    struct EXT2DriverRequest request = {
        .buf = (void *)0,
        .name = filename_copy,  // Use the local copy
        .name_len = (uint8_t)filename_len,
        .parent_inode = parent_inode,
        .buffer_size = 0x400000, // 4 MB max size
        .is_directory = false,
    };
    
    // Read the executable
    int8_t read_result = read(request);
    if (read_result != 0)
    {
        // Free the allocated frame and restore page directory
        paging_free_user_page_frame(kernel_page_dir, (void *)0);
        paging_use_page_directory(current_page_dir);
        *return_code = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        return;
    }
    
    // Now create the process - process_create_user_process will copy from 
    // the buffer to newly allocated frames for the new process
    int32_t result = process_create_user_process(request);
    
    // Free the temporary buffer frame
    paging_free_user_page_frame(kernel_page_dir, (void *)0);
    
    // Restore original page directory (caller's process)
    // MUST do this BEFORE writing to return_code since it's a user-space pointer
    paging_use_page_directory(current_page_dir);
    
    // Now we can safely write to return_code since we're back in caller's page directory
    *return_code = result;
}

void process_info(uint32_t pid, ProcessMetadata *process_metadata)
{
    for (int32_t i = 0; i < PROCESS_COUNT_MAX; i++)
    {
        struct ProcessControlBlock *pcb = &(_process_list[i]);
        if (pcb->metadata.state != PROCESS_STATE_INACTIVE && pcb->metadata.pid == pid && process_manager._process_pid_used[pid])
        {
            process_metadata->pid = pcb->metadata.pid;
            process_metadata->state = pcb->metadata.state;
            memcpy(process_metadata->name, pcb->metadata.name, PROCESS_NAME_LENGTH_MAX);
            return;
        }
    }
    // If process not found, set pid to UINT32_MAX to indicate invalid
    process_metadata->pid = UINT32_MAX;
}