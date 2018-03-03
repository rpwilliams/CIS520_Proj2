#include <stdio.h>
#include <syscall.h>

int
main (int argc, char **argv)
{
  int i;

  for (i = 0; i < 20; i++)
    printf ("%s ", argv[i]);
  printf ("\n");

  return EXIT_SUCCESS;
}
