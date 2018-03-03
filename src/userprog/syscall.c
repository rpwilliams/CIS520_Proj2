#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);
void check_valid_ptr (const void *ptr);
void check_valid_buffer(void *buffer, unsigned size);
void get_arguments(struct intr_frame *f, int *args, int n);

/* A lock to ensure multiple processes can't edit a file at the same time */
struct lock* file_lock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{  
  printf ("system call!\n");
  int args[3];
  /* Ensure user provided pointer is valid/safe */
  check_valid_ptr((const void *) f->esp);
  
  /* Based on what the system call number the
   stack pointer is point to, make an appropriate system call */
  switch(*(int *) f->esp) {
  	/* Halt the operating system. */
  	case SYS_HALT:
  		break;
  	/* Terminate this process. */
  	case SYS_EXIT:
  		// printf("in exit");
  		get_arguments(f, &args[0], 1);
  		exit(args[0]);
  		break;
  	/* Start another process. */
  	case SYS_EXEC:
  		break;
  	/* Wait for a child process to die. */
  	case SYS_WAIT:
      get_arguments(f, &args[0], 1);
      f->eax = wait((pid_t) args[0]);
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
  		/* Get the 3 arguments for write (filename, buffer, and size) off the stack */ 
  		get_arguments(f, &args[0], 3);
  		/* Ensure the buffer is valid */
  		check_valid_buffer((void*) args[1], args[2]);

  		/* Transform buffer from user virtual address to kernel virtual address */
  		args[1] = (int) pagedir_get_page(thread_current()->pagedir, (const void*) args[1]);		

  		f->eax = write(args[0], (const void *) args[1], (unsigned) args[2]);
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
  	default:
  		// printf("no man's land");
  		exit(-1);
  		break;

  }
 
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
  /* Set the exit status of the current thread */
  thread_current()->exit_status = status;
	thread_exit();
}

// pid_t exec (const char *cmd_line) {

// }

int wait (pid_t pid) {
  return process_wait(pid);
}

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

/* Writes to a file. Returns size of file we wrote. */
int write (int fd, const void *buffer, unsigned size) {
	/* If the file descriptor is standard output,
	 we write the buffer for the entire length of size */
	// lock_acquire(&file_lock);
	if(fd == STDOUT_FILENO) {
		/* Print the buffer to the console */
    puts("We writin");
		putbuf(buffer, size);
		return size;
	}
	else if(fd == STDIN_FILENO) {
		return 0; // Error
	}
	else {
		// lock_acquire(&file_lock);
		// TODO: All other cases
		return 0;

	}
	// lock_release(&file_lock);
	
}

// void seek (int fd, unsigned position) {

// }

// unsigned tell (int fd) {

// }

// void close (int fd) {

// }

/* Ensures the pointer is valid */
void check_valid_ptr(const void *ptr) {
	/* If a pointer is null, is not a user virtual address,
	 or points to unmapped memory, it is invalid*/
	if(ptr == NULL || !is_user_vaddr(ptr)) {
		/* Kill process and free resources */
		exit(-1);
	}
}

/* Ensures the buffer is valid */
void check_valid_buffer(void *buffer, unsigned size) {
	/* If a pointer is null, is not a user virtual address,
	 or points to unmapped memory, it is invalid*/
	char *ptr = (char*) buffer;
	for(unsigned i = 0; i < size; i++) {
		check_valid_ptr((const void*) ptr);
		ptr++;
	}
	
}

/* Gets n arguments off of the stack */
void get_arguments(struct intr_frame *f, int *args, int n) {
	int *arg;
	for(int i = 0; i < n; i++) {
		arg = (int*) f->esp + i + 1;
		check_valid_ptr((const void*) arg);
		args[i] = *arg;
    // printf(i, *arg, "ARGS[%d]: %d");
	}
}

