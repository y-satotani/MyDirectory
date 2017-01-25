
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

int cmd_invalid(int argc, char* argv[], int sfd, struct buffer* buf) {
  fprintf(stderr, "command \"%s\" not found\n", argv[0]);
  return PS_SUCCESS;
}

int cmd_check(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  int n_entries = 0;
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_CHECK);
  n_entries = deserialize_long(buf);

  printf("%d profiles in view\n", n_entries);
  return status;
}

int cmd_print(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  int n_entries;
  struct profile_table table;
  struct profile_view view;
  if(argc < 2) {
    fprintf(stderr, "profile number not provided\n");
    return PS_INVARG;
  }

  n_entries = atoi(argv[1]);
  new_profile_table(&table, MAX_PROFILES);
  new_profile_view(&view, MAX_PROFILES);
  serialize_long(buf, n_entries);
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_PRINT);
  deserialize_profile_table(&table, buf);

  create_view_from(&view, &table);
  write_profiles_fancy(stdout, &view);

  delete_profile_view(&view);
  delete_profile_table(&table);
  return status;
}

int cmd_readn(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  int n_entries;
  char* filename;
  if(argc < 2) {
    fprintf(stderr, "file name is not provided\n");
    return PS_INVARG;
  }

  filename = argv[1];
  serialize_str(buf, filename);
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_READ);
  n_entries = deserialize_long(buf);

  printf("read %d profiles on server (status=%d)\n", n_entries, status);
  return status;
}

int cmd_readl(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  FILE *fp;
  struct profile_table table;
  long n_entries;
  if(argc < 2) {
    fprintf(stderr, "file name not provided\n");
    return PS_INVARG;
  }

  fp = fopen(argv[1], "r");
  if(!fp) {
    fprintf(stderr, "file not found\n");
    return PS_INVARG;
  }

  new_profile_table(&table, MAX_PROFILES);
  read_profiles_csv(&table, fp);
  fclose(fp);

  serialize_profile_table(buf, &table);
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_READ_L);
  n_entries = deserialize_long(buf);

  printf("read %li profiles on client (status=%d)\n", n_entries, status);
  return status;
}

int cmd_writen(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  int n_entries;
  char* filename;
  if(argc < 2) {
    fprintf(stderr, "file name is not provided\n");
    return PS_INVARG;
  }

  filename = argv[1];
  serialize_str(buf, filename);
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_WRITE);
  n_entries = deserialize_long(buf);

  printf("write %d profiles on server\n", n_entries);
  return status;
}

int cmd_writel(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  char* filename;
  FILE* fp;
  struct profile_table table;
  struct profile_view view;

  if(argc < 2) {
    fprintf(stderr, "file name is not provided\n");
    return PS_INVARG;
  }
  filename = argv[1];
  fp = fopen(filename, "w");
  if(!fp) {
    fprintf(stderr, "cannot open file\n");
    return PS_INVARG;
  }
  new_profile_table(&table, MAX_PROFILES);
  new_profile_view(&view, MAX_PROFILES);

  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_WRITE_L);
  deserialize_profile_table(&table, buf);
  create_view_from(&view, &table);

  write_profiles_csv(fp, &view);
  fclose(fp);
  printf("write %d profiles on local\n", table.n_entries);

  delete_profile_view(&view);
  delete_profile_table(&table);
  return status;
}

int cmd_sort(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  int col;

  if(argc < 2) {
    fprintf(stderr, "column number is not provided");
    return PS_INVARG;
  }
  col = atoi(argv[1]);

  serialize_short(buf, col);
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_SORT);

  if(status == PS_SUCCESS)
    printf("sort profile table by column %d\n", col);
  else
    fprintf(stderr, "invalid profile number %d\n", col);
  return status;
}

int cmd_find(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  char word[128] = "\0";
  int i;
  struct profile_table table;
  struct profile_view view;
  if(argc < 2) {
    fprintf(stderr, "search word not provided\n");
    return PS_INVARG;
  }
  for(i = 1; i < argc; i++) {
    strcat(word, argv[i]);
    if(i < argc-1)
      strcat(word, " ");
  }
  new_profile_table(&table, MAX_PROFILES);
  new_profile_view(&view, MAX_PROFILES);

  serialize_str(buf, word);
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_FIND);
  deserialize_profile_table(&table, buf);

  if(status == PS_SUCCESS) {
    create_view_from(&view, &table);
    write_profiles_fancy(stdout, &view);
  }

  delete_profile_view(&view);
  delete_profile_table(&table);
  return status;
}

int cmd_quit(int argc, char* argv[], int sfd, struct buffer* buf) {
  short cmd_index, status;
  write_buffer(sfd, buf, 1024);
  reset_buffer(buf);

  read_buffer(sfd, buf, 1024);
  cmd_index = deserialize_short(buf);
  status = deserialize_short(buf);
  assert(cmd_index == CMD_QUIT);

  printf("quit\n");
  return status;
}

int main(int argc, char* argv[]) {
  struct hostent* host;
  struct sockaddr_in hostaddr;
  int sfd;

  char* host_name;
  int port_no;

  if(argc <= 2) {
    return -1;
  }
  host_name = argv[1];
  port_no = atoi(argv[2]);

  host = gethostbyname(host_name);
  if(host == NULL) {
    fprintf(stderr, "Error: host not found\n");
    return -1;
  }

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sfd < 0) {
    fprintf(stderr, "Error: could not open socket\n");
    return -1;
  }

  hostaddr.sin_family = host->h_addrtype;
  bzero((char*)&hostaddr.sin_addr, sizeof(hostaddr.sin_addr));
  memcpy((char*)&hostaddr.sin_addr, (char*)host->h_addr,
	 host->h_length);
  hostaddr.sin_port = htons(port_no);
  if(connect(sfd, (struct sockaddr*)&hostaddr, sizeof(hostaddr)) < 0) {
    fprintf(stderr, "Error: connection failed\n");
    return -1;
  }

  struct buffer buf;
  new_buffer(&buf, 1024);

  char *request = NULL;
  size_t req_len = 0;
  ssize_t read;
  command_index index = CMD_INVALID;
  while(index != CMD_QUIT) {
    // get request
    read = getline(&request, &req_len, stdin);
    if(read == -1) break;

    int i;
    for(i = 0; i < req_len; i++) {
      if(request[i] == '\r' || request[i] == '\n')
	request[i] = '\0';
    }

    // send request
    //exec_command(request);
    char* argv[16];
    int argc = split(request, argv, ' ', 16);
    //int status;

    index = command_index_of(argv[0]);
    reset_buffer(&buf);
    serialize_short(&buf, index); // header as command index
    //status = command_table[index](argc, argv, sfd, &buf);
    command_table[index](argc, argv, sfd, &buf);
  }

  if(request) free(request);
  delete_buffer(&buf);
  close(sfd);
  return 0;
}
