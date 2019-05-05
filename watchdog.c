#include <stdio.h> 
#include <stdlib.h>     // exit
#include <signal.h>     // signal
#include <unistd.h>     // getcwd
#include <sys/stat.h>   // stat
#include <string.h>     // strcpy
#include <time.h>       // time
#include <stdbool.h>    // boolean type for c90
#include <dirent.h>     // list all file
#include <errno.h>

#define MAX_PATH 100
#define KST 9
struct stat cur_stat[10];   // 3 directories, 7 files
struct stat prev_stat[10];  // previous stat
bool first_compare;
char filename[12][MAX_PATH];

void alert(char* name, char *message){
    time_t t;
    struct tm *info;
    char time_str[15];
    time(&t);
    info = gmtime(&t);
    info->tm_hour = (info->tm_hour + KST) % 24;// Korea standard time
    strftime(time_str, 15,"[%I:%M:%S %p]",info);
    printf("%s \"%s\" %s\n",time_str,  name,  message);
}

void checkDirectory(char *directory){
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if(d){
        while((dir = readdir(d)) != NULL){
            if(dir->d_type == DT_DIR){
                //printf(" [%s] is directory\n",dir->d_name);
                continue;
            }
            char full_path[MAX_PATH];
            strcpy(full_path, directory);
            strcat(full_path, "/");
            strcat(full_path, dir->d_name);
            bool available = false;
            for(int i = 0 ; i < 12 ; i ++){
                if(strcmp(full_path, filename[i]) == 0) {
                    available = true;
                    break;
                }
            }
            if(!available){
                alert(dir->d_name, "not allowed file has been deleted!");
                remove(full_path);
            }
        }
        closedir(d);
    }
}

void init(){
    first_compare = true;
    strcpy(filename[0],"/home/dlfltltm/workspace");
    strcpy(filename[1],"/home/dlfltltm/workspace/subdir1");
    strcpy(filename[2],"/home/dlfltltm/workspace/subdir2");
    strcpy(filename[3],"/home/dlfltltm/workspace/aaa.txt");
    strcpy(filename[4],"/home/dlfltltm/workspace/bbb.txt");
    strcpy(filename[5],"/home/dlfltltm/workspace/ccc.txt");
    strcpy(filename[6],"/home/dlfltltm/workspace/subdir1/ddd.txt");
    strcpy(filename[7],"/home/dlfltltm/workspace/subdir1/eee.txt");
    strcpy(filename[8],"/home/dlfltltm/workspace/subdir2/fff.txt");
    strcpy(filename[9],"/home/dlfltltm/workspace/subdir2/ggg.txt");
    strcpy(filename[10],"/home/dlfltltm/workspace/watchdog");
    strcpy(filename[11],"/home/dlfltltm/workspace/watchdog.c");
}

void get_stat(){
    for (int i = 0 ; i < 10 ; i++){
        prev_stat[i] = cur_stat[i];
        stat(filename[i], &cur_stat[i]);
        if(stat(filename[i], &cur_stat[i]) < 0 ){
            //perror("stat error ");
            alert(filename[i], "file name changed!");
        }
    }
}

int check5(){
    checkDirectory(filename[0]);
    checkDirectory(filename[1]);
    checkDirectory(filename[2]);
    return 1;
}

int check10(){
    check5();
    get_stat();
    if(first_compare){
        first_compare = false;
        return 1;
    }
    for(int i = 0 ; i < 10 ; i++){
        if(prev_stat[i].st_mtime != cur_stat[i].st_mtime){
            alert(filename[i], "last modification time changed!");
        }
        if(prev_stat[i].st_atime != cur_stat[i].st_atime){
            alert(filename[i], "last access time changed!");
        }
        if(prev_stat[i].st_size != cur_stat[i].st_size){
            alert(filename[i], "file size changed!");
        }
        if(prev_stat[i].st_uid != cur_stat[i].st_uid){
            alert(filename[i], "file uid changed!");
        }
        if(prev_stat[i].st_gid != cur_stat[i].st_gid){
            alert(filename[i], "file gid changed!");
        }
    }
    return 1;
}

void timer_handler(int signo){
    check5();
    sleep(5);
    check10();
    alarm(5);
}

int main(){
    printf("Watchdog is running. Press Enter to exit\n");
    init();
    signal(SIGALRM,timer_handler);
    alarm(5);
    getchar();
/*
    while(1){
        pause();
    }   
*/
    printf("Watchdog is terminated\n");
    return 0; 
}
