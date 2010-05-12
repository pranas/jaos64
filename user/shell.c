
#include "shell.h"

static int running;

#define puts syscall_puts
#define putint syscall_putint
#define puthex syscall_puthex
#define kmalloc syscall_kmalloc
#define kfree syscall_kfree
#define readline syscall_readline


static char* token(char* str, int num, char* token);
static void main_loop();

int main()
{
	running = 1;
	main_loop();
}

static char* token(char* str, int num, char* token)
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
		if (str[i] == '\0' || str[i] == '\n' || str[i] == ' ')
		{
			if (in_word)
			{
				end = i;
				in_word = 0;
				words++;
			}
			if (str[i] == '\0' || str[i] == '\n')
				break;
		}
		else if (!in_word)
		{
			start = i;
			in_word = 1;
		}
		if (words == num)
			break;
	}

	if (words != num)
	{
		memset(token, 0, sizeof(token));
		return token;
	}
	int len = end - start;
	strncpy(token, str + start, len);
	token[len] = '\0';
	return token;
}

static void main_loop()
{
	char* line, cmd, arg1;
	char buffer[81];
	char command[81];
	char args[16][81];

	while (running)
	{
		line = (char*) readline(buffer);
		memset(command, 0, sizeof(command));
		memset(args[0], 0, sizeof(args[0]));
		token(line, 1, command);
		token(line, 2, args[0]);
		if (strlen(command) > 1)
		{
			puts("The command is "); puts(command); puts("\n");
		}
		if (strlen(args[0]) > 1)
		{
			puts("The first parameter is "); puts(args[0]); puts("\n");
		}
		if (command && strncmp(command, "exit", strlen(command)))
		{
			puts("Exit command\n");
			running = 0;
		}
	}
	puts("Exiting...\n");
} 

