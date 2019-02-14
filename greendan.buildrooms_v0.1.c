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
char* searchForKey(const char *, const char *);


int main(int argc, char const *argv[]) {

  pid_t pid = getpid();
  char dirName[64];
  struct stat filedata;
  struct dirent *fileInDir;
  DIR *dir;
  char filename[512];
  char *roomNames[] = {"Yeezus_room", "Dr._Octagonecologyst",
                       "Lil_Boat_room", "Daytona_room", "Deltron_room",
                       "MFDoom_room", "Supreme_Clientele_room"};

  /* concat _dirpfx(greendan.room) with the PID and store in c-string dirName */
  sprintf(dirName, "%s%d", _dirPfx, pid);

  /* call the buildir() function and pass the c-string dirName and the array of roomName pointers */
  /* buildDir() creates the necessary file structure and populates the room files with */
  /* information. It returns an open DIR*  */
  dir = buildDir(dirName, roomNames);
  if(dir == NULL){
    printf("%s\n", "error");
    fprintf (stderr, "%s: opendir(); %s\n",
    program_invocation_short_name, strerror(errno));
  }


  /* call readdir() on the DIR* returned by buildDir() */
  /*while((fileInDir = readdir(dir))){
    if(strcmp(fileInDir->d_name, ".") != 0 && strcmp(fileInDir->d_name, "..") != 0){
      sprintf(filename, "%s/%s", dirName, fileInDir->d_name);
      memset(filename, '\0', sizeof(filename));
    }
  }
*/
  closedir(dir);

  return 0;
}

void printRoom(char* filename){

}

/******************************************************************************
** Function: searchForKey()
** Author: Daniel Green
** Description: this function is passed a char* of an existing filename, and
*** a char* to a key word that is to be searched for. If found it returns the
*** address of the first character of the keyword. else it retruns NULL.
*******************************************************************************/
char* searchForKey(const char* filename, const char* key){
  int i, j, found, lineCnt;
  static char line[64];
  FILE *fp;
  const int keylen = strlen(key);
  const int linelen = strlen(line);
  char* loc;
  lineCnt = 0;
  fp = fopen(filename, "r");


  while(fgets(line, 64, fp)){

    lineCnt ++;
    printf("%s", line);

    for(i=0; i<linelen;i++){
      found = 1;

      for(j=0;j<keylen;j++){
        printf("line[%d]:%c key[%d]:%c\n --- line[%d + %d]: %c\n\n",
                i, line[i], j, key[j], i, j, line[i+j]);
        if(line[i +j] != key[j]){
          found = 0;
          printf("BREAK\n");
          break;
        }
      }
      if(found == 1){
        printf("key found on line %d at index: %d\n", lineCnt, i);
        loc = malloc(keylen *sizeof(char));
        memcpy(loc, &line[i], keylen);

        return loc;
      }
    }
  }
  fclose(fp);

  return NULL;
}



/*******************************************************************************
** Function: builDir()
** Author: Daniel Green, greendan@oreonstate.edu
** Description: This function is passed a char* that will be the name of the
*** created directory called <dirname>, and populate new directory with files
*** whose names are build from the array of char* roomnames.
*** It returns an open DIR* if successful, else it returns NULL.
*******************************************************************************/
DIR* buildDir(char *dirname, char *roomnames[]){
  int mkdirRet, i;
  char filepath[64];
  char connectionX[64] = "CONNECTION:";
  FILE *fp;
  char* noLoDashRmName;

  mkdirRet = mkdir(dirname, 0755);

  if(mkdirRet != 0){
    printf("%s\n", "error");
    fprintf (stderr, "%s: mkdir(); %s\n",
              program_invocation_short_name, strerror(errno));
    return NULL;
  }

  for(i = 0; i<ROOMS; i++){
    noLoDashRmName = replacechar(roomnames[i], '_', ' ', 32);
    sprintf(filepath, "%s/%s", dirname, roomnames[i]);
    fp = fopen(filepath, "a");
    if(i == 0){
      fprintf(fp, "ROOM NAME: %s\n%s\n%s\n", noLoDashRmName, connectionX, "ROOM TYPE: START ROOM");
      fclose(fp);
    }
    else if(i == ROOMS-1){
      fprintf(fp, "ROOM NAME: %s\n%s\n%s\n", noLoDashRmName, connectionX, "ROOM TYPE: END ROOM");
      fclose(fp);
    }
    else{
      fprintf(fp, "ROOM NAME: %s\n%s\n%s\n", noLoDashRmName, connectionX, "ROOM TYPE: MID ROOM");
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
