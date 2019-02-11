#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
/*#include <sys/types.h> */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int const ROOMS = 7;
char const *_dirPfx = "greendan.room.";

int cleanup(char *, const char *);
char* replacechar(char *, char, char, size_t);
DIR* buildDir(char *, char *[]);
int getConnects(char*);


int main(int argc, char const *argv[]) {

  pid_t pid = getpid();
  char dirName[64];
  struct stat filedata;
  struct dirent *fileInDir;
  DIR *dir;
  char *roomNames[] = {"Yeezus_room", "Dirty_Sprite_room",
                       "Lil_Boat_room", "Daytona_room", "Deltron_room",
                       "MFDoom_room", "Supreme_Clientele_room"};


  sprintf(dirName, "%s%d", _dirPfx, pid);

  dir = buildDir(dirName, roomNames);
  if(dir == NULL){
    printf("%s\n", "error");
    fprintf (stderr, "%s: opendir(); %s\n",
    program_invocation_short_name, strerror(errno));
  }

  char filename[512];
  while((fileInDir = readdir(dir))){
    if(strcmp(fileInDir->d_name, ".") != 0 && strcmp(fileInDir->d_name, "..") != 0){
      sprintf(filename, "%s/%s", dirName, fileInDir->d_name);
      getConnects(filename);
      memset(filename, '\0', sizeof(filename));
    }
  }

  closedir(dir);

  return 0;
}


int getConnects(char* filename){
  int count = 0;
  char line[128];
  FILE* fp = fopen(filename, "r");
  while(fgets(line, 64, fp)){
    printf("%s\n", line);
  }
  fclose(fp);

  return count;
}



/*******************************************************************************
** Function to make a directory called dirname, and populate it with files
** whose names are build from the array of char* roomnames.
**It returns an open DIR*
*******************************************************************************/
DIR* buildDir(char *dirname, char *roomnames[]){
  int mkdirRet, i;
  char filepath[64];
  char connectionX[64] = "CONNECTION X";
  FILE *fp;
  char* noLoDashRmName;

  mkdirRet = mkdir(dirname, 0755);

  if(mkdirRet != 0){
    printf("%s\n", "error");
    fprintf (stderr, "%s: mkdir(); %s\n",
              program_invocation_short_name, strerror(errno));
  }

  for(i = 0; i<ROOMS; i++){
    noLoDashRmName = replacechar(roomnames[i], '_', ' ', 32);
    sprintf(filepath, "%s/%s", dirname, roomnames[i]);
    fp = fopen(filepath, "a");
    if(i == 0){
      fprintf(fp, "ROOM NAME: %s\n%s\n%s", noLoDashRmName, connectionX, "ROOM TYPE: START ROOM");
      fclose(fp);
    }
    else if(i == ROOMS-1){
      fprintf(fp, "ROOM NAME: %s\n%s\n%s", noLoDashRmName, connectionX, "ROOM TYPE: END ROOM");
      fclose(fp);
    }
    else{
      fprintf(fp, "ROOM NAME: %s\n%s\n%s", noLoDashRmName, connectionX, "ROOM TYPE: MID ROOM");
      fclose(fp);
    }
    memset(noLoDashRmName, '\0', 32);
    free(noLoDashRmName);
  }


  return opendir(dirname);
}


/******************************************************************************
** Func: replacechar()
** Author: Daniel Green, greendan@oregonstate.edu
** Usage: str is a c-string, orig is the char to be replaced, rep is the
** replacement char, n is the size of char pointer to allocate on the heap.
** Return: a newly allcated char*.
*******************************************************************************/
char* replacechar(char *str, char orig, char rep, size_t n) {
    char *iix = malloc(n * sizeof(char));
    memset(iix, '\0', n);
    char a;
    int i;

    for (i=0; str[i] != '\0'; i++){
      a = str[i];
      if(a == orig){
        iix[i] = rep;
      }
      else{
        iix[i] = a;
      }
    }

    return iix;
}


/******* NOT USED *********************************/
/*****************************************************************************/
/* Author: Daniel Green, greendan@oreonstate.edu
** Descriptions: A function to recursively enter all directories in <path> that
** have the prefix <delstr> and remove all files including the directories.
******************************************************************************/

int cleanup(char* path, const char *delstr){
  struct dirent *file;
  struct stat st;
  char newpath[256];

  DIR *dir = opendir(path);

  /* path is a file, so remove it */
  if(!dir){
    fprintf (stderr, "%s: opendir(); %s, %s\n",
              program_invocation_short_name, path, strerror(errno));
    /*remove(path);*/
    return 1;
  }
  while((file = readdir(dir)) != NULL){
    int result = stat(file->d_name, &st);
    if(strstr(file->d_name, delstr) != NULL){
        strcpy(newpath, path);
        strcat(newpath, "/");
        strcat(newpath, file->d_name);
        printf("recurse into %s\n", newpath);
        cleanup(newpath, delstr);
      }
    }

    /*remove(path);*/
    closedir(dir);
  return 0;
}
