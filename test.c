#define TEST
#include "wish.c" //just include the main file, the TEST will disble the original main function
#include <stdio.h>
#include <string.h>

int test_parse_string(){
    char** ret_array;
    size_t size;
    {
        //Test 1
        char* c = "Hello         World        ";
        parse_string(c, &ret_array, &size);
        char* ret[] = {"Hello", "World"};
        for(int i = 0; i < size; ++i){
            if(strcmp(ret_array[i], ret[i]) != 0){
                printf("Failed parse string Test 1!\n");
                exit(0);
            }
        }
    }
    {
        //Test 2
        char* c = "           Hello         World        ";
        parse_string(c, &ret_array, &size);
        char* ret[] = {"Hello", "World"};
        for(int i = 0; i < size; ++i){
            if(strcmp(ret_array[i], ret[i]) != 0){
                printf("Failed parse string Test 2!\n");
                exit(0);
            }
        }
    }
    {
        //Test 3
        char* c = "  \t      \t   Hello    \t     World    \t    ";
        parse_string(c, &ret_array, &size);
        char* ret[] = {"Hello", "World"};
        for(int i = 0; i < size; ++i){
            if(strcmp(ret_array[i], ret[i]) != 0){
                printf("Failed parse string Test 3!\n");
                exit(0);
            }
        }
    } 
    {
        char* c = "  \t      \n   Hello    \n     World    \t    ";
        parse_string(c, &ret_array, &size);
        char* ret[] = {"Hello", "World"};
        for(int i = 0; i < size; ++i){
            if(strcmp(ret_array[i], ret[i]) != 0){
                printf("Failed parse string Test 4!\n");
                exit(0);
            }
        }

    }
    printf("Success!!\n");
    exit(0);
    return 1;
}

int main(){
    test_parse_string();
    printf("Hello Wold\n");
}