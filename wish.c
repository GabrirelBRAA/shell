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

//const char *paths[2] = {"/bin/", "/usr/bin/"};
const char ** paths;


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

        if (string[i] == *delim){

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

//Generates a new process and awaits its return
void true_fork_exec(char ** array){
    int rc = fork();
    if(rc < 0){
        printf("Fork Failed!\n");
        exit(1);
    } else if (rc == 0){
        //printf("Child process! rc: %d\n", rc);
        //Maybe a function that checks what needs to be checked here
        //allocate pointer and concatenate
        char * const * arr2 = (char* const *)array;
        char p[500] = {"/bin/"}; //Should check all paths here
        strcat(p, arr2[0]);
        execv(p, arr2);
        printf("Error! %d\n", errno);
    } else {
        int rc_wait = wait(NULL);
        //printf("This is the parent rc:%d\n", rc);
        return;
    }
    exit(0); //Should never reach here
}

//Gets the current dir and prints it
void get_dir(){
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        printf("Current dir: %s\n", cwd);
    } else{
        printf("Failed to get current dir, Error: %d\n", errno);
    }
}

int main(){

    paths = malloc(sizeof(char**) * 2);
    paths[0] = "/bin/";
    paths[1] = "/usr/bin/";

    //Let getline set these
    size_t size = 0;
    char* c = NULL;

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

    //printf("wish>");
    while(1){
        printf("wish> ");
        int ret_size = getline(&c, &size, stdin);
        if(ret_size == -1){
            if(feof(stdin)){
                return 0;
            }
        }

        if(strcmp(c, "\n") == 0){
            continue;
        }

        if(strcmp(c, "exit\n") == 0){
            return 0;
        }

        if(strcmp(c, "dir\n") == 0){
            get_dir();
            continue;
        }

        char** ret_array;
        size_t s;

        char* free_p = parse_string(c, &ret_array, &s);
        //printf("ret_array %ld\n", ret_array);

        //printf("%s", c);
        //printf("Size is: %d\n", size);
        true_fork_exec(ret_array);

        for (int i = 0; i < s; ++i){
            //printf("%s\n", *(ret_array + i));
        }
        free(free_p);
    }

    //Never reaches here
    return 0;
}