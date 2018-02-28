#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
void check_valid_ptr (const void *ptr);
void check_valid_buffer(void *buffer, unsigned size);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{  
  // char* argv[3];
  /* Ensure user provided pointer is valid/safe */
  check_valid_ptr((const void *) f->esp);
  printf ("system call!\n");
  /* Based on what the system call number the
   stack pointer is point to, make an appropriate system call */
  switch(*(int *) f->esp) {
  	/* Halt the operating system. */
  	case SYS_HALT:
  		break;
  	/* Terminate this process. */
  	case SYS_EXIT:
  		// exit(argv[0]);
  		break;
  	/* Start another process. */
  	case SYS_EXEC:
  		break;
  	/* Wait for a child process to die. */
  	case SYS_WAIT:
  		break;
  	/* Create a file. */
  	case SYS_CREATE:
  		break;
  	/* Delete a file. */
  	case SYS_REMOVE:
  		break;
  	/* Open a file. */
  	case SYS_OPEN:
  		break;
  	/* Obtain a file's size. */
  	case SYS_FILESIZE:
  		break;
  	/* Read from a file. */
  	case SYS_READ:
  		break;
  	/* Write to a file. */
  	case SYS_WRITE:
  		break;
  	/* Change position in a file. */
  	case SYS_SEEK:
  		break;
  	/* Report current position in a file. */
  	case SYS_TELL:
  		break;
  	/* Close a file. */
  	case SYS_CLOSE:
  		break;

  }
  
  thread_exit ();
}


// void halt(void) {

// }


/* Terminates the current user program, returning status to the kernel. 
   If the process's parent waits for it, this is the status that
   will be returned. A status of 0 indicates success and nonzero 
   values indicate errors. */
void exit(int status) {
	/* Print process name and exit status */
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

void check_valid_ptr(const void *ptr) {
	/* If a pointer is null, is not a user virtual address,
	 or points to unmapped memory, it is invalid*/
	if(ptr == NULL || !is_user_vaddr(ptr)) {
		/* Kill process and free resources */
		exit(-1);
	}
}

void check_valid_buffer(void *buffer, unsigned size) {
	/* If a pointer is null, is not a user virtual address,
	 or points to unmapped memory, it is invalid*/
	char *ptr = (char*) buffer;
	for(unsigned i = 0; i < size; i++) {
		check_valid_ptr((const void*) ptr);
		ptr++;
	}
	
}
