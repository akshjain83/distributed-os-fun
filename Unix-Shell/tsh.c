/*
 * tsh - A tiny shell program with job control
 *
 * @Author     : Akshay Jain
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "globals.h"
#include "jobs.h"
#include "helper-routines.h"



/* Global variables */

char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */



/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);



/*
 * main - The shell's main routine
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
void eval(char *cmdline)
{
  /* Parse command line */
  //
  // The 'argv' vector is filled in by the parseline
  // routine below. It provides the arguments needed
  // for the execve() routine, which you'll need to
  // use below to launch a process.
  //
  char *argv[MAXARGS];

  //
  // The 'bg' variable is TRUE if the job should run
  // in background mode or FALSE if it should run in FG
  //
  int bg = parseline(cmdline, argv);
  if (argv[0] == NULL)
    return;   /* ignore empty lines */
  
  if((builtin_cmd(argv)) == 0)
  {
    pid_t pid;
    sigset_t mask; 
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    if((pid = fork()) == 0)
    {
      setpgid(0,0);
      sigprocmask(SIG_UNBLOCK, &mask, NULL);
      //if((execve(argv[0], argv, NULL)) < 0)
      if((execvp(argv[0], argv)) < 0)
      {
        printf("%s: Command not found\n", argv[0]);
        exit(0);
      }
    }
    if(bg)
    {
      addjob(jobs, pid, BG, cmdline);
      sigprocmask(SIG_UNBLOCK, &mask, NULL);
      //int jid = pid2jid(pid);
      printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
    }      
    else
    { 
      addjob(jobs, pid, FG, cmdline);
      sigprocmask(SIG_UNBLOCK, &mask, NULL);
      waitfg(pid);
    }
  }
  return;
}


/////////////////////////////////////////////////////////////////////////////
//
// builtin_cmd - If the user has typed a built-in command then execute
// it immediately. The command name would be in argv[0] and
// is a C string. We've cast this to a C++ string type to simplify
// string comparisons; however, the do_bgfg routine will need
// to use the argv array as well to look for a job number.
//
int builtin_cmd(char **argv)
{
  if((strcmp(argv[0], "quit"))==0)
    exit(0);
  else if(((strcmp(argv[0], "fg"))==0) || ((strcmp(argv[0], "bg"))==0))
  { 
     do_bgfg(argv);
     return 1;
  }
  else if((strcmp(argv[0], "jobs"))==0)
  {
    listjobs(jobs);
    return 1;
  }
  return 0;     /* not a builtin command */
}

/////////////////////////////////////////////////////////////////////////////
//
// do_bgfg - Execute the builtin bg and fg commands
//
void do_bgfg(char **argv)
{
  struct job_t *jobp=NULL;

  /* Ignore command if no argument */
  if (argv[1] == NULL) {
    printf("%s command requires PID or %%jobid argument\n", argv[0]);
    return;
  }

  /* Parse the required PID or %JID arg */
  if (isdigit(argv[1][0])) {
    pid_t pid = atoi(argv[1]);
    if (!(jobp = getjobpid(jobs, pid))) {
      printf("(%d): No such process\n", pid);
      return;
    }
  }
  else if (argv[1][0] == '%') {
    int jid = atoi(&argv[1][1]);
    if (!(jobp = getjobjid(jobs, jid))) {
      printf("%s: No such job\n", argv[1]);
      return;
    }
  }
  else {
    printf("%s: argument must be a PID or %%jobid\n", argv[0]);
    return;
  }

  //
  // You need to complete rest. At this point,
  // the variable 'jobp' is the job pointer
  // for the job ID specified as an argument.
  //
  // Your actions will depend on the specified command
  // so we've converted argv[0] to a string (cmd) for
  // your benefit.
  //
  kill(-(jobp->pid), SIGCONT);
  if((strcmp(argv[0], "bg"))==0)
  {
    jobp->state = BG;
    printf("[%d] (%d) %s", jobp->jid, jobp->pid, jobp->cmdline);
  }
  else
  {
    if((strcmp(argv[0], "fg"))==0)
    {
      jobp->state = FG;
      waitfg(jobp->pid);
    }
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// waitfg - Block until process pid is no longer the foreground process
//
void waitfg(pid_t pid)
{
  while((fgpid(jobs)) == pid)
    sleep(1);
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// Signal handlers
//


/////////////////////////////////////////////////////////////////////////////
//
// sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
//     a child job terminates (becomes a zombie), or stops because it
//     received a SIGSTOP or SIGTSTP signal. The handler reaps all
//     available zombie children, but doesn't wait for any other
//     currently running children to terminate.
//
void sigchld_handler(int sig)
{
  int status;
  pid_t pid; 
  while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
  {
    struct job_t *pidjob = getjobpid(jobs, pid);
    int jid = pid2jid(pid);
    if(WIFSTOPPED(status))
    {
      pidjob->state = ST;
      printf("Job [%d] (%d) stopped by signal %d\n", jid, pid, WSTOPSIG(status));
    }
    else if(WIFEXITED(status))
      deletejob(jobs, pid);
    else if(WIFSIGNALED(status))
    {
      deletejob(jobs, pid);
      printf("Job [%d] (%d) terminated by signal %d\n", jid, pid, WTERMSIG(status));      
    }
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigint_handler - The kernel sends a SIGINT to the shell whenever the
//    user types ctrl-c at the keyboard.  Catch it and send it along
//    to the foreground job.
//
void sigint_handler(int sig)
{
  pid_t fj_pid;
  if((fj_pid = fgpid(jobs)) > 0)
    kill(-fj_pid, sig);
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
//     the user types ctrl-z at the keyboard. Catch it and suspend the
//     foreground job by sending it a SIGTSTP.
//
void sigtstp_handler(int sig)
{
  pid_t fj_pid;
  if((fj_pid = fgpid(jobs)) > 0)
    kill(-fj_pid, sig);
  return;
}

/*********************
 * End signal handlers
 *********************/
