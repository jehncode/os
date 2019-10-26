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
#define NAME_LEN 8
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
void getRoomDir(char* roomdir, const char* targetdir);
struct Room* initRooms(const int n);
void initConnections(struct Room* room);
struct Room* getRooms(const char* target, int n);

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
    printf("error: current directory could not be accessed\n");
    printf("exitiing program\n");
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
 * Description: retrieves room information from directory
 * @param dir
 * ***************************************************************************/
struct Room* getRooms(const char* target, int n) {
  // get latest room file directory
  char roomdir[BUFFER];
  getRoomDir(roomdir, target);

  // initialize rooms
  struct Room* rooms = initRooms(n);

  // populate room information

  return rooms;
}

/* ****************************************************************************
 * Description: returns array of the randomly-selected rooms
 * @param n: number of rooms to create
 * ***************************************************************************/

int main() {
  char targetdir[BUFFER];
  sprintf(targetdir, "%s", DIRPREFIX);
  //openRoomDir(dir);


  int steps = 0;  // track number of steps (rooms visited) of path
  char path[MAX_PATH][NAME_LEN];  // player's path

  struct Room* rooms = getRooms(targetdir, NUM_ROOMS);


  // generate 7 room files

  free(rooms);
  return 0;
}
