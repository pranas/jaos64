#define puts syscall_puts
#define putint syscall_putint
#define fork syscall_fork
#define exec syscall_exec
#define exit syscall_exit

#include <stdint.h>

int main()
{
    for(;;)
    {
        uint64_t pid = fork();
        if (pid == 0)
        {
            puts("Child returned...\n");
        }
        else
        {
            puts("Forked pid: ");
            putint(pid);
            puts("\n");
        }
    }
    asm("int $0x20");
    exit();
}
