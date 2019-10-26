/* ****************************************************************************
 * Author:  Jenny Huang
 * Date:    October 19, 2019
 * Description: generates 7 different room files which contains one room per 
 * file. These files are created in a directory called 
 * huangjen.rooms.<PROCESS_ID_OF_ROOM_PROGRAM>
 * ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h> 
#include <unistd.h>
#include <time.h>

#define DIRPREFIX "huangjen.rooms."
#define BUFFER 32
#define NAME_LEN 9 // max 8 + \0
#define NUM_ROOMS 7
#define MAX_ROOMS 10
#define MAX_CONN 6
#define MAX_PATH 100

// room type
enum room_type {
  START_ROOM, MID_ROOM, END_ROOM
};

/* ****************************************************************************
 * Description: struct to represent a single instance of a room
 * ***************************************************************************/
struct Room {
  char name[NAME_LEN];
  char outbounds[MAX_CONN][NAME_LEN];  // names of outbound connections
  int n;  // number of outbound connections
  enum room_type type;
};

/* ****************************************************************************
 * function declarations 
 * ***************************************************************************/
struct Room* initRooms(const int n);
void initConnections(struct Room* room);

struct Room* getRooms(const char* target, int n);
void getRoomDir(char* roomdir, const char* targetdir);
void getRoomFileNames(char** names, const char* loc, int n);
void readRoomFiles(struct Room* rooms, char** names, const char* loc, int n);

void parseLine(char* line, char* key, char* val);
void exitDirAccessError();
void printRooms(const struct Room* rooms, int n);
char* typeStr(enum room_type type);
enum room_type typeFromStr(char* str);

/* ****************************************************************************
 * Description: opens the current directory and loops through each file/
 * subdirectory inside it looking for the entries that begins with the target
 * pathname. Gets the last modified timestamp of it, records the name and 
 * timestamp, and compares these to previously held entries and stores the 
 * newer
 * NOTE: THIS FUNCTION CONSIDERS LECTURE 2.4 SEARCHING THROUGH DIRECTORIES
 * @param roomdir latest room directory
 * @param targetdir target directory prefix
 * ***************************************************************************/
void getRoomDir(char* roomdir, const char* targetdir) { 
  time_t latest;  // timestamp of latest subdir examined
  char newestdir[BUFFER]; // holds the name of newest dir that contains prefix
  memset(roomdir, '\0', sizeof(roomdir)); // clear directory string 

  DIR* tocheck; // start directory
  struct dirent* dirfile; // current subdir of starting dir
  struct stat dirattribute;   // info gained about subdir

  tocheck = opendir("."); // open directory this prog is run in

  if (tocheck == NULL) {    // check if curr dir could be opened
    exitDirAccessError();
    exit(100);
  }

  // check each entry in directory
  while ( (dirfile = readdir(tocheck)) != NULL) {
    // if there's a match for target directory
    if (strstr(dirfile->d_name, targetdir) != NULL) {
      // printf("Found prefix: %s\n", dirfile->d_name);

      // get attribute of entry
      stat(dirfile->d_name, &dirattribute);

      // check time of entry to see if it's latest
      if (dirattribute.st_mtime > latest ) {
        // set latest time and directory
        latest = dirattribute.st_mtime;
        memset(roomdir, '\0', sizeof(roomdir));
        strcpy(roomdir, dirfile->d_name);
        // printf("newer subdir: %s, new time: %d\n", dirfile->d_name, latest);
      }
    }
  }
  closedir(tocheck);
  // printf("Newest entry found is %s\n", roomdir);
}

/* ****************************************************************************
 * Description: initializes room connections to null and connection count to 0
 * @param room
 * ***************************************************************************/
void initConnections(struct Room* room) {
  room->n = 0;
  int i = 0;
  for (; i < MAX_CONN; i++) {
    char* conn = room->outbounds[i];
    memset(conn, '\0', sizeof(conn));
  }
}

/* ****************************************************************************
 * Description: initialize rooms
 * @param dir
 * ***************************************************************************/
struct Room* initRooms(const int n) {
  struct Room* rooms = (struct Room*) malloc(sizeof(struct Room) * NUM_ROOMS);

  int i = 0;
  for (; i < n; i++) {
    struct Room* room = &rooms[i];
    memset(room->name, '\0', sizeof(room->name));   // null room name
    initConnections(room);    // initialize connections
    room->type = MID_ROOM;    // default to mid
  }

  return rooms;
}

/* ****************************************************************************
 * Description: returns room file names 
 * @param rooms
 * @param loc
 * @param n
 * ***************************************************************************/
void getRoomFileNames(char** names, const char* loc, int n) {
  DIR* dir = opendir(loc); // pointer to directory
  if (dir == NULL) {
    printf("attempting to access: %s\n", loc);
    exitDirAccessError();
    exit(100);
  }

  struct dirent* file;
  int ct = 0;
  while ( (file = readdir(dir)) != NULL) {
    if (!isalpha(file->d_name[0])) continue;  // ignore "." and ".."
    // save file names  
    memset(names[ct], '\0', sizeof(names[ct]));
    sprintf(names[ct], "%s\0", file->d_name);
    ct++;
  }

  // debug, print file names
  // int i = 0; for (; i < n; i++) printf("filenames %d: %s\n", i+1, names[i]);
  
  //printRooms(rooms, n);
  closedir(dir);
}

void readRoomFiles(struct Room* rooms, char** names, const char* loc, int n) {
  char filepath[BUFFER];
  FILE* roomfile;

  int i = 0;
  for (; i < n; i++) {
    // open file
    memset(filepath, '\0', sizeof(filepath));
    sprintf(filepath, "./%s/%s", loc, names[i]);
    // printf("filepath: %s\n", filepath);

    roomfile = fopen(filepath, "r+");
    if (roomfile == NULL) {
      printf("attempt to access %s\n", filepath);
      perror("error: could not access file. Proceeding to exit.");
      exit(EXIT_FAILURE);
    }

    struct Room* room = &rooms[i];

    // PARSE FILE
    char line[BUFFER];    // holds line of file
    memset(line, '\0', sizeof(line)); // clear buffer
    
    // go through each line and parse key/value to assign room info
    while (fgets(line, sizeof(line), roomfile) != NULL) {
      // skip if line just contains new line character
      if (strlen(line) == 1) continue;

      // otherwise, parse line for key & value
      char key[BUFFER];
      char val[BUFFER];
          // printf("%s", line);
      parseLine(line, key, val);
          // printf("key: %s\t val: %s\n", key, val);

    // set room info based on key
      // name
      if (strcmp(key, "ROOM NAME") == 0) {          
        sprintf(room->name, "%s\0", val);
        // printf("room->name set: %s\n", room->name);
      } 
      // connections
      if (strcmp(key, "CONNECTION ") == 0) {   
        sprintf(room->outbounds[room->n++], "%s\0", val);
        // printf("room connection added: %s\n", room->outbounds[room->n - 1]);
      }
      // room_type
      if (strcmp(key, "ROOM TYPE") == 0) {
        room->type = typeFromStr(val);
        // printf("room type set: %s\n", typeStr(room->type)); 
      }

      // reset line buffer
      memset(line, '\0', sizeof(line));
    }

    // close file
    fclose(roomfile);
            // printf("\n");
  }

  // printRooms(rooms, n);
}

/* ****************************************************************************
 * Description: parses a line for a key and value used to extract room info 
 * from file lines
 * @param line
 * @param key
 * @param val
 * ***************************************************************************/
void parseLine(char* line, char* key, char* val) {
  // clear buffer
  memset(key, '\0', sizeof(key));
  memset(val, '\0', sizeof(val)); 

  // parse for key
  sprintf(key, "%s\0", strtok(line, ":123456790"));

  // parse for value
  sprintf(val, "%s\0", strtok(NULL, ": \n")); 
}

/* ****************************************************************************
 * Description: retrieves and sets room information from files
 * @param rooms
 * @param loc
 * @param n
 * ***************************************************************************/
void setRoomInfo(struct Room* rooms, const char* loc, int n) {
  // allocate space to store all room file names
  char** filenames = (char**) malloc(sizeof(char*) * n);
  int m = 0; 
  for (; m < n; m++) {
    // space for each file name
    filenames[m] = (char*) malloc(sizeof(char) * (NAME_LEN));
  }

  // get the file names
  getRoomFileNames(filenames, loc, n);
  // int i = 0; for (; i < n; i++) printf("filenames %d: %s\n", i+1, filenames[i]);

  // read files for room info
  readRoomFiles(rooms, filenames, loc, n);

  // free file names
  int f = 0; for (; f < n; f++) free(filenames[f]);
}

/* ****************************************************************************
 * Description: returns rooms retrieved from files
 * @param dir
 * ***************************************************************************/
struct Room* getRooms(const char* target, int n) {
  // get latest room file directory
  char location[BUFFER];
  // sprintf(location, "./%s", roomdir);
  getRoomDir(location, target);
  // printf("getRooms() location = %s\n", location);

  // initialize rooms
  struct Room* rooms = initRooms(n);

  // retrieve room info 
  setRoomInfo(rooms, location, n);

  return rooms;
}

/* ****************************************************************************
 * Description: prints information for rooms, used for debugging
 * @param rooms
 * @param n: number of rooms
 * ***************************************************************************/
void printRooms(const struct Room* rooms, int n) {
  int i;
  for (i = 0; i < n; i++) {
    const struct Room* room = &rooms[i];
    // print room name
    printf("ROOM NAME: %s\n", room->name);

    // print connections
    int j;
    for (j = 0; j < room->n; j++) {
      printf("CONNECTION %d: %s\n", j + 1, room->outbounds[j]);
    }

    // print room type
    printf("ROOM TYPE: %s\n\n", typeStr(room->type));
  }
  return;
}

/* ****************************************************************************
 * Description: returns room_type as a string
 * @param type
 * ***************************************************************************/
char* typeStr(enum room_type type) {
  if (type == START_ROOM) {
    return "START_ROOM";
  }
  if (type == MID_ROOM) {
    return "MID_ROOM";
  }
  if (type == END_ROOM) {
    return "END_ROOM";
  }
  return "";
}

/* ****************************************************************************
 * Description: returns type based on room_type as a string
 * @param str
 * ***************************************************************************/
enum room_type typeFromStr(char* str) {
  if (strcmp(str, "START_ROOM") == 0) {
    return START_ROOM;
  }
  if (strcmp(str, "END_ROOM") == 0) {
    return END_ROOM;
  }
  return MID_ROOM;
} 

/* ****************************************************************************
 * Description: print error message for directory acces failure and exit
 * ***************************************************************************/
void exitDirAccessError() {
  perror("error: directory could not be accessed. Proceeding to exit.\n");
  exit(100);
}

/* ****************************************************************************
 * Description: returns array of the randomly-selected rooms
 * @param n: number of rooms to create
 * ***************************************************************************/
int main() {
  // set up room file search
  char targetdir[BUFFER];
  sprintf(targetdir, "%s", DIRPREFIX);
  memset(targetdir, '\0', sizeof(targetdir));
  strcpy(targetdir, DIRPREFIX);

  // retrieve room info
  struct Room* rooms = getRooms(targetdir, NUM_ROOMS);

  // game
  int steps = 0;  // track number of steps (rooms visited) of path
  char path[MAX_PATH][NAME_LEN];  // player's path

  // generate 7 room files

  free(rooms);
  return 0;
}
