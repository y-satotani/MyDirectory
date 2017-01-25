#pragma once

#include "profile.h"

#define MAX_PROFILES 10000

typedef int(*command)(int, char**, int, struct buffer*);

typedef enum {
  CMD_INVALID,
  CMD_CHECK, CMD_PRINT,
  CMD_READ, CMD_READ_L,
  CMD_WRITE, CMD_WRITE_L,
  CMD_SORT, CMD_FIND,
  CMD_QUIT, CMD_END
} command_index;

char* command_name_table[] = {
  "%",
  "%C", "%P",
  "%R", "%RL",
  "%W", "%WL",
  "%S", "%F",
  "%Q"
};

command_index command_index_of(char* name) {
  int i;
  command_index index = CMD_INVALID;
  for(i = 0; i < CMD_END; i++)
    if(strcmp(name, command_name_table[i]) == 0) {
      index = i;
      break;
    }
  return index;
}

int cmd_invalid(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_check(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_print(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_readn(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_readl(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_writen(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_writel(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_sort(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_find(int argc, char* argv[], int sfd, struct buffer* buf);
int cmd_quit(int argc, char* argv[], int sfd, struct buffer* buf);

command command_table[] = {
  cmd_invalid,
  cmd_check, cmd_print,
  cmd_readn, cmd_readl,
  cmd_writen, cmd_writel,
  cmd_sort, cmd_find,
  cmd_quit
};
