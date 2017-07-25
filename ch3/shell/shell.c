#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE	80
#define MAX_ARGN	((MAX_LINE + 1) / 2)
#define HISTORY	10

int parse_line(char *str, char **args, int lim)
{
	char *token;
	int n = 0;

	while ((token = strsep(&str, "\t ")) != NULL) {
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
	char *history[HISTORY];
	char *line;
	char buf[MAX_LINE + 1];
	char *args[MAX_ARGN + 1];
	char *str;
	int head = 0, count = 0;
	int bg;
	int argn;
	int tmp, index;
	int wstatus, rv;
	pid_t pid;
	size_t len;

	memset(history, 0, sizeof(history));

	for (;;) {
		printf("osh> ");
		fflush(stdout);

		if (fgets(buf, MAX_LINE + 1, stdin) == NULL)
			break;

		len = strlen(buf);
		if (buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
		} else {
			while ((tmp = getchar()) != EOF && tmp != '\n')
				;
		}

		while ((pid = waitpid(-1, &wstatus, WNOHANG)) > 0) {
			printf("[bg done] %d\n", pid);
		}

		if (buf[0] == '!') {
			if (buf[1] == '!') {
				if (count == 0) {
					fprintf(stderr, "No commands in history.\n");
					continue;
				}
				index = (head + HISTORY - 1) % HISTORY;
			} else if (isdigit(buf[1])) {
				sscanf(&buf[1], "%d", &tmp);
				if (tmp == 0 || tmp > count) {
					fprintf(stderr, "No such commands in history\n");
					continue;
				}
				index = (head + HISTORY - tmp) % HISTORY;
			} else {
				fprintf(stderr, "Invalid syntax");
				continue;
			}
			strcpy(buf, history[index]);
			printf("%s\n", buf);
		}

		line = strdup(buf);

		argn = parse_line(buf, args, MAX_ARGN);

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

		if (strcmp(args[0], "exit") == 0) {
			break;
		} else if (strcmp(args[0], "history") == 0) {
			index = (head + HISTORY - count) % HISTORY;
			for (int i = count; i > 0; i--) {
				printf("%d %s\n", i, history[index]);
				index = (index + 1) % HISTORY;
			}
			free(line);
			continue;
		} else if ((pid = fork()) == 0) {
			if (execvp(args[0], args) == -1) {
				fprintf(stderr, "%s\n", strerror(errno));
				break;
			}
		} else {
			if (bg) {
				printf("[bg run] %d\n", pid);
			} else {
				waitpid(pid, &wstatus, WUNTRACED);
				rv = WEXITSTATUS(wstatus);
			}
		}

		if (count < HISTORY)
			count++;
		free(history[head]);
		history[head] = line;
		head = (head + 1) % HISTORY;
	}
	return 0;
}
