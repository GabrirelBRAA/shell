#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>

/*
The main function is at the bottom
The code is all contained in this single file.
*/

char ** paths; //paths stores all the paths the shell searches for commands
size_t paths_size;
char* current_path; //This is the current directory the program is in


/*
    Parses the string by creating substrings using empty spaces as separators

    Caller should call free on the returned pointer when done with the array
*/
char* parse_string(char* string, char*** ret_array, size_t* size){

    size_t s = 0; //size of return array
    char* delim = " ";
    size_t length = strlen(string);
    char ** ret_arr = *ret_array;

    //New write string for parsing
    char* new_parsed_string = malloc((length + 1) * sizeof(char));
    char* free_p = new_parsed_string;

    int delim_flag = 0; //true if we are in an empty space sequence
    int write_flag = 0; //true if we have already written some char
    int write_i = 0; //index of the write string
    
    //Removing duplicate empty spaces and spaces at begining and end
    for (int i = 0; i < length; ++i){

        if (string[i] == *delim || string[i] == '\t' || string[i] == '\n'){

            delim_flag = 1;
            continue;

        } else {

            if(i == length - 1){
                //last char is always \n and we dont want it neither the empty space before
                //so just end it here
                break;

            } else if (delim_flag == 1) {
                //This if means we were on an empty space sequence and are now in an letter
                //write a single empty space to the buffer in place of the whole sequence of spaces
                if (write_flag){
                    new_parsed_string[write_i] = ' ';
                    ++write_i;
                    s++;
                }
                delim_flag = 0;

            }
        }

        new_parsed_string[write_i] = string[i];
        write_flag = 1;//write flag is used to stop writing an empty space if we have not written an actual letter before
        ++write_i;

    }
    new_parsed_string[write_i] = '\0';
    ++s;

    //printf("%s\n", new_parsed_string);
    //printf("length: %d", strlen(new_parsed_string));
    //printf("size of s: %d", s);
    //exit(0);

    ret_arr = malloc(1 + s * sizeof(long)); //Dont think this buffer is being allocated properly because of s variable
    //printf("ret_arr %ld\n", ret_arr);
    //printf("Allocated %d times\n", s);

    int position = 0;  
    char* current_string;

    //Separating string and setting pointers in array
    while((current_string = strsep(&new_parsed_string, delim)) != NULL){
        if(*current_string == '\0'){
            break;
        }
        ret_arr[position] = current_string;
        ++position;
    }
    ret_arr[s] = NULL;

    *size = s;
    *ret_array = ret_arr;
    return free_p; //Freeing this will free all the strings in ret_array
}

//Not being used
int validate_path(char* exe_name){

    char p[PATH_MAX];

    for(int i = 0; i < paths_size; ++i){
        strcat(p, paths[i]); 
        strcat(p, exe_name); 

        if(access(p, X_OK) == 0){
            printf("Ok it works\n");
        } else {
            printf("Did not find the desired result\n");
            int error = errno;
            printf("Error: %d\n", error);
        }

        printf("%s\n", p);
        p[0] = '\0';
    }
    
}

//Adding new path to char
void add_path(char* new_path){

    char* string;
    size_t new_path_length = strlen(new_path);
    //If the path does not have the trailing / we add it
    if(new_path[new_path_length - 1] != '/'){
        ++new_path_length;
        string = (char*) malloc((new_path_length + 1) * sizeof(char));
        strcpy(string, new_path);
        strcat(string, "/");
    } else {
        string = (char*) malloc((new_path_length + 1) * sizeof(char));
        strcpy(string, new_path);
    }

    if(access(string, X_OK) == 0){
        ++paths_size;
        paths = realloc(paths, (paths_size) * sizeof(char*));
        paths[paths_size - 1] = string;
    } else {
        free(string);
        printf("Path is inacessible\n");
    }

}

//Generates a new process and awaits its return
void fork_exec(char ** array){

    char * const * arr2 = (char* const *)array;
    char path[PATH_MAX] = "\0";
    for (int i = 0; i < paths_size; ++i){
        strcpy(path, paths[i]);
        strcat(path, arr2[0]);
        if(access(path, X_OK) == 0){
            break;           
        } 
        if(i == paths_size - 1){
            printf("Failed to find command\n");
            return;
        }
        path[0] = '\0';
    }

    int rc = fork();
    if(rc < 0){
        printf("Fork Failed!\n");
        exit(1);
    } else if (rc == 0){
        execv(path, arr2);
        printf("Error! %d\n", errno);
    } else {
        int rc_wait = wait(NULL);
        //printf("This is the parent rc:%d\n", rc);
        return;
    }
    exit(0); //Should never reach here
}

//Gets the current dir and prints it
char* get_dir(){
    //char cwd[PATH_MAX];
    char* cwd_p = (char *) malloc((PATH_MAX + 1) * sizeof(char));
    if(getcwd(cwd_p, PATH_MAX) != NULL){
        //printf("Current dir: %s\n", cwd_p);
        return cwd_p;
    } else{
        //printf("Failed to get current dir, Error: %d\n", errno);
        free(cwd_p);
        return NULL;
    }
}

//TODO substitute this into the interactive and batch mode
void process_command(FILE* file){

        size_t size = 0;
        char *c = NULL;

        int ret_size = getline(&c, &size, stdin);
        if(ret_size == -1){
            if(feof(stdin)){
                exit(0);
            }
        }

        if(strcmp(c, "\n") == 0){
            free(c);
            return;
        }

        if(strcmp(c, "exit\n") == 0){
            free(c);
            exit(0);
        }

        if(strcmp(c, "dir\n") == 0){
            //char* p = get_dir();
            //free(p);
            printf("%s\n", current_path);
            free(c);
            return;
        }

        if(strcmp(c, "printpath\n") == 0){
            for(int i = 0; i < paths_size; i++){
                printf("%s\n", paths[i]);
            }
            free(c);
            return;
        }

        char** ret_array;
        size_t s;

        char* free_p = parse_string(c, &ret_array, &s);

        if(strcmp(ret_array[0], "path") == 0){
            printf("path command\n");
            for(int i = 1; i < s; i++){
                add_path(ret_array[i]);
            }
            free(free_p);
            free(c);
            return;
        }

        if(strcmp(ret_array[0], "cd") == 0){
            chdir(ret_array[1]);
            current_path = get_dir();
            free(free_p);
            free(c);
            return;
        }

        fork_exec(ret_array);


        free(free_p);
        free(c);

}

void batch_mode(FILE* file){
    while(1){
        process_command(file);
    }

}

void interactive_mode(){
    size_t size = 0;
    char* c = NULL;
    while(1){
        printf("galsh> ");
        process_command(stdin);
    }
}



#ifndef TEST
int main(int argc, char** argv){

    /* Allocates these initial directories as search paths*/
    paths = malloc(sizeof(char**) * 2);
    paths[0] = "/bin/";
    paths[1] = "/usr/bin/";
    paths_size = 2;

    current_path = get_dir();

    /*This part is supposed to read the .txt file and do all the specified commands*/
    if(argc == 2){
        FILE* f = fopen(argv[1], "r");
        char* line = NULL;
        size_t line_size;
        batch_mode(f);
        fclose(f);
        exit(0);
    }

    //Useful for checking paths later
    /*
    if(access("/bin/ls", X_OK) == 0){
        printf("Ok it works\n");
    } else {
        printf("Did not find the desired result\n");
        int error = errno;
        printf("Error: %d", error);
    }
    */

    interactive_mode();
    //Never reaches here
    return 0;
}
#endif

//Old interactive function, not really used right now
void interactive_mode_old(){
    size_t size = 0;
    char* c = NULL;
    while(1){
        printf("galsh> ");
        int ret_size = getline(&c, &size, stdin);
        if(ret_size == -1){
            if(feof(stdin)){
                return;
            }
        }

        if(strcmp(c, "\n") == 0){
            continue;
        }

        if(strcmp(c, "exit\n") == 0){
            return;
        }

        if(strcmp(c, "dir\n") == 0){
            //char* p = get_dir();
            //free(p);
            printf("%s\n", current_path);
            continue;
        }

        if(strcmp(c, "printpath\n") == 0){
            for(int i = 0; i < paths_size; i++){
                printf("%s\n", paths[i]);
            }
            continue;
        }

        char** ret_array;
        size_t s;

        char* free_p = parse_string(c, &ret_array, &s);
        //printf("ret_array %ld\n", ret_array);

        if(strcmp(ret_array[0], "path") == 0){
            printf("path command\n");
            for(int i = 1; i < s; i++){
                add_path(ret_array[i]);
            }
            free(free_p);
            continue;
        }

        if(strcmp(ret_array[0], "cd") == 0){
            printf("cd command\n");
            free(free_p);
            continue;
        }

        //printf("%s", c);
        //printf("Size is: %d\n", size);
        fork_exec(ret_array);

        for (int i = 0; i < s; ++i){
            //printf("%s\n", *(ret_array + i));
        }

        free(free_p);
    }

}

//old batch mode function
void batch_mode_old(FILE* file){
    size_t size = 0;
    char* c = NULL;
    while(1){
        int ret_size = getline(&c, &size, file);
        if(ret_size == -1){
            if(feof(file)){
                return;
            }
        }

        if(strcmp(c, "\n") == 0){
            continue;
        }

        if(strcmp(c, "exit\n") == 0){
            exit(0);
        }

        if(strcmp(c, "dir\n") == 0){
            //char* p = get_dir();
            //free(p);
            printf("%s\n", current_path);
            continue;
        }

        if(strcmp(c, "printpath\n") == 0){
            for(int i = 0; i < paths_size; i++){
                printf("%s\n", paths[i]);
            }
            continue;
        }

        char** ret_array;
        size_t s;

        char* free_p = parse_string(c, &ret_array, &s);
        //printf("ret_array %ld\n", ret_array);

        if(strcmp(ret_array[0], "path") == 0){
            printf("path command\n");
            for(int i = 1; i < s; i++){
                add_path(ret_array[i]);
            }
            free(free_p);
            continue;
        }
        //printf("%s", c);
        //printf("Size is: %d\n", size);
        fork_exec(ret_array);

        for (int i = 0; i < s; ++i){
            //printf("%s\n", *(ret_array + i));
        }

        free(free_p);
    }

}