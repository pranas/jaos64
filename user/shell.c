
#include "shell.h"

static int running;

#define puts syscall_puts
#define puthex syscall_puthex
#define kmalloc syscall_kmalloc
#define kfree syscall_kfree
#define readline syscall_readline

static char* token(char* str, int num)
{
	int start, end, words, i;
	int in_word;
	in_word = 0;
	start = 0;
	end = 0;
	words = 0;

	if (num == 0)
		return (char*) 0;
	for (i = 0; i < strlen(str)+1; i++)
	{
		if (str[i] == '\0')
		{
			if (in_word)
			{
				end = i;
				in_word = 0;
				words++;
			}
			break;
		}
		if (str[i] != ' ')
		{
			if (!in_word)
			{
				start = i;
				in_word = 1;
			}
		}
		if (str[i] == ' ')
		{
			if (in_word)
			{
				end = i;
				in_word = 0;
				words++;
			}
		}
		if (words == num)
			break;
	}

	if (words != num)
		return (char*) 0;
	int len = end - start;
	char* token = (char*) kmalloc(len+1);
	strncpy(token, str + start, len);
	token[len] = '\0';
	return token;
}

static void main_loop()
{
	char* line;
	char* command, argument;

	while (running)
	{
		line = readline();
		command = argument = 0;
		command = token(line, 1);
		argument = token(line, 2);
		if (command)
		{
			puts("The command is "); puts(command); puts("\n");
			kfree(command);
		}
		if (argument)
		{
			puts("The first parameter is "); puts(argument); puts("\n");
			kfree(argument);
		}
		if (command && strncmp(command, "exit", strlen(command)))
		{
			puts("Exit command\n");
			running = 0;
		}
	}
	puts("Exiting...\n");
} 

void shell_init()
{
	running = 1;
	main_loop();
}

