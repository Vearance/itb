#include "header/usermode/commands/exec.h"
#include "header/usermode/user-shell.h"

int8_t exec_command(char *filename, uint32_t parent_inode)
{
  uint8_t retcode;
  syscall(SYS_CREATE_PROCESS, (uint32_t)filename, parent_inode, (uint32_t)&retcode);
  return (int8_t)retcode;
}