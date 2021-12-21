//Dennis Preston Beaty
//COSC 360 Lab 3
//9/22/2021
//FakeMake
#include<sys/stat.h>
#include<string.h>
#include<stdlib.h>
#include "fields.h"
#include "dllist.h"

//strucct definition
typedef struct file_data{
    char *file_name;
    int made;
    struct file_data *o_file_data;
} File_Data;

//Used for freeing list of files
void free_list_1(Dllist files){
  Dllist temp;
  File_Data *file;
//traverse the given file list to free
  dll_traverse(temp,files){
    file= (File_Data *)temp->val.v;
    if (file->o_file_data!= NULL){
      //free each o file
      free(file->o_file_data->file_name);
      free(file->o_file_data);

    }
    //free each file
    free(file->file_name);
    free(file);
  }
  //free the list
  free_dllist(files);
}
//Used for freeing list of strings
void free_list_2(Dllist strings){
  Dllist temp;
  //free each string in list
  dll_traverse(temp,strings){
    free(temp->val.s);
  }
  //free list
  free_dllist(strings);
}
//used for inserting into a list
File_Data* list_file_insert(Dllist list,char* file){
  File_Data *inserter= (File_Data *) malloc(sizeof(File_Data));
  inserter->file_name=strdup(file);
  inserter->o_file_data=NULL;
  struct stat buffer;
  if (stat(file,&buffer)== -1) {
    inserter->made=0;
  }
  else{
    inserter->made=buffer.st_mtime;
  }
  dll_append(list,new_jval_v((void *)inserter));
  return inserter;
}
//used for adding ofile to filedata struct
File_Data* add_ofile(File_Data* data_file){
  //remove ending
  char* copy=strdup(data_file->file_name);
  char* just_name=strtok(copy,".");
  char actual_name[strlen(just_name) + 2];
  //add new ending
  strcpy(actual_name,just_name);
  strcat(actual_name,".o");
  free(copy);
  struct stat buffer;
  //updatepointer
  if (stat(actual_name,&buffer)!= -1){
    File_Data *inserter= (File_Data *) malloc(sizeof(File_Data));
    inserter->file_name=strdup(actual_name);
    inserter->made=buffer.st_mtime;
    data_file->o_file_data=inserter;
    return inserter;
  }
  return NULL;
}

int compile(char* type,Dllist f_list,char* print){
  char hold[99];
  Dllist temp;
  //create a string to hold list of flags
  char *flag_combine= (char*) malloc(sizeof(char) * 99);
  flag_combine[0]= '\0';
  dll_traverse(temp,f_list){
    strcat(flag_combine,temp->val.s);
    strcat(flag_combine," ");
  }
  //format and print to stdout
  sprintf(hold,"gcc %s %s%s\n",type,flag_combine,print);
  int end=system(hold);
  printf(hold);
  free(flag_combine);
  return end;
}

int main(int argc,char **argv){
  if (argc>2){
        // Check for correct number of arguments
      fprintf(stderr,"usage: fakemake [description-file]\n");
      exit(1);
  }
  char* read_file= "";
  if (argc==2){
    read_file=argv[1];
  }
  else{
    read_file="fmakefile";
  }
  //Open the read_file
  IS input=new_inputstruct(read_file);
  if (input==NULL) {
    //if file can't be opened
    fprintf(stderr, "fakemake: %s: ", read_file);
    exit(1);
  }
  //Initialize Variables for reading
  char *exec_name=NULL;
  Dllist c_list, h_list, l_list, f_list;
  c_list=new_dllist();
  h_list=new_dllist();
  l_list=new_dllist();
  f_list=new_dllist();
  //read line by line
  while (get_line(input)>=0){
    //if line from input is empty go to next line
    if (input->NF<= 1){
      continue;
    }
    char *specification=input->fields[0];
    //Name of executable
    if (strcmp(specification,"E")==0){
      if (exec_name!= NULL){
        fprintf(stderr,"fmakefile (%d) cannot have more than one E line\n",input->line);
        jettison_inputstruct(input);
        free_list_1(c_list);
        free_list_1(h_list);
        free_list_2(l_list);
        free_list_2(f_list);
        if (exec_name!= NULL){
          free(exec_name);
        }
        exit(1);
      }
      exec_name=strdup(input->fields[1]);
    }
    //source files
    else if (strcmp(specification,"C")==0){
      //for each sourcec file
      for (size_t i=1; i< input->NF; i++){
        File_Data *source_file=list_file_insert(c_list, input->fields[i]);
        //if null free mem
        if (source_file==NULL){
          jettison_inputstruct(input);
          free_list_1(c_list);
          free_list_1(h_list);
          free_list_2(l_list);
          free_list_2(f_list);
          if (exec_name!= NULL){
            free(exec_name);
          }
            exit(1);
        }
        //if not add files
        add_ofile(source_file);
      }
    }
    //Header files
    else if (strcmp(specification,"H")==0){
      for (size_t i=1; i< input->NF; i++){
        if (list_file_insert(h_list,input->fields[i])==NULL){
          //if null free mem
          jettison_inputstruct(input);
          free_list_1(c_list);
          free_list_1(h_list);
          free_list_2(l_list);
          free_list_2(f_list);
          if (exec_name!= NULL){
            free(exec_name);
          }
            exit(1);
        }
      }
    }
    //Flags
    else if (strcmp(specification,"F")==0){
      for (size_t i=1; i< input->NF; i++){
        dll_append(f_list,new_jval_s(strdup(input->fields[i])));
      }
    }
    //Links
    else if (strcmp(specification,"L")==0){
      for (size_t i=1; i< input->NF; i++){
        dll_append(l_list,new_jval_s(strdup(input->fields[i])));
      }
    }
  }
  //Making sure exec_name was specified
  if (exec_name==NULL){
    fprintf(stderr,"No exec_name specified\n");
    //free mem
    jettison_inputstruct(input);
    free_list_1(c_list);
    free_list_1(h_list);
    free_list_2(l_list);
    free_list_2(f_list);
    if (exec_name!= NULL){
      free(exec_name);
    }
    exit(1);
  }
  //Gather data for compilation
  Dllist temp;
  File_Data *buffer;
  int change_time=0;
  dll_traverse(temp,h_list){
    buffer= (File_Data *)temp->val.v;
    if (buffer->made > change_time){
      change_time=buffer->made;
    }
  }
  int exec_num=0;
  struct stat buff;
  //determine if an executable exists
  int detect=(stat(exec_name,&buff)!= -1);
  if (detect){
    exec_num=buff.st_mtime;
  }

  int latest_update=0;
  int compilations=0;
  int negone=0;
  Dllist tmp2;
  File_Data *file_obj;
  //if a file requires an o file compile
  dll_traverse(tmp2,c_list){
    file_obj= (File_Data *)tmp2->val.v;
    //doesnt exist w/o time
    if (file_obj->made==0){
      fprintf(stderr,"fmakefile: %s: No such file or directory\n",file_obj->file_name);
      exit(1);
      }
      //if o doesnt exist or any files have been modified recompile
      if (file_obj->o_file_data==NULL|| file_obj->o_file_data->made< file_obj->made|| file_obj->o_file_data->made< change_time){
        int output=compile("-c",f_list,file_obj->file_name);
        if (output!= 0){
          negone++;
        }
        //update pointer
        if (file_obj->o_file_data==NULL){
          add_ofile(file_obj);
        }
        compilations++;
      }
      //check for new latest time
      if (file_obj->o_file_data->made > latest_update){
        latest_update=file_obj->o_file_data->made;
      }
  }
  //no files compiled
  if (compilations==0 && negone==0){
      compilations= (latest_update > exec_num);
  }
  if (negone!=0) {
    compilations=-1;
  }

  //if compile doesnt work stop program
  if (compilations==-1){
    fprintf(stderr,"Command failed.  Exiting\n");
    //free memory
    jettison_inputstruct(input);
    free_list_1(c_list);
    free_list_1(h_list);
    free_list_2(l_list);
    free_list_2(f_list);
    if (exec_name!= NULL){
      free(exec_name);
    }
    exit(1);
  }
  //comp executable
  if (buff.st_mtime< compilations || !detect || compilations > 0 ){
    /* code */
    int condition=0;
    Dllist tmp;
    File_Data *file2;
    //creation of string
    char* comp= (char*) malloc(sizeof(char) *(strlen(exec_name)+ 4));
    strcpy(comp,"-o ");
    strcat(comp,exec_name);
    //string of args
    char *argues= (char *) malloc(sizeof(char) *200);
    argues[0]= '\0';
    //add o files to argues
    dll_traverse(tmp,c_list){
      file2= (File_Data *)tmp->val.v;
      strcat(argues,file2->o_file_data->file_name);
      strcat(argues," ");
    }
    //add all l files to argues
    dll_traverse(tmp,l_list){
      strcat(argues,tmp->val.s);
      strcat(argues," ");
    }
    argues[strlen(argues)-1]= '\0';
    int output=compile(comp,f_list,argues);

    free(comp);
    free(argues);

    if (output!=0){
      /* code */
      condition= 1;
    }
    if (condition!=0){
      fprintf(stderr,"Command failed.  Fakemake exiting\n");
      //free memory
      jettison_inputstruct(input);
      free_list_1(c_list);
      free_list_1(h_list);
      free_list_2(l_list);
      free_list_2(f_list);
      if (exec_name!= NULL){
        free(exec_name);
      }
      exit(1);
    }
  }
  //no changes so exit
  else{
    printf("%s up to date\n",exec_name);
    //free mem
    jettison_inputstruct(input);
    free_list_1(c_list);
    free_list_1(h_list);
    free_list_2(l_list);
    free_list_2(f_list);
    if (exec_name!= NULL){
      free(exec_name);
    }
    exit(1);
  }
  jettison_inputstruct(input);
  free_list_1(c_list);
  free_list_1(h_list);
  free_list_2(l_list);
  free_list_2(f_list);
  if (exec_name!= NULL){
    free(exec_name);
  }
  return 0;
}
