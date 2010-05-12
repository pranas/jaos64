#define puts syscall_puts
#define fork syscall_fork

int main()
{
	if (fork() == 0)
	{
        for(;;)
        {
            puts("Ping! (child)\n");
        }
	}
	else
	{
        for(;;)
        {
            puts("Pong! (father)\n");
        }
	}
    return 0;
}
