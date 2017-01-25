
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "mystring.h"
#include "profile.h"

#define MAX_PROFILES 10000

struct profile_table current_profile_table;
struct profile_view current_profile_view;

typedef void(*command_t)(int, char**);
struct named_command {
  char name[10];
  command_t command;
};
typedef enum {
  CMD_INVALID,
  CMD_CHECK, CMD_PRINT,
  CMD_READ, CMD_WRITE,
  CMD_SORT, CMD_FIND,
  CMD_QUIT, CMD_END
} command_index;
struct named_command command_table[CMD_END];
void initialize_command_table(void);

void cmd_invalid(int argc, char* argv[]) {
  fprintf(stdout, "\"%s\" command not found\n", argv[0]);
}

void cmd_check(int argc, char* argv[]) {
  fprintf(stdout, "%d profiles in view\n",
          current_profile_view.n_entries);
}

void cmd_print(int argc, char* argv[]) {
  int n = atoi(argv[1]);
  struct profile_view shown;
  new_profile_view(&shown, current_profile_view.n_entries);
  take(&shown, &current_profile_view, n);
  write_profiles_fancy(stdout, &shown);
  delete_profile_view(&shown);
}

void cmd_read(int argc, char* argv[]) {
  FILE *fp = fopen(argv[1], "r");
  delete_profile_table(&current_profile_table);
  new_profile_table(&current_profile_table, MAX_PROFILES);
  read_profiles_csv(&current_profile_table, fp);
  create_view_from(&current_profile_view, &current_profile_table);
}

void cmd_write(int argc, char* argv[]) {
  FILE *fp = fopen(argv[1], "w");
  write_profiles_csv(fp, &current_profile_view);
}

void cmd_sort(int argc, char* argv[]) {
  int which = atoi(argv[1]);
  struct profile_view sorted;
  new_profile_view(&sorted, current_profile_view.n_entries);
  sorted_by(&sorted, &current_profile_view, which);
  copy_view(&current_profile_view, &sorted);
  delete_profile_view(&sorted);
}

void cmd_find(int argc, char* argv[]) {
  char word[64] = "\0";
  struct profile_view found;
  // TODO inefficient algorithm
  for(int i = 1; i < argc; i++) {
    strcat(word, argv[i]);
    if(i < argc-1)
      strcat(word, " ");
  }
  new_profile_view(&found, current_profile_view.n_entries);
  search_word_in_all(&found, &current_profile_view, word);
  write_profiles_fancy(stdout, &found);
  delete_profile_view(&found);
}

void cmd_quit(int argc, char* argv[]) {
  delete_profile_view(&current_profile_view);
  delete_profile_table(&current_profile_table);
  printf("Bye.\n");
  exit(0);
}

void initialize_command_table(void) {
  command_table[CMD_INVALID] = (struct named_command){ "\0", cmd_invalid };
  command_table[CMD_CHECK] = (struct named_command){ "%C", cmd_check };
  command_table[CMD_PRINT] = (struct named_command){ "%P", cmd_print };
  command_table[CMD_READ] = (struct named_command){ "%R", cmd_read };
  command_table[CMD_WRITE] = (struct named_command){ "%W", cmd_write };
  command_table[CMD_SORT] = (struct named_command){ "%S", cmd_sort };
  command_table[CMD_FIND] = (struct named_command){ "%F", cmd_find };
  command_table[CMD_QUIT] = (struct named_command){ "%Q", cmd_quit };
}

void exec_command(char* line) {
  char _line[strlen(line) + 1];
  memcpy(_line, line, sizeof(_line));
  char* argv[10];
  int i, argc = split(_line, argv, ' ', 10);
  command_index index = CMD_INVALID;
  for(i = 0; i < CMD_END; i++) {
    if(strcmp(command_table[i].name, argv[0]) == 0) {
      index = i;
      break;
    }
  }
  command_table[index].command(argc, argv);
}

int main(int argc, char* argv[]) {
  initialize_command_table();
  new_profile_table(&current_profile_table, MAX_PROFILES);
  new_profile_view(&current_profile_view, MAX_PROFILES);

  exec_command("hogehoge");
  exec_command("%R sample.csv");
  exec_command("%C");
  printf("Print first 3 profiles\n");
  exec_command("%P 3");
  printf("Print last 3 profiles\n");
  exec_command("%P -3");
  printf("Find profile that has \"The Bridge\"\n");
  exec_command("%F The Bridge");
  printf("Sort by name (alphabetical)\n");
  exec_command("%S 2");
  printf("Then, print first 3 profiles\n");
  exec_command("%P 3");
  exec_command("%Q");

  // this command would not run
  write_profiles_fancy(stdout, &current_profile_view);
}
