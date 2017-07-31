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
#define MAX_HISTORY	10

#define backward(head, n)	(((head) + MAX_HISTORY - (n)) % MAX_HISTORY)
#define forward(head, n)	(((head) + (n)) % MAX_HISTORY)

static char *history[MAX_HISTORY];
static size_t count, head;

void add_history(const char *line)
{
	if (count < MAX_HISTORY)
		count++;
	free(history[head]);
	history[head] = strdup(line);
	head = forward(head, 1);
}

void print_history(void)
{
	for (size_t i = count; i; i--)
		printf("%lu %s\n", i, history[backward(head, i)]);
}

void clear_history(void)
{
	for (size_t i = 0; i < MAX_HISTORY; i++) {
		free(history[i]);
		history[i] = NULL;
	}
}

int check_history(char *line)
{
	size_t index, id;

	if (line[0] != '!')
		return 0;

	if (line[1] == '!') {
		if (count == 0) {
			fprintf(stderr, "No commands in history.\n");
			return -1;
		}
		index = backward(head, 1);
	} else if (isdigit(line[1])) {
		sscanf(&line[1], "%lu", &id);
		if (id == 0 || id > count) {
			fprintf(stderr, "No such command in history\n");
			return -1;
		}
		index = backward(head, id);
	} else {
		fprintf(stderr, "Invalid syntax for history");
		return -1;
	}

	strcpy(line, history[index]);
	printf("%s\n", line);
	return 0;
}

int check_background(int argn, char **args)
{
	char *last_arg = args[argn - 1];
	size_t len = strlen(last_arg);

	if (last_arg[len - 1] == '&') {
		if (len > 1) {
			last_arg[len - 1] = '\0';
		} else {
			args[argn - 1] = NULL;
		}
		return 1;
	}
	return 0;
}

int parse_line(char *str, char **args, int lim)
{
	int n = 0;
	int quote = 0;
	int escape = 0;
	char *p, *q;

	args[n++] = str;

	for (p = q = str; *p; p++) {
		if (quote) {
			if (p[0] == '"') {
				quote = 0;
				continue;
			}

			if (p[0] == '\\' && (p[1] == '"' || p[1] == '\\'))
				p++;
			*q++ = *p;
		} else if (escape) {
			*q++ = *p;
			escape = 0;
		} else {
			switch (p[0]) {
			case '\\':
				escape = 1;
				break;
			case '"':
				quote = 1;
				break;
			case ' ':
			case '\t':
				if (q != str && q[-1] != '\0') {
					*q++ = '\0';
					args[n++] = q;
				}
				break;
			default:
				*q++ = *p;
			}
		}
	}

	*q = '\0';
	args[n] = NULL;

	return n;
}

int read_line(char *buf, size_t size)
{
	size_t len;
	int tmp;

	if (fgets(buf, size, stdin) == NULL)
		return EOF;

	if ((len = strlen(buf)) > 0) {
		if (buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
		} else {
			while ((tmp = getchar()) != EOF && tmp != '\n')
				;
		}
	}

	return 0;
}

int strempty(const char *s)
{
	for (const char *p = s; *p; p++)
		if (!isspace(*p))
			return 0;

	return 1;
}

int main(void)
{
	char line[MAX_LINE + 1];
	char *args[MAX_ARGN + 1];
	int bg;
	int argn;
	int wstatus;
	pid_t pid;

	for (;;) {
		while ((pid = waitpid(-1, &wstatus, WNOHANG)) > 0) {
			printf("[bg done] %d\n", pid);
		}

		printf("osh> ");
		fflush(stdout);

		if (read_line(line, MAX_LINE + 1) == EOF)
			break;

		if (strempty(line))
			continue;

		if (check_history(line) == -1)
			continue;

		add_history(line);

		if ((argn = parse_line(line, args, MAX_ARGN)) == 0)
			continue;

		bg = check_background(argn, args);

		if (strcmp(args[0], "exit") == 0) {
			break;
		} else if (strcmp(args[0], "history") == 0) {
			print_history();
		} else if ((pid = fork()) == 0) {
			if (execvp(args[0], args) == -1) {
				fprintf(stderr, "%s\n", strerror(errno));
				break;
			}
		} else {
			if (bg)
				printf("[bg start] %d\n", pid);
			else
				waitpid(pid, &wstatus, WUNTRACED);
		}
	}
	clear_history();
	return 0;
}
