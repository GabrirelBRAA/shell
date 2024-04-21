#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>

char* parse_string(char** string, char*** ret_array, size_t* size){
    size_t s = 0;
    char* ret;
    char* delim = " ";
    size_t length = strlen(*string);
    char ** ret_arr = *ret_array;

    //Copying string for parsing
    char* new_string = malloc((length + 1) * sizeof(char));
    char* free_p = new_string;
    memcpy(new_string, *string, length + 1);

    //Finding how many empty spaces we have
    for (int i = 0; i < length; ++i){
        if ( (new_string)[i] == *delim ){
            if(i == (length - 2)){
                break;
            }
            s++;
        }
        if (new_string[i] == '\n'){
            new_string[i] = ' ';
        }
    }
    ++s;

    ret_arr = malloc(s * sizeof(long));
    printf("ret_arr %ld\n", ret_arr);
    printf("Allocated %d times\n", s);

    //ret = strsep(string, delim);
    int position = 0;  
    while((ret = strsep(&new_string, delim)) != NULL){
        if(*ret == '\0'){
            break;
        }
        *(ret_arr + position) = ret;
        ++position;
    }
    *size = s;
    *ret_array = ret_arr;
    return free_p;
}

void fork_exec(const char* path){
    int rc = fork();
    if(rc < 0){
        printf("Fork Failed!\n");
        exit(1);
    } else if (rc == 0){
        printf("Child process! rc: %d\n", rc);
        char* arr[] = {"ls", NULL};
        execv("/bin/ls", arr);
        printf("Error! %d\n", errno);
    } else {
        int rc_wait = wait(NULL);
        printf("This is the parent rc:%d\n", rc);
        return;
    }
    exit(0);
}

void get_dir(){
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        printf("Current dir: %s\n", cwd);
    } else{
        printf("Failed to get current dir, Error: %d\n", errno);
    }
}

int main(){
    printf("Welcome to the shell\n");
    printf("Say something and I will say it back\n");

    size_t size = 2;
    char* c = (char*) malloc(size * sizeof(char));

    if(access("/bin/ls", X_OK) == 0){
        printf("Ok it works\n");
    } else {
        printf("Did not find the desired result\n");
        int error = errno;
        printf("Error: %d", error);
    }

    while(1){
        printf("wish>");
        int ret_size = getline(&c, &size, stdin);
        if(ret_size == -1){
            if(feof(stdin)){
                return 0;
            }
        }

        if(strcmp(c, "exit\n") == 0){
            return 0;
        }

        if(strcmp(c, "fork\n") == 0){
            fork_exec("");
            continue;
        }

        if(strcmp(c, "dir\n") == 0){
            get_dir();
            continue;
        }

        char** ret_array;
        size_t s;

        char* free_p = parse_string(&c, &ret_array, &s);
        printf("ret_array %ld\n", ret_array);

        printf("%s", c);
        printf("Size is: %d\n", size);

        for (int i = 0; i < s; ++i){
            printf("%s\n", *(ret_array + i));
        }
        free(free_p);
    }
    return 0;
}