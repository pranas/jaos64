#define puts syscall_puts
#define putint syscall_putint
#define fork syscall_fork
#define exec syscall_exec
#define exit syscall_exit
#define sleep syscall_sleep

#include <stdint.h>
#include "syscall.h"

int main()
{
    if (fork() == 0)
    {
        for(;;)
        {
            puts("\nPong!\n");
            sleep();
        }        
    }
    else
    {
        for(;;)
        {
            puts("\nPing?\n");
            sleep();
        }
    }
    exit();
}
