#include "exit.h"

void exit()
{
    uint64_t pid = get_current_pid();
    
    // cleaning memory here

    change_task_status(pid, 2); // 2 means it will be removed during maintenance
    asm volatile("sti");
    asm volatile("int $0x20");
}