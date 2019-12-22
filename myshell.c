/****************************************************************
 * Name        : Vimean Sam                                                *
 * Class       : CSC 415                                        *
 * Date        : 10/7/2018                                               *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFERSIZE 256
#define PROMPT "myShell "
#define PROMPTSIZE sizeof(PROMPT)

void clear(){
    printf("\033[H\033[J");
}

void parse(char* input, char* par[], char* par2[]){
    //tokens
    char* token;
    char* args[BUFFERSIZE];
    int i = 0;
    int j = 0;
    int l = 0;
    int index = 0;
    int pipe_index = 0;
    //create delimiter
    const char* delim = " ";
    token = strtok(input, delim);
    //loop through tokens
    while( token != NULL ) {
        args[i++] = token;
        token = strtok(NULL, delim);
    }
    //parameters
    for(j = 0; j < i; j++){
        par[j] = args[j];
        index++;
        if(strcmp(args[j], "|") == 0){
            pipe_index = index;
            break;
        }
    }
    par[j] = NULL;
    //printf("%d\n", pipe_index);
    //pipe arguments
    for(int k = pipe_index; k < i; k++){
        par2[l++] = args[k];
    }
    par2[l] = NULL;
}

void execute(char* par[], int backflag, int outflag, int return_out, int inflag, int return_in){
    pid_t id = fork();
    if(id <  0){
        perror("error forking\n");
    }else if(id == 0){ 
        if(execvp(par[0],par) < 0 && (strcmp(par[0],"exit") != 0) && (strcmp(par[0],"cd") != 0) && (strcmp(par[0],"pwd") != 0)){
            perror("Invalid Command");
        }
        exit(0);
    }else{
        //standard input
        if(outflag > 0){
            dup2(return_out,1);
            close(return_out);
        }
        //standard output
        if(inflag == 1){
            dup2(return_in, 0);
            close(return_in);
        }
        //run process in background
        if(backflag == 0){
            int wstatus;
            waitpid(id, &wstatus, 0);
        }
    }
}
//execute a single shell pipe
//create 2 process, a pipe, dup lhs of pipe to stdout, dup rhs of pipe to stdin
void executePipe(char* par1[], char* par2[]){
    //create 2 process
    pid_t id1, id2;
    //create a pipe
    //0 = read, 1 = write
    int pipe_id[2];
    if(pipe(pipe_id) < 0){
        perror("error creating pipe\n");
    }
    //process 1
    id1 = fork();
    if(id1 < 0){
        perror("error forking\n");
    }else if (id1 == 0) {
        //dup lhs to stdout
        dup2(pipe_id[1], 1);
        close(pipe_id[0]);
        //execute lhs
        if(execvp(par1[0], par1) < 0){
            perror("Invalid Command");
            exit(0);
        }
        exit(0);
    }
    //process 2
    id2 = fork();
    if(id2 < 0){
        perror("error forking\n");
    }else if (id2 == 0) {
        //dup rhs to stdin
        dup2(pipe_id[0], 0);
        close(pipe_id[1]);
        //execute rhs
        if(execvp(par2[0], par2) < 0){
            perror("Invalid Command");
            exit(0);
        }
        exit(0);
    }
    //close pipes
    close(pipe_id[0]);
    close(pipe_id[1]);
    //wait for processes to finish
    int wstatus;
    waitpid(id1, &wstatus, 0);
    waitpid(id2, &wstatus, 0);
}

int main(int* argc, char** argv)
{
    clear();
    char userinput[BUFFERSIZE];
    char* parameters[BUFFERSIZE];
    char cwd[BUFFERSIZE];
    char* file_name;
    char* current_path;
    char* homepath;
    int offset;
    //background
    int background_flag = 0;
    //pipe
    char* pipe_par[BUFFERSIZE];
    int pipe_flag = 0;
    //redirect standard output
    int std_output = 0;
    int fd = 0;
    int display_out = 0;
    //redirect standard input
    int std_input = 0;
    int fd_in = 0;
    int display_in = 0;
    
    while(1){
    //prompt the use for input
        homepath = getenv("HOME");
        current_path = getcwd(cwd, sizeof(cwd));
        offset = strlen(homepath);
        if(strstr(current_path, homepath) != NULL){
            printf("%s%s",PROMPT,"~");
            if(strcmp(current_path, homepath) == 0){
                printf("/ ");
            }else{
                printf("%.*s%s ",0, current_path, &current_path[offset]);
            }
            printf(">> ");
        }else{
            printf("%s%s >> ",PROMPT, current_path);
        }
        fgets(userinput, BUFFERSIZE, stdin);
        background_flag = 0;
        pipe_flag = 0;
        //reset std output
    	std_output = 0;
    	fd = 0;
    	display_out = 0;
    	//reset std in
    	std_input = 0;
        fd_in = 0;
        display_in = 0;
    	//clear symbols that may cause errors
        char *symbols;
        if ((symbols = strchr(userinput, '\n')) != NULL){
            *symbols = '\0';
        }
        if ((symbols = strchr(userinput, '&')) != NULL){
            *symbols = '\0';
            background_flag = 1;
        }
        if ((symbols = strchr(userinput, '>')) != NULL){
            char* temp;
            //for append function
            if ((temp = strchr(symbols+1, '>')) != NULL){
                file_name = symbols+3;
                *symbols = '\0';
                std_output = 2;
            }else{
                file_name = symbols+2;
                *symbols = '\0';
                std_output = 1;
            }
        }
        if ((symbols = strchr(userinput, '<')) != NULL){
            file_name = symbols+2;
            *symbols = '\0';
            std_input = 1;
        }
        if ((symbols = strchr(userinput, '|')) != NULL){
            pipe_flag = 1;
        }
        //tokenize arguments
        parse(userinput, parameters, pipe_par);
        //redirect to file
        if(std_output > 0){
            if(std_output == 1){
                fd = open(file_name, O_WRONLY|O_TRUNC|O_CREAT, S_IRWXU); 
            }else{
                fd = open(file_name, O_WRONLY|O_APPEND|O_CREAT, S_IRWXU); 
            }
            display_out = dup(1);
            dup2(fd,1);
            close(fd);
        }
        if(std_input == 1){
            fd_in = open(file_name, O_RDONLY, S_IRWXU);
            display_in = dup(0);
            dup2(fd_in, 0);
            close(fd_in);
        }
        if(strcmp(userinput, "exit") == 0){
            break;
        }
        if(strcmp(userinput, "cd") == 0){
            if(chdir(parameters[1]) < 0){
                perror("Invalid Path");
            }
        }
        if(strcmp(userinput, "pwd") == 0){
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Current working dir: %s\n", cwd);
            }else{
                perror("cannot find working directory");
            }    
        }
        //execution based on flag status and pipe status
        if(pipe_flag == 1){
            executePipe(parameters,pipe_par);
        }else{
            execute(parameters, background_flag, std_output, display_out, std_input, display_in);
        }
    }
    return 0;
}
