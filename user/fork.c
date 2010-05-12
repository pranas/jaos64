#define puts syscall_puts
#define fork syscall_fork

int main()
{
	if (fork() == 0)
	{
        for(;;)
        {
            puts("Child\n");
        }
	}
	else
	{
        for(;;)
        {
            puts("Father\n");
        }
	}
    return 0;
}
