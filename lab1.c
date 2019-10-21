/*
NAME: Keyu Ji
EMAIL: g.jikeyu@gmail.com
ID: 704966744
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

// how to collect signal and exit status

void change_FD(int i, int o, int e, int* file_num, int max_fn)
{
	close(0);
	dup(file_num[i]);
	close(1);
	dup(file_num[o]);
	
	close(2);
	dup(file_num[e]);
	close(file_num[i]);
	if (o != i)
		close(file_num[o]);
	if (e != o && e != i)
		close(file_num[e]);
	file_num[i] = -1;
	file_num[o] = -1;
	file_num[e] = -1;
	for (int i = 0; i <= max_fn; ++i)
		if (file_num[i] != -1)
			close(file_num[i]);
}

void error_action(int signum)
{
	fprintf(stderr, "%d caught\n", signum);
	exit(signum);
}

struct pid_info
{
	pid_t p_num;
	int b_index;
	int e_index;
};

int main(int argc, char* const argv[])
{
	int verbose = 0;
	int profile = 0;
	static struct option long_options[] = {
		{"append",		no_argument, 		0,  1},
		{"cloexec",		no_argument, 		0,  2},
		{"creat",		no_argument, 		0,  3},
		{"directory",	no_argument, 		0,  4},
		{"dsync",		no_argument, 		0,  5},
		{"excl",		no_argument, 		0,  6},
		{"nofollow",	no_argument, 		0,  7},
		{"nonblock",	no_argument, 		0,  8},
		{"rsync",		no_argument, 		0,  9},
		{"sync",		no_argument, 		0,  10},
		{"trunc",		no_argument, 		0,  11},
        {"rdonly",      required_argument, 	0,  'r'},
        {"rdwr",		required_argument,	0,	'b'},
        {"wronly", 		required_argument, 	0,  'w'},
        {"pipe",		no_argument,		0,	'p'},
        {"command",    	required_argument,	0,  'c'},
        {"wait",		no_argument,		0,	'm'},
        {"close",		required_argument, 	0,	's'},
        {"verbose",   	no_argument, 		0,  'v'},
        {"profile",		no_argument, 		0,	'f'},
        {"abort",		no_argument,		0,	'a'},
        {"catch",		required_argument,	0,	'h'},
        {"ignore",		required_argument,	0,	'i'},
        {"default",		required_argument,	0,	'd'},
        {"pause",		no_argument,		0,	'z'},
        {0,           	0,                 	0,  0}
	};
	int long_index = 0;
	int option;
	int nfn = 10;
	int *file_num = malloc(nfn * sizeof(int));
	if (file_num == NULL)
	{
		fprintf(stderr, "%s\n", "Unable to allocate memory. Hence cannot accept any files\n");
		free(file_num);
		exit(1); 
	}
	int npid = 10;
	struct pid_info* pid_table = malloc(npid * sizeof(struct pid_info));
	if (pid_table == NULL)
	{
		fprintf(stderr, "%s\n", "Unable to allocate memory. Hence cannot keep track of processes\n");
		free(pid_table);
		free(file_num);
		exit(1); 
	}
	int max_fn = -1;
	int max_p = -1;
	int xstat = 0;
	int oflags = 0;
	int xsignal = 0;

	while ((option = getopt_long(argc, argv, "r:w:c:v", long_options, &long_index)) && (option != -1))
	{
		switch(option)
		{
			case 1:
				oflags |= O_APPEND;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 2:
				oflags |= O_CLOEXEC;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 3:
				oflags |= O_CREAT;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 4:
				oflags |= O_DIRECTORY;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 5:
				oflags |= O_DSYNC;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 6:
				oflags |= O_EXCL;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 7:
				oflags |= O_NOFOLLOW;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 8:
				oflags |= O_NONBLOCK;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 9:
				oflags |= O_RSYNC;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 10:
				oflags |= O_SYNC;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 11:
				oflags |= O_TRUNC;
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				break;
			case 'r':
			{
				max_fn++;
				if (max_fn >= nfn)
				{
					nfn = nfn * 2;
					int *tmp = NULL;
					tmp = (int *)realloc(file_num, nfn*sizeof(int));
					if (tmp == NULL)
					{
						fprintf(stderr, "%s\n", "Unable to allocate more memory. Hence cannot accept more files\n");
						free(file_num);
						xstat = 1;
						break;
					}
					file_num = tmp;
				}

				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}

				oflags |= O_RDONLY;
				int fd = open(optarg, oflags, 0644);
				oflags = 0;
				if (fd == -1)
				{
					fprintf(stderr, "problematic file: %s\n", optarg);
					fprintf(stderr, strerror(errno));
					xstat = 1;
					file_num[max_fn] = -1;
				}
				else
					file_num[max_fn] = fd;
			}
				break;
			case 'b':
			{
				max_fn++;
				if (max_fn >= nfn)
				{
					nfn = nfn * 2;
					int *tmp = NULL;
					tmp = (int *)realloc(file_num, nfn*sizeof(int));
					if (tmp == NULL)
					{
						fprintf(stderr, "%s\n", "Unable to allocate more memory. hence cannot accept more files\n");
						free(file_num);
						xstat = 1;
						break;
					}
					file_num = tmp;
				}

				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}

				oflags |= O_RDWR;
				int fd = open(optarg, oflags, 0644);
				oflags = 0;
				if (fd == -1)
				{
					fprintf(stderr, "problematic file: %s\n", optarg);
					fprintf(stderr, strerror(errno));
					xstat = 1;
					file_num[max_fn] = -1;
				}
				else
					file_num[max_fn] = fd;
			}
				break;
			case 'w':
			{
				max_fn++;
				if (max_fn >= nfn)
				{
					nfn = nfn * 2;
					int *tmp = NULL;
					tmp = (int *)realloc(file_num, nfn*sizeof(int));
					if (tmp == NULL)
					{
						fprintf(stderr, "%s\n", "Unable to allocate more memory. hence cannot accept more files\n");
						free(file_num);
						xstat = 1;
						break;
					}
					file_num = tmp;
				}

				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}

				oflags |= O_WRONLY;
				int fd = open(optarg, oflags, 0644);
				oflags = 0;
				if (fd == -1)
				{
					fprintf(stderr, "problematic file: %s\n", optarg);
					fprintf(stderr, strerror(errno));
					xstat = 1;
					file_num[max_fn] = -1;
				}
				else
					file_num[max_fn] = fd;
			}
				break;
			case 'p':
			{
				max_fn += 2;
				if (max_fn >= nfn)
				{
					nfn = nfn * 2;
					int *tmp = NULL;
					tmp = (int *)realloc(file_num, nfn*sizeof(int));
					if (tmp == NULL)
					{
						fprintf(stderr, "%s\n", "Unable to allocate more memory. hence cannot accept more files\n");
						free(file_num);
						xstat = 1;
						break;
					}
					file_num = tmp;
				}

				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}

				int pipefd[2];
				if (pipe(pipefd) == -1)
				{
					perror("pipe");
					xstat = 1;
					file_num[max_fn-1] = -1;
					file_num[max_fn] = -1;
				}
				else
				{
					file_num[max_fn-1] = pipefd[0];
					file_num[max_fn] = pipefd[1];
				}
			}
				break;
			case 'c':
			{
				int si = atoi(strdup(argv[optind - 1]));
				int so = atoi(strdup(argv[optind]));
				int se = atoi(strdup(argv[optind + 1]));
				if (si > max_fn || so > max_fn || se > max_fn || si < 0 || so < 0 || se < 0)
				{
					fprintf(stderr, "%s\n", "--command: contains file descriptor that doesn't exist");
					xstat = 1;
					break;
				}
				if (file_num[si] == -1 || file_num[so] == -1 || file_num[se] == -1)
				{
					fprintf(stderr, "%s\n", "--command: contains closed or problematic file descriptor, unable to open");
					xstat = 1;
					break;
				}
				char *cmd;
				cmd = strdup(argv[optind + 2]);
				if (cmd[0] == '-' && cmd[1] == '-')
				{
					fprintf(stderr, "%s\n", "--command: missing cmd");
					xstat = 1;
					break;
				}
				int index = 2;
				while (1)
				{	
					if (((optind + index + 1) < argc) &&
						((strlen(argv[optind + index + 1]) == 1) ||
						((strlen(argv[optind + index + 1]) > 1) &&
						!(((argv[optind + index + 1][0] == '-') 
						&& (argv[optind + index + 1][1] == '-'))))))
						index++;
					else
						break;
				}
				char* args[index];
				args[0] = cmd;
				for (int i = 3; i <= index; ++i)
				{
					args[i - 2] = argv[optind + i];
				}
				args[index - 1] = NULL;
				if (verbose == 1)
				{
					for (int i = -2; i <= index; ++i)
					{
						fprintf(stdout, "%s ", argv[optind + i]);
					}
					fprintf(stdout, "\n");
					fflush(stdout);
				}
				pid_t pid = fork();
				if (pid < 0) 
			   	{ 
			        fprintf(stderr, "fork failed\n");
			        xstat = 1;
			    }
			    else if (pid == 0)
			    {
			    	change_FD(si, so, se, file_num, max_fn);
			    	execvp(cmd, args);
			    	exit(1);
			    }
			    else
			    {
			    	max_p++;
			    	if (max_p >= npid)
					{
						npid = npid * 2;
						struct pid_info *tmp = NULL;
						tmp = realloc(pid_table, npid*sizeof(struct pid_info));
						if (tmp == NULL)
						{
							fprintf(stderr, "%s\n", "Unable to allocate more memory. Hence cannot keep track of more processes\n");
							free(file_num);
							xstat = 1;
							break;
						}
					pid_table = tmp;
					}

					pid_table[max_p].p_num = pid;
					pid_table[max_p].b_index = optind+2;
					pid_table[max_p].e_index = optind+index;
			    }
			    if (index + optind + 1 > optind && index + optind + 1 <= argc)
			    	optind = optind + index + 1;
			}
				break;
			case 'm':
			{
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}

				while (1)
				{
					int wstatus;
					int ret;
					ret = wait(&wstatus);
					if (ret == -1)
						break;
					if (WIFSIGNALED(wstatus))
					{
						fprintf(stdout, "signal %d ", WTERMSIG(wstatus));
						if (WTERMSIG(wstatus) > xsignal)
							xsignal = WTERMSIG(wstatus);
					}
					else if (WIFEXITED(wstatus))
					{
						fprintf(stdout, "exit %d ", WEXITSTATUS(wstatus));
						if (WEXITSTATUS(wstatus) > xstat)
							xstat = WEXITSTATUS(wstatus);
					}
					for (int i = 0; i <= max_p; ++i)
					{
						if (ret == pid_table[i].p_num)
							for (int j = pid_table[i].b_index; j <= pid_table[i].e_index; ++j)
								fprintf(stdout, "%s ", argv[j]);
					}
					fprintf(stdout, "\n");
					if (profile == 1)
					{
						int who = RUSAGE_CHILDREN;
						struct rusage usage;
						int ret;
						ret = getrusage(who, &usage);
						if (ret != 0)
							perror("getrusage");
						printf("user CPU time: %ld+%ld, ", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
						printf("system CPU time: %ld+%ld, ", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
					}
				}
			}
				break;
			case 's':
			{
				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}

				int num = atoi(strdup(argv[optind - 1]));
				if (num > max_fn || num < 0)
				{
					fprintf(stderr, "%s\n", "--close: invalid file descriptor");
					xstat = 1;
				}
				else
				{
					if (file_num[num] == -1)
					{
						fprintf(stderr, "%s\n", "--close: closed or problematic file descriptor, unable to open");
						xstat = 1;
					}
					else
					{
						close(file_num[num]);
						file_num[num] = -1;
					}
				}
			}
				break;
			case 'v':
				verbose = 1;
				break;
			case 'f':
			{
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				profile = 1;
			}
				break;
			case 'a':
			{
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				char* null_point = NULL;
				*null_point = 'a';
			}
				break;
			case 'h':
				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}
				int signalnum = atoi(optarg);
				signal(signalnum, error_action);
				break;
			case 'i':
				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}
				signal(atoi(optarg), SIG_IGN);
				break;
			case 'd':
				if (verbose == 1)
				{
					fprintf(stdout, "%s %s\n", argv[optind-2], argv[optind-1]);
					fflush(stdout);
				}
				signal(atoi(optarg), SIG_DFL);
				break;
			case 'z':
				if (verbose == 1)
				{
					fprintf(stdout, "%s\n", argv[optind-1]);
					fflush(stdout);
				}
				pause();
				break;
			case '?':
				fprintf(stderr, "%s %s\n", "unrecognized argument:", optarg);
				xstat = 1;
				break;
		}
	}
	if (xsignal != 0)
		raise(xsignal);
	else
		exit(xstat);
}


/*
pid_t p = fork();
if (p == 0)
{
	execvp("date", argv);
	perror("date");
	_exit(126);
}
else if (p < 0)
{
	perror("fork");
}
else
{
	int status;
	if (waitpid(p, &status, 0)<0)
	{
		print("ouch!\n");
	}
	else if (WIFEXITED(status) && WEXITSTATUS(status))
	{
		print("date ok\n");
	}
	else
	{
		print("date failed\n");
	}
}
*/

/*
pid_t c=fork();
if (c<0) error();
if (c==0) {
	int fd[2];
	if (piep(fd)<0) error;
	pid_t a=fork()
	if (g<0) error();
	if (g==0) {
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]); // make sure that fd[1] != 1
		execvp("du", ...);
		error();
	}
	close(fd[1]);
	dup2(fd[0],0);
	close(fd[0]); // same here
	execvp("sort", ...);
	error();
}


	signal(SIGINT, interrupt)
char *fname = "foo";
outname = malloc(strlen("foo")+4);
sprintf(outname, "%s.gz", "foo");
fd = open(outname, ...);
[read, write]
	pthread_sigmask([block SIGINT]); // prevent if user kills it when it has unlinked the input file
	outname=NULL;
close(fd);
unlink(fname);

void interrupt(int sig) {
	if (outname != NULL)
		unlink(outname);
	_exit(120);
}
*/






/*
void handlesig(int sig) {
	printf("signal %d arrived\n", sig);
}

int main(int argc, char const *argv[])
{
	signal(SIGINT, handlesig);
	printf("Hello\n"); // handler can come in during print
}
*/