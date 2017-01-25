
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "mystring.h"
#include "profile.h"

struct profile_table current_profile_table;
struct profile_view current_profile_view;

int cmd_invalid(int argc, char* argv[], int sfd, struct buffer* buf) {
  printf("server recieved invalid command\n");

  reset_buffer(buf);
  serialize_short(buf, CMD_INVALID);
  serialize_short(buf, PS_SUCCESS);

  return PS_SUCCESS;
}

int cmd_check(int argc, char* argv[], int sfd, struct buffer* buf) {
  printf("check command: %d profiles in current table\n",
         current_profile_view.n_entries);

  reset_buffer(buf);
  serialize_short(buf, CMD_CHECK);
  serialize_short(buf, PS_SUCCESS);
  serialize_long(buf, current_profile_view.n_entries);

  return PS_SUCCESS;
}

int cmd_print(int argc, char* argv[], int sfd, struct buffer* buf) {
  int n_entries, status;
  struct profile_view shown;
  new_profile_view(&shown, current_profile_view.n_entries);

  n_entries = deserialize_long(buf);
  status = take(&shown, &current_profile_view, n_entries);
  printf("print command: argument=%d status=%d\n", n_entries, status);

  reset_buffer(buf);
  serialize_short(buf, CMD_PRINT);
  serialize_short(buf, status);
  serialize_profile_view(buf, &shown);

  delete_profile_view(&shown);
  return status;
}

int cmd_readn(int argc, char* argv[], int sfd, struct buffer* buf) {
  int n_entries, status = PS_SUCCESS;
  char* filename;
  FILE *fp;

  // TODO error handling
  deserialize_str(&filename, buf);
  fp = fopen(filename, "r");
  delete_profile_table(&current_profile_table);
  new_profile_table(&current_profile_table, MAX_PROFILES);
  n_entries = read_profiles_csv(&current_profile_table, fp);
  create_view_from(&current_profile_view, &current_profile_table);
  printf("read net command: read %d profile(s), status=%d\n", n_entries, status);

  reset_buffer(buf);
  serialize_short(buf, CMD_READ);
  serialize_short(buf, status);
  serialize_long(buf, n_entries);

  free(filename);
  return status;
}

int cmd_readl(int argc, char* argv[], int sfd, struct buffer* buf) {
  int status = PS_SUCCESS;

  delete_profile_view(&current_profile_view);
  delete_profile_table(&current_profile_table);
  new_profile_table(&current_profile_table, MAX_PROFILES);
  new_profile_view(&current_profile_view, MAX_PROFILES);

  deserialize_profile_table(&current_profile_table, buf);
  create_view_from(&current_profile_view, &current_profile_table);
  printf("read local command: read %d profile(s)\n",
	 current_profile_table.n_entries);

  reset_buffer(buf);
  serialize_short(buf, CMD_READ_L);
  serialize_short(buf, status);
  serialize_long(buf, current_profile_table.n_entries);

  return status;
}

int cmd_writen(int argc, char* argv[], int sfd, struct buffer* buf) {
  int n_entries, status = PS_SUCCESS;
  char* filename;
  FILE *fp;

  // TODO error handling
  deserialize_str(&filename, buf);
  fp = fopen(filename, "w");
  n_entries = write_profiles_csv(fp, &current_profile_view);
  fclose(fp);
  //printf("write net command: write %d profile(s), status=%d\n", n_entries, status);

  reset_buffer(buf);
  serialize_short(buf, CMD_WRITE);
  serialize_short(buf, status);
  serialize_long(buf, n_entries);

  free(filename);
  return status;
}

int cmd_writel(int argc, char* argv[], int sfd, struct buffer* buf) {
  reset_buffer(buf);
  serialize_short(buf, CMD_WRITE_L);
  serialize_short(buf, PS_SUCCESS);
  serialize_profile_view(buf, &current_profile_view);

  printf("write local command: write %d profile(s)\n",
         current_profile_view.n_entries);

  return PS_SUCCESS;
}

int cmd_sort(int argc, char* argv[], int sfd, struct buffer* buf) {
  uint16_t which, status;
  struct profile_view sorted;
  which = deserialize_short(buf);
  new_profile_view(&sorted, current_profile_view.n_entries);

  status = sorted_by(&sorted, &current_profile_view, which);
  copy_view(&current_profile_view, &sorted);

  reset_buffer(buf);
  serialize_short(buf, CMD_SORT);
  serialize_short(buf, status);

  delete_profile_view(&sorted);
  return status;
}

int cmd_find(int argc, char* argv[], int sfd, struct buffer* buf) {
  int status;
  char* word;
  struct profile_view found;
  deserialize_str(&word, buf);
  new_profile_view(&found, current_profile_view.n_entries);

  status = search_word_in_all(&found, &current_profile_view, word);

  reset_buffer(buf);
  serialize_short(buf, CMD_FIND);
  serialize_short(buf, status);
  serialize_profile_view(buf, &found);

  delete_profile_view(&found);
  return status;
}

int cmd_quit(int argc, char* argv[], int sfd, struct buffer* buf) {
  reset_buffer(buf);
  serialize_short(buf, CMD_QUIT);
  serialize_short(buf, PS_SUCCESS);

  return PS_SUCCESS;
}

int main(int argc, char* argv[]) {
  int host_sfd, client_sfd;
  struct sockaddr_in my_addr, peer_addr;
  socklen_t peer_addr_size;
  int port_no;

  if(argc < 2) {
    fprintf(stderr, "Error: no port number provided\n");
    return -1;
  }
  port_no = atoi(argv[1]);

  host_sfd = socket(AF_INET, SOCK_STREAM, 0);
  if(host_sfd < 0) {
    fprintf(stderr, "Error: cannot open socket\n");
    return -1;
  }

  memset(&my_addr, 0, sizeof(struct sockaddr_in));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  my_addr.sin_port = htons(port_no);
  if(bind(host_sfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
    fprintf(stderr, "Erorr: failed to bind socket\n");
    return -1;
  }


  if(listen(host_sfd, 5) < 0) {
    fprintf(stderr, "Error: cannot listen\n");
    return -1;
  }

  while(1) {
    peer_addr_size = sizeof(struct sockaddr_in);
    client_sfd = accept(host_sfd,
                        (struct sockaddr*)&peer_addr, &peer_addr_size);
    if(client_sfd < 0) {
      fprintf(stderr, "Error: cannot accept\n");
      continue;
    }

    struct buffer buf;
    new_profile_table(&current_profile_table, MAX_PROFILES);
    new_profile_view(&current_profile_view, MAX_PROFILES);
    new_buffer(&buf, 1024);

    while(1) {
      // recieve request
      read_buffer(client_sfd, &buf, 1024);

      // exec. requested command
      uint16_t index = CMD_INVALID;
      //int status;
      index = deserialize_short(&buf);
      printf("command no. %d\n", index);
      if(index < 0 || index >= CMD_END) index = CMD_INVALID;
      command_table[index](0, NULL, client_sfd, &buf);
      //status = command_table[index](0, NULL, client_sfd, &buf);
      //printf("status: %d\n", status);

      // send response
      write_buffer(client_sfd, &buf, 1024);
      reset_buffer(&buf);

      if(index == CMD_QUIT) break;
    }

    delete_profile_table(&current_profile_table);
    delete_profile_view(&current_profile_view);
    delete_buffer(&buf);
    close(client_sfd);
  }

  close(host_sfd);
  return 0;
}
