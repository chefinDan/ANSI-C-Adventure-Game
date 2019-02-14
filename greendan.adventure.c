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
#include <time.h>

char const *_dirPfx = "greendan.rooms.";
pthread_mutex_t lock;


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


char* openNewestDir();
struct Graph* graphFromFiles(DIR *, char *);
struct Room *newRoom(char *name);
void printMap(struct Graph* graph);
void setype(struct Room* rm, char *type);
static void *getCurrTime(void *arg);

static void *getCurrTime(void *arg){
  pthread_mutex_lock(&lock);
  time_t rawtime;
  struct tm * timeinfo;
  int s;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ( "Current local time and date: %s", asctime (timeinfo) );

  pthread_mutex_unlock(&lock);
  return NULL;
}

int main(){
  pthread_mutex_init(&lock, NULL);
  int i, steps, pathTaken[100];
  DIR* dir;
  char* dirname;
  char input[64];
  struct RoomList* mapCrawl;

  pthread_t t1;
  int s;

  dirname = openNewestDir();

  dir = opendir(dirname);

  struct Graph* graph = graphFromFiles(dir, dirname);

  mapCrawl = graph->roomListArray;

  /*printMap(graph);*/


  for(i=0;i<graph->rmMax;i++){
    if(strcmp(mapCrawl[i].listFor.type, "START ROOM") == 0){
      mapCrawl = &graph->roomListArray[i];
      break;
    }
  }

  steps = 0;
  while(strcmp(mapCrawl->listFor.type, "END ROOM") != 0){

    printf("\nCURRENT LOCATION: %s\n", mapCrawl->listFor.name);
    printf("POSSIBLE CONNECTIONS: ");

    for(i=0;i<mapCrawl->numOfRooms; i++){
      printf("%s, ", mapCrawl->connections[i].name);
    }

    printf("WHERE TO? >");

    fgets(input, 64, stdin);

    while(strcmp(input, "time\n") == 0){
      s = pthread_create(&t1, NULL, getCurrTime, NULL);
      pthread_join(t1, NULL);

      printf("\nWHERE TO? >");

      fgets(input, 64, stdin);
    }

    int valid = 0;
    for(i=0; i<mapCrawl->numOfRooms; i++){
      if(strcmp(mapCrawl->connections[i].name, strtok(input, "\n")) == 0){
        valid = 1;
      }
    }
    if(valid == 1){
      char *travelTo;
      for(i=0;i<mapCrawl->numOfRooms; i++){
        if(strcmp(input, mapCrawl->connections[i].name) == 0){
            travelTo = input;
        }
      }
      for(i=0;i<graph->rmMax; i++){
        if(strcmp(travelTo, graph->roomListArray[i].listFor.name) == 0){
          mapCrawl = &graph->roomListArray[i];
          break;
        }
      }
      pathTaken[steps] = i;
      steps++;
    }
    else{
      printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
    }
  }
  pathTaken[steps] = i;
  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
  for(i = 0; i<steps;i++){
    printf("%s\n",graph->roomListArray[pathTaken[i]].listFor.name);
  }


  return 0;
}


char *getKeyValue(char* key, char* line){
  int keylen, linelen, j, i, k, found;
  keylen = strlen(key);
  linelen = strlen(line);

  for(i=0; i<linelen;i++){
    found = 1;

    for(j=0;j<keylen;j++){
      if(line[i +j] != key[j]){
        found = 0;
        break;
      }
    }
    if(found == 1){
      for(k=0; line[k+keylen]; k++){
        if(line[k+keylen] >= 65 && line[k+keylen] <= 122){
          return line+k+keylen;
        }
      }
    }
  }
  return NULL;
}

void printMap(struct Graph* graph)
{
    int i, j;
    for (i = 0; i<graph->rmMax; i++)
    {

      char *title = graph->roomListArray[i].listFor.name;
      char *type = graph->roomListArray[i].listFor.type;
      printf("\n\nAdjacency list of room: %s\n ", title);
      printf("Room Type: %s\n", type) ;

      for(j = 0; j < graph->roomListArray[i].numOfRooms; j++){
        printf("%s\n", graph->roomListArray[i].connections[j].name);
      }
    }
}


struct Graph* graphFromFiles(DIR* dir, char* dirname){
  struct dirent* file;
  char line[64];
  char fullName[258];
  char* name, *type, *connection;
  int rmCnt, connectCnt;
  FILE *fp;

  struct Graph *graph = malloc(sizeof(struct Graph));
  graph->rmMax = 7;
  graph->maxConnects = 6;
  graph->roomListArray = malloc(7 * sizeof(struct RoomList));

  rmCnt = 0;
  while((file = readdir(dir))){
    connectCnt = 0;

    graph->roomListArray[rmCnt].numOfRooms = 0;
    graph->roomListArray[rmCnt].connections = malloc(6 * sizeof(struct Room));

    if(strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0){
      sprintf(fullName, "%s/%s", dirname, file->d_name);
      fp = fopen(fullName, "r");

      while(fgets(line, 64, fp)){

        if((name = getKeyValue("ROOM NAME", line))){
          graph->roomListArray[rmCnt].listFor.name = malloc(64 * sizeof(char));
          memset(graph->roomListArray[rmCnt].listFor.name, '\0', 64);
          strcpy(graph->roomListArray[rmCnt].listFor.name, strtok(name, "\n"));
        }

        else if((connection = getKeyValue("CONNECTION", line))){
          graph->roomListArray[rmCnt].connections[connectCnt].name = malloc(64 * sizeof(char));
          memset(graph->roomListArray[rmCnt].connections[connectCnt].name, '\0', 64);
          strcpy(graph->roomListArray[rmCnt].connections[connectCnt].name, strtok(connection, "\n"));
          connectCnt++;
          graph->roomListArray[rmCnt].numOfRooms++;
        }

        else if((type = getKeyValue("ROOM TYPE", line))){
          graph->roomListArray[rmCnt].listFor.type = malloc(64 * sizeof(char));
          memset(graph->roomListArray[rmCnt].listFor.type, '\0', 64);
          strcpy(graph->roomListArray[rmCnt].listFor.type, strtok(type, "\n"));
          rmCnt++;
        }
      }
      fclose(fp);
    }
  }
  return graph;
}

struct Room *newRoom(char *name){

}

void setype(struct Room* rm, char *type){
  rm->type = malloc(32 * sizeof(char));
  memset(rm->type, '\0', 32);
  strcpy(rm->type, type);

}

char* openNewestDir(){
  int newestDirTime = -1;
  static char newestDirName[256];
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dir;
  struct dirent *fileInDir;
  struct stat fileStat;

  dir = opendir(".");
  if (dir > 0){
    while ((fileInDir = readdir(dir)) != NULL)
    {
      if (strstr(fileInDir->d_name, _dirPfx) != NULL)
      {
        stat(fileInDir->d_name, &fileStat);

        if ((int)fileStat.st_mtime > newestDirTime){
          newestDirTime = (int)fileStat.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
        }
      }
    }
  }

  return newestDirName;
}
