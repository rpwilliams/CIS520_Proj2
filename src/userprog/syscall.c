#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/filesys.h"


static void syscall_handler (struct intr_frame *);
void check_valid_ptr (const void *ptr);
void check_valid_buffer(void *buffer, unsigned size);
void get_arguments(struct intr_frame *f, int *args, int n);
int get_kernel_ptr(const void *user_ptr);
struct file* get_file_from_list(int fd);
void remove_file_from_list(int fd);

/* The bottom of the user virtual address space */
#define MIN_VIRTUAL_ADDR ((void *) 0x08048000)

/* A lock to ensure multiple processes can't edit a file at the same time */
struct lock file_lock;

/* Contains mapping of files to their file descriptors 
   to be added to list of file descriptors */
struct file_entry {
  struct file *file;
  int fd;
  struct list_elem file_elem;
};

void
syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{  
  /* The stack arguments -> we will only ever need up to 3 */
  int args[3];
  /* Ensure user provided pointer is valid/safe */
  check_valid_ptr((const void *) f->esp);
  
  /* Based on what the system call number the
   stack pointer is point to, make an appropriate system call */
  switch(*(int *) f->esp) {
  	/* Halt the operating system. */
  	case SYS_HALT:
      halt();
  		break;
  	/* Terminate this process. */
  	case SYS_EXIT:
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
      get_arguments(f, &args[0], 2);
      /* Transforms file from user virtual address to kernel virtual address */
      args[0] = get_kernel_ptr((const void*) args[0]);
      f->eax = create((const char*) args[0], (unsigned) args[1]);
  		break;
  	/* Delete a file. */
  	case SYS_REMOVE:
      get_arguments(f, &args[0], 1);
      /* Transforms file from user virtual address to kernel virtual address */
      args[0] = get_kernel_ptr((const void*) args[0]);
      f->eax = remove((const char*) args[0]);
  		break;
  	/* Open a file. */
  	case SYS_OPEN:
      get_arguments(f, &args[0], 1);
      args[0] = get_kernel_ptr((const void*) args[0]);
      f->eax = open((const char*) args[0]);
  		break;
  	/* Obtain a file's size. */
  	case SYS_FILESIZE:
  		break;
  	/* Read from a file. */
  	case SYS_READ:
      /* Get the 3 arguments for read (filename, buffer, and size) off the stack */ 
      get_arguments(f, &args[0], 3);
      /* Ensure the buffer is valid */
      check_valid_buffer((void*) args[1], (unsigned) args[2]);
      /* Transform buffer from user virtual address to kernel virtual address */
      args[1] = get_kernel_ptr((const void*) args[1]);   
      f->eax = read(args[0], (void *) args[1], (unsigned) args[2]);
  		break;
  	/* Write to a file. */
  	case SYS_WRITE:
  		/* Get the 3 arguments for write (filename, buffer, and size) off the stack */ 
  		get_arguments(f, &args[0], 3);
  		/* Ensure the buffer is valid */
  		check_valid_buffer((void*) args[1], (unsigned) args[2]);
  		/* Transform buffer from user virtual address to kernel virtual address */
  		args[1] = get_kernel_ptr((const void*) args[1]);		
  		f->eax = write(args[0], (const void *) args[1], (unsigned) args[2]);
  		break;
  	/* Change position in a file. */
  	case SYS_SEEK:
      get_arguments(f, &args[0], 2);
      /* Returns result of seek in EAX register */
      seek(args[0], (unsigned) args[1]);
  		break;
  	/* Report current position in a file. */
  	case SYS_TELL:
      get_arguments(f, &args[0], 1);
      f->eax = tell(args[0]);
  		break;
  	/* Close a file. */
  	case SYS_CLOSE:
      get_arguments(f, &args[0], 1);
      close(args[0]);
  		break;
  	default:
  		exit(-1);
  		break;

  }
 
}

/* Terminates pintos -- rarely used */
void halt(void) {
  shutdown_power_off(); 
}


/* Terminates the current user program, returning status to the kernel. 
   If the process's parent waits for it, this is the status that
   will be returned. A status of 0 indicates success and nonzero 
   values indicate errors. */
void exit(int status) {
	/* Print process name and exit status */
	printf("%s: exit(%d)\n", thread_current()->name, status);
  /* Set the exit status of the current thread */
  thread_current()->exit_status = status;
	thread_exit();
}

// pid_t exec (const char *cmd_line) {

// }

int wait (pid_t pid) {
  return process_wait(pid);
}

/* Create a new file, and return true if successful */
bool create (const char *file, unsigned initial_size) {
  /* Use a lock to avoid race conditions */
  lock_acquire(&file_lock);
  bool success = filesys_create(file, initial_size);
  lock_release(&file_lock);
  return success;
}

/* Deletes the file called file, returns true if successful */
bool remove (const char *file) {
  /* Use a lock to avoid race conditions */
  lock_acquire(&file_lock);
  bool success = filesys_remove(file);
  lock_release(&file_lock);
  return success;
}

/* Opens the file passed in. Returns a file descripter of -1 if it could not be opened. */
int open (const char *file) {
  lock_acquire(&file_lock);
  struct file* open_file = filesys_open(file);

  /* File could not be opened */
  if(open_file == NULL) {
    lock_release(&file_lock);
    return -1;
  }

  /* Create a new file_entry struct */
  struct file_entry *file_entry = malloc(sizeof(struct file_entry));
  file_entry->file = open_file;
  int fd = thread_current()->fd;
  file_entry->fd = fd;
  thread_current()->fd++;
  /* Add the file_entry to the list of file entries */
  list_push_back(&thread_current()->fd_list, &file_entry->file_elem);

  lock_release(&file_lock);
  return fd;
}

// int filesize (int fd) {

// }

/* Reads size bytes from the file open as fd into buffer.
   Returns the number of bytes actually read (0 at end of file)
   or -1 if the file could not be read */
int read (int fd, void *buffer, unsigned size) {
  
  lock_acquire(&file_lock);
  if(fd == STDIN_FILENO) {
    uint8_t* local_buffer = (uint8_t*) buffer;
    for(unsigned i = 0; i < size; i++) {
      local_buffer[i] = input_getc();
    }
    lock_release(&file_lock);
    return size;
  }
  
  lock_release(&file_lock);
  return 0;
  
}

/* Writes to a file. Returns size of file we wrote. */
int write (int fd, const void *buffer, unsigned size) {
  lock_acquire(&file_lock);
	/* If the file descriptor is standard output,
	 we write the ENTIRE buffer the console */
	if(fd == STDOUT_FILENO) {
		/* Prints the entire buffer to the console */
		putbuf(buffer, size);
    lock_release(&file_lock);
		return size;
	}
	

  lock_release(&file_lock);
	return 0;
}


/* Changes the next byte to be read or written in open file fd to position,
   expressed in bytes from the beginning of the file */
void seek (int fd, unsigned position) {
  lock_acquire(&file_lock);
  /* Gets a file with a matching file descriptor */
  struct file *f = get_file_from_list(fd);

  if(f == NULL) {
    lock_release(&file_lock);
    return;
  }
  file_seek(f, position);
  lock_release(&file_lock);
}

/* Returns the position of the next byte to be read 
   or written in the open file fd */
unsigned tell (int fd) {
  lock_acquire(&file_lock);
  /* Gets a file with a matching file descriptor */
  struct file *f = get_file_from_list(fd);

  if(f == NULL) {
    lock_release(&file_lock);
    return -1;
  }
  unsigned pos = (unsigned) file_tell(f);
  lock_release(&file_lock);
  return pos;
}

/* Closes the passed file descriptor */
void close (int fd) {
  lock_acquire(&file_lock);
  remove_file_from_list(fd);
  lock_release(&file_lock);
}

/* Ensures the pointer is valid */
void check_valid_ptr(const void *ptr) {
	/* If a pointer is null, is not a user virtual address,
	 or points to unmapped memory, it is invalid*/
	if(ptr == NULL || !is_user_vaddr(ptr) || ptr < MIN_VIRTUAL_ADDR) {
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

/* Converts the user pointer to a kernel pointer and returns it */
int get_kernel_ptr(const void *user_ptr) {
  /* Ensure the user pointer is valid */
  check_valid_ptr(user_ptr);
  /* Converts the user pointer to a kernel pointer */
  void *kernel_ptr = pagedir_get_page(thread_current()->pagedir, user_ptr);
  /* Ensure the kernel pointer is not null */
  if(kernel_ptr == NULL) {
    exit(-1);
  }
  return (int) kernel_ptr;
}

/* Gets n arguments off of the stack */
void get_arguments(struct intr_frame *f, int *args, int n) {
	int *arg;
	for(int i = 0; i < n; i++) {
		arg = (int*) f->esp + i + 1;
		check_valid_ptr((const void*) arg);
		args[i] = *arg;
	}
}

/* Gets a file from the list of file_entrys based off the file descriptor */
struct file* get_file_from_list(int fd) {
  struct list_elem *e;

  for(e  = list_front(&thread_current()->fd_list); e != NULL; e = e->next) {
    struct file_entry *file_entry = list_entry(e, struct file_entry, file_elem);
    if(fd == file_entry->fd) {
      return file_entry->file;
    }
  }
  return NULL;
}

/* Removes and closes a file from the list of file_entrys based off the file descriptor */
void remove_file_from_list(int fd) {
  /* Ensure the list is not empty */
  if(list_empty(&thread_current()->fd_list)) {
    return;
  }

  struct list_elem *e;

  for(e  = list_front(&thread_current()->fd_list); e != NULL; e = e->next) {
    struct file_entry *file_entry = list_entry(e, struct file_entry, file_elem);
    if(fd == file_entry->fd) {
      file_close(file_entry->file);
      list_remove(&file_entry->file_elem);
      return; // We are only removing one file_entry, so we're done
    }
  }
}
