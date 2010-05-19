#define puts syscall_puts
#define exit syscall_exit

#include <stdint.h>

int main()
{
	puts("Hello World!\n");
    exit();
}
