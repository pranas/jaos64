#define puts syscall_puts
#define fork syscall_fork

int main()
{
//    asm volatile("xchg %bx, %bx");
	if (fork() == 0)
	{
        for(;;)
        {
            puts("Child\n");
            asm volatile("hlt");
        }
	}
	else
	{
        for(;;)
        {
            puts("Father\n");
            asm volatile("hlt");
        }
	}
    return 0;
}
