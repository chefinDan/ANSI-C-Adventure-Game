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

char const *_dirPfx = "greendan.room.";

struct Room{
  char *name;
  char *type;
};

struct RoomList{
  struct Room listFor;
  int numOfRooms;
  struct Room *connections;
};

struct Graph{
  int rmMax;
  int maxConnects;
  struct RoomList *roomListArray;
};


struct Room *newRoom(char *name, char *type);
struct Graph *makeGraph(int v, int a, char* names[]);
char** getRoomConnections(struct Graph* graph, int node);
void printRoomConnections(struct Graph *graph,  int node);
int canAddConnection(struct Graph* graph, int src, int dest);
void addConnection(struct Graph* graph, int src, int dest);
int graphIsFull(struct Graph *graph);
void printMap(struct Graph* graph);
DIR* buildDir(char *dirname, struct Graph* graph);
char* replacechar(char *str, char orig, char rep, size_t n);
void generateConnections(struct Graph* graph);


int main(){

  char dirName[64];
  char *roomNames[] = {"Yeezus", "Dr. Octagonecologyst",
                       "Lil Boat", "Daytona", "Deltron",
                       "MM.. Food", "Supreme Clientele"};
  int maxRooms = 7;
  int maxConnects = 6;
  pid_t pid = getpid();
  struct Graph* graph = makeGraph(maxRooms, maxConnects, roomNames);
  sprintf(dirName, "%s%d", _dirPfx, pid);

  generateConnections(graph);

  DIR* dir = buildDir(dirName, graph);

  printMap(graph);



  return 0;
}



struct Room *newRoom(char *name, char *type){
  struct Room *newRoom = malloc(sizeof(struct Room));
  newRoom->name = name;
  newRoom->type = type;

  return newRoom;
}


struct Graph *makeGraph(int v, int a, char* names[]){
  int i;
  struct Graph *newGraph = malloc(sizeof(struct Graph));
  newGraph->rmMax = v;
  newGraph->maxConnects = a;
  newGraph->roomListArray = malloc(newGraph->rmMax * sizeof(struct RoomList));

  for(i=0; i<v; ++i){
    if(i == 0){
      newGraph->roomListArray[i].listFor = *newRoom(names[i], "START ROOM");
      newGraph->roomListArray[i].numOfRooms = 0;
      newGraph->roomListArray[i].connections = malloc(a * sizeof(struct Room));
    }
    else if(i == v-1){
      newGraph->roomListArray[i].listFor = *newRoom(names[i], "END ROOM");
      newGraph->roomListArray[i].numOfRooms = 0;
      newGraph->roomListArray[i].connections = malloc(a * sizeof(struct Room));
    }
    else{
      newGraph->roomListArray[i].listFor = *newRoom(names[i], "MID ROOM");
      newGraph->roomListArray[i].numOfRooms = 0;
      newGraph->roomListArray[i].connections = malloc(a * sizeof(struct Room));
    }
  }

  return newGraph;
}

/* helper function to get char** list of room connections for a specific room */
char** getRoomConnections(struct Graph* graph, int node){
  int i, j;

    for(i = 0; i<graph->roomListArray[node].numOfRooms;i++){}
    char **connects = malloc((i+1) * sizeof(char *));

    for(j = 0; j<i; j++){
      connects[j] = malloc(32 * sizeof(char));
      connects[j] = graph->roomListArray[node].connections[j].name;
    }
    return connects;
}

void printRoomConnections(struct Graph *graph,  int node){
  int i;

  if(node < 7 && node >= 0){
    char **connects = getRoomConnections(graph, node);
    printf("Node %d has connectiosns to:\n", node);
    for(i = 0; i<graph->roomListArray[node].numOfRooms; i++){
      printf("%s\n",connects[i]);
    }
  }
  else return;
}


int canAddConnection(struct Graph* graph, int src, int dest){
  int i, maxConnects;
  char *srcName, *destName;
  srcName = graph->roomListArray[src].listFor.name;

  /* check for connection with self */
  if(src == dest){
    printf("A room cannoy connect to itself.\n");
    return 0;
  }

  /* check bounds on src and dest */
  if(dest >= graph->rmMax || dest < 0 || src >= graph->rmMax || src < 0){
    printf("Can only add connections from 0-%d\n", graph->rmMax-1);
    return 0;
  }

  /* check for existing connection */
  for(i = 0; i<graph->roomListArray[dest].numOfRooms; i++){
    destName = graph->roomListArray[dest].connections[i].name;
    if(strcmp(destName, srcName) == 0){
      printf("Room %s already has a connection to room %s\n", srcName, destName);
      return 0;
    }
  }


  maxConnects = graph->maxConnects;
  destName = graph->roomListArray[dest].listFor.name;
  if(graph->roomListArray[dest].numOfRooms == maxConnects){
    printf("Room %s has maximum connections.\n", destName);
    return 0;
  }
  else if(graph->roomListArray[src].numOfRooms == maxConnects){
    printf("Room %s has maximum connections.\n", srcName);
    return 0;
  }

  return 1;
}


void addConnection(struct Graph* graph, int src, int dest){
  int i;

  if(!canAddConnection(graph, src, dest)){
    return;
  }

  for(i = 0; i<graph->maxConnects; i++){
    if(graph->roomListArray[src].connections[i].type == NULL){

      graph->roomListArray[src].connections[i] = graph->roomListArray[dest].listFor;
      graph->roomListArray[src].numOfRooms++;
      break;
    }
  }

  for(i = 0; i<graph->maxConnects; i++){
    if(graph->roomListArray[dest].connections[i].type == NULL){
      graph->roomListArray[dest].connections[i] = graph->roomListArray[src].listFor;
      graph->roomListArray[dest].numOfRooms++;
      break;
    }
  }
}

int graphIsFull(struct Graph *graph){
  int i;
  int fullCnt=0;
  for(i = 0; i<graph->rmMax; i++){
    if(graph->roomListArray[i].numOfRooms >= 3){
      fullCnt++;
    }
  }
  if(fullCnt == 7){
    return 1;
  }
  return 0;
}


void printMap(struct Graph* graph)
{
    int i, j;
    for (i = 0; i<graph->rmMax; i++)
    {

      char *title = graph->roomListArray[i].listFor.name;
      printf("\n\nAdjacency list of room %s\n ", title);

      for(j = 0; j < graph->roomListArray[i].numOfRooms; j++){
        printf("%s\n", graph->roomListArray[i].connections[j].name);
      }
    }
}

/*******************************************************************************
** Function: builDir()
** Author: Daniel Green, greendan@oreonstate.edu
** Description: This function is passed a char* that will be the name of the
*** created directory called <dirname>, and populate new directory with files
*** whose names are build from the array of char* roomnames.
*** It returns an open DIR* if successful, else it returns NULL.
*******************************************************************************/
DIR* buildDir(char *dirname, struct Graph* graph){
  int mkdirRet, i, j;
  char filepath[64];;
  char* roomName;
  FILE *fp;

  mkdirRet = mkdir(dirname, 0755);

  if(mkdirRet != 0){
    printf("%s\n", "error");
    fprintf (stderr, "%s: mkdir(); %s\n",
              program_invocation_short_name, strerror(errno));
    return NULL;
  }

  for(i = 0; i<graph->rmMax; i++){

    roomName = graph->roomListArray[i].listFor.name;
    sprintf(filepath, "%s/%s", dirname, roomName);
    fp = fopen(filepath, "a");
    fprintf(fp, "ROOM NAME: %s\n", roomName);

    for(j = 0; j<graph->roomListArray[i].numOfRooms; j++){
      fprintf(fp, "CONNECTION %d: %s\n",j, graph->roomListArray[i].connections[j].name);
    }

    fprintf(fp, "ROOM TYPE: %s\n", graph->roomListArray[i].listFor.type);

    fclose(fp);
  }

  return opendir(dirname);
}

void generateConnections(struct Graph* graph){
  while(!graphIsFull(graph)){
    addConnection(graph, rand()%7, rand()%7);
  }
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
