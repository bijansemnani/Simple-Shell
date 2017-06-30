#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


extern char **getline();
int forking(char **args);

int first_fork(char **args){
  int pid;
  int status;
  //start fork
  pid = fork();

  //if child enter forking function
  if(pid == 0){
    forking(args);
  }
  //Else if pid > 0 wait (Parent)
  else if(pid > 0){
      while((pid = wait(&status)) != -1);
  } else{
    perror("fork failed");
    exit(1);
  }
  return 1;
}

//Pipe function
int pipe_func(char **args, int index){
  int pipeArray[2];
  int pipePid;

  //Open a pipe
  pipe(pipeArray);

  //Start a pipe fork
  pipePid = fork();

  //If child dup write and stdout
  if(pipePid == 0){
    dup2(pipeArray[1], 1);
    close(pipeArray[0]);
    while (index < 10)
      args[index++] = NULL;

   //If parent dup read and stdin
  }else if(pipePid > 0){
    dup2(pipeArray[0], 0);
    close(pipeArray[1]);
    args[index++] = NULL;

    //Shift remaining args to front of array
    //and recursively call forking with new argsList if not Null
    for(int j = 0; j+index < 10; j++){
      args[j] = args[index+j];
    }
    if(args[0] != NULL){
      forking(args);
    }
  }
  return 1;
}

//Forking function
int forking(char **args){
  int file;

  //Check command line for special cases
  for(int i = 0; args[i] != NULL; i++){
    //File redirect output
    if(strcmp(args[i], ">") == 0){
      file = open(args[i+1],O_WRONLY | O_CREAT, S_IRWXU);
      if(file == -1){
        perror("file didn't open");
        exit(1);
      }
      //dup new file and stdout
      if(dup2(file,1) == -1){
        perror("dup failed");
        exit(1);
      }
      args[i] = " ";
      args[i++] = NULL;
    }
    //File redirect input
    if(strcmp(args[i], "<") == 0){
      file = open(args[i+1],O_RDONLY);
      if(file == -1){
        perror("file did not open");
        exit(1);
      }
      //dup current file and stdin
      if(dup2(file, 0) == -1){
        perror("dup failed");
        exit(1);
      }
      args[i] = " ";
      args[i++] = NULL;
    }
    //Piping
    if(strcmp(args[i], "|") == 0){
      args[i] = " ";
      pipe_func(args, i);

    }

  }
  //Execute args
  if(args[0] != NULL){
   if(execvp(args[0], args) == -1)
    perror("error");
    exit(1);
  }
  return 1;
}

int main() {
   char **args;
   while (1) {
     printf("> ");
     args = getline();
     if (args[0] == NULL){
       continue;
     }
     //Check for exit
     if (strcmp(args[0],"exit") == 0){
       exit(0);
     //Check for cd
     }else if (strcmp(args[0],"cd") == 0){
       if(args[1] == NULL){
         args[1] = getenv("HOME");
       }
       if(chdir(args[1]) != 0){
         perror("error");
       }
       continue;
     }
     first_fork(args);

   }
   return 0;
}
