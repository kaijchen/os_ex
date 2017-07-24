#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE	80
#define MAX_ARGN	((MAX_LINE + 1) / 2)

int parse_line(char *str, char **args, int lim)
{
	char *token;
	int n = 0;

	while ((token = strsep(&str, "\n\t ")) != NULL) {
		if (token[0] == '\0')
			continue;

		args[n] = token;

		if (++n == lim)
			break;
	}

	return n;
}

int main(void)
{
	char line[MAX_LINE + 1];
	char *args[MAX_ARGN + 1];
	char *str;
	int bg;
	int argn;
	int tmp;
	int wstatus, rv;
	pid_t pid;

	for (;;) {
		printf("osh> ");
		fflush(stdout);

		if (fgets(line, MAX_LINE + 1, stdin) == NULL)
			break;

		if (line[strlen(line) - 1] != '\n') {
			while ((tmp = getchar()) != EOF && tmp != '\n')
				;
		}

		argn = parse_line(line, args, MAX_ARGN);
		if (argn == 0) {
			continue;
		} else {
			bg = 0;
			args[argn] = NULL;
			str = args[argn - 1];
			tmp = strlen(str);
			if (str[tmp - 1] == '&') {
				bg = 1;
				if (tmp > 1)
					str[tmp - 1] = '\0';
				else
					args[--argn] = NULL;
			}
		}

		if ((pid = fork()) == 0) {
			if (execvp(args[0], args) == -1) {
				fprintf(stderr, "%s\n", strerror(errno));
				break;
			}
		} else {
			if (bg) {
				printf("[background] %d\n", pid);
			} else {
				waitpid(pid, &wstatus, WUNTRACED);
				rv = WEXITSTATUS(wstatus);
			}
		}
		while ((pid = waitpid(-1, &wstatus, WNOHANG)) > 0) {
			printf("[done] %d\n", pid);
		}
	}
	return 0;
}
