//Dennis Preston Beaty
//COSC360
//Lab 7
//jsh3.c
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "jrb.h"
#include "fields.h"

void sign_detect_edit(int dir, int read_val, int write_val, char **argv_temp){
  //if there is a <,>,>> open for in/out
  if (dir==1) {
    /* code */
    if (read_val != -1) {
      /* code */
      dup2(read_val, 0);
    }
    close(read_val);

    if (write_val != -1) {
      /* code */
      dup2(write_val, 1);
    }
    close(write_val);
  }
  execvp(argv_temp[0], argv_temp);
  perror(argv_temp[0]);
  exit(1);
}

void error_route(){
  perror("error");
  exit(1);
}

int main(int argc, char const *argv[]) {
  /* code */
  if (argc>2) {
    /* code */
    //Check for correct # of arguments
    fprintf(stderr, "usage: jsh3 [prompt]\n");
    exit(1);
  }

  const char *argv1= argv[1];
  char **argv_temp;
  char *path_var;
  char *out=NULL;
  char *in=NULL;
  char *out_add=NULL;
  IS input= new_inputstruct(NULL);
  JRB tree= make_jrb();
  JRB temp;
  int amp=0;
  int dir=0;
  int stats;
  int read_val;
  int write_val;
  int add_val;
  int iter;
  int i;
  int parent_val, child_val;
  int array[2];

  if (argv[1]==NULL) {
    /* code */
    printf("jsh: ");
  }
  else if (strcmp(argv1, "-")!=0) {
    /* code */
    printf("%s: ", argv1);
  }

  //start reading in
  while (get_line(input)>=0) {
    /* code */
    if (strcmp(argv1, "-")!=0) {
      /* code */
      printf("%s: ", argv1);
    }
    else if (argv[1]==NULL) {
      /* code */
      printf("jsh: ");
    }
    //Reset variables
    add_val=-1;
    write_val=-1;
    read_val=-1;
    amp=0;
    //Command Checking
    if (input->NF>0) {
      /* code */
      //If the command is only 1 word
      if (input->NF==1) {
        /* code */
        path_var= input->fields[0];
        argv_temp= (char**) malloc(sizeof(char*)*2);
        argv_temp[0]= path_var;
        argv_temp[1]= NULL;
      }
      //if the command is more than 1 word
      else{
        path_var=input->fields[0];
        //check if last character is &
        if (strcmp(input->fields[input->NF-1], "&")==0) {
          /* code */
          amp= 1;
          argv_temp= (char**) malloc(sizeof(char*)*input->NF);
          for (i = 0; i < input->NF-1; i++) {
            /* code */
            argv_temp[i]= input->fields[i];
          }
          argv_temp[input->NF-1]= NULL;
        }
        //there is no &
        else{
          argv_temp= (char**) malloc(sizeof(char*)*(input->NF+1));
          for (i = 0; i < input->NF; i++) {
            /* code */
            argv_temp[i]= input->fields[i];
          }
          argv_temp[input->NF]= NULL;
        }
        //Reset vars
        iter=0;
        dir=0;
        //search for <,>,>>,|
        for (i = 0; i < input->NF; i++) {
          /* code */
          if (argv_temp[i]==NULL) {
            /* code */
            break;
          }
          // <
          if (strcmp(argv_temp[i], "<")==0) {
            /* code */
            in= argv_temp[i+1];
            read_val= open(in, O_RDONLY);
            dir= 1;
            argv_temp[i]= NULL;
            i++;
            argv_temp[i]= NULL;
          }
          //jsh3
          else if(strcmp(input->fields[i], "|")==0){
            argv_temp[iter]= NULL;  //reset vars
            iter=0;
            if (pipe(array)< 0) {
              /* error */
              error_route();
            }
            int fork_track= fork();
            if (fork_track==0) {
              /* code */
              if (read_val!= -1) {
                /* files is able to be read */
                if (dup2(read_val,0)!= 0) {
                  /* code */
                  error_route();
                }
                close(read_val);
                read_val= -1;
              }
              if (dup2(array[1], 1)!= 1) {
                /* stdout -> pipe */
                error_route();
              }
              close(array[0]);
              close(array[1]);
              execvp(argv_temp[0], argv_temp);
              perror(argv_temp[0]);
              exit(1);
            }
            else{
              if (read_val!= -1) {
                close(read_val);
              }
              read_val= array[0]; //stdin -> read
              close(array[1]);
            }
          }
          // >
          else if (strcmp(argv_temp[i], ">")==0) {
            /* code */
            out= argv_temp[i+1];
            write_val= open(out, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            dir= 1;
            argv_temp[i]= NULL;
          }
          // >>
          else if (strcmp(argv_temp[i], ">>")==0) {
            /* code */
            out_add= argv_temp[i+1];
            write_val= open(out_add, O_WRONLY | O_APPEND | O_CREAT, 0644);
            dir= 1;
            argv_temp[i]= NULL;
            i++;
            argv_temp[i]=NULL;
          }
          //add words to argv_temp buffer
          else{
            argv_temp[iter]= input->fields[i];
            iter++;
          }
        }
      }
      //if > redirect, remove last word
      if (out!=NULL) {
        /* code */
        i=0;
        while (argv_temp[i]!=out) {
          /* code */
          if (argv_temp[i]==NULL) {
            /* code */
            break;
          }
          else if (argv_temp[i]==out) {
            /* code */
            argv_temp[i]= NULL;
          }
          i++;
        }
        argv_temp[i]= NULL;
      }
      //begin forking
      if (amp==0) {
        /* code */
        parent_val= fork();
        if (parent_val==0) {
          /* code */
          sign_detect_edit(dir, read_val, write_val, argv_temp);
        }
        else{
          //if a & did not exist keep going until you come across the forked parent_val
          child_val= wait(&stats);
          while (child_val!=parent_val) {
            /* code */
            temp= jrb_find_int(tree, child_val);
            if (temp!=NULL) {
              /* code */
              //Delete the &
              jrb_delete_node(temp);
            }
            child_val= wait(&stats);
          }
        }
      }
      //ampersand exists
      else{
        parent_val= fork();
        if (parent_val==0) {
          /* code */
          jrb_insert_int(tree, parent_val, new_jval_i(1));
          //if there is a <,>,>> open for in/out
          sign_detect_edit(dir, read_val, write_val, argv_temp);
        }
      }
    }
    //reset variables
    out= NULL;
    out_add= NULL;
    in= NULL;
    close(add_val);
    close(read_val);
    close(write_val);
  }
  jettison_inputstruct(input);
  return 0;
}
