#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  // char* argv[3];
  printf ("system call!\n");
  /* Based on what the stack pointer is point to, make an appropriate system call */
  switch(*(int *) f->esp) {
  	case SYS_HALT:
  		break;
  	case SYS_EXIT:
  		// exit(argv[0]);
  		break;
  	case SYS_EXEC:
  		break;
  	case SYS_WAIT:
  		break;
  	case SYS_CREATE:
  		break;
  	case SYS_REMOVE:
  		break;
  	case SYS_OPEN:
  		break;
  	case SYS_FILESIZE:
  		break;
  	case SYS_READ:
  		break;
  	case SYS_WRITE:
  		break;
  	case SYS_SEEK:
  		break;
  	case SYS_TELL:
  		break;
  	case SYS_CLOSE:
  		break;

  }
  
  thread_exit ();
}


// void halt(void) {

// }

void exit(int status) {
	/* Print exit message formatted as required,
	 with process name and exit status when terminated */
	printf("%s: (%d)\n", thread_current()->name, status);
	thread_exit();
}

// pid_t exec (const char *cmd_line) {

// }

// int wait (pid_t pid) {

// }

// bool create (const char *file, unsigned initial_size) {

// }

// bool remove (const char *file) {

// }

// int open (const char *file) {

// }

// int filesize (int fd) {

// }

// int read (int fd, void *buffer, unsigned size) {

// }

// int write (int fd, const void *buffer, unsigned size) {

// }

// void seek (int fd, unsigned position) {

// }

// unsigned tell (int fd) {

// }

// void close (int fd) {

// }
