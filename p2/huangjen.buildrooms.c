/* ****************************************************************************
 * Author:  Jenny Huang
 * Date:    October 19, 2019
 * Description: generates 7 different room files which contains one room per 
 * file. These files are created in a directory called 
 * huangjen.rooms.<PROCESS_ID_OF_ROOM_PROGRAM>
 * ***************************************************************************/
#include <stdio.h>
#include <sys/stat.h> // mkdir
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h> 

#define DIRPREFIX "huangjen.rooms."
#define BUFFER 32 
#define NAME_LEN 8
#define NUM_ROOMS 7 // number of rooms to create
#define MAX_ROOMS 10  // maximum number of room names to select from
#define MIN_OB 3 // min outbound connections
#define MAX_OB 6 // max outbound connections

const char* ROOM_NAMES[MAX_ROOMS] = { 
  "squelchr", 
  "sploosho", 
  "nozzleno",
  "splashot",
  "squiffer", 
  "bboozler", 
  "dynamoro", 
  "inkbrush",
  "tslosher", 
  "splatlng" 
};

// room type
enum room_type {
  START_ROOM, MID_ROOM, END_ROOM
};

/* ****************************************************************************
 * Description: struct to represent a single instance of a room
 * note: name and connections are represented as integer indexes for char vals
 * given by ROOM_SELECTION
 * ***************************************************************************/
struct Room {
  int name;
  int connection[MAX_OB];
  int numConn;
  enum room_type type;
};

// function declarations
struct Room* initRooms(int);
void setRoomType(struct Room*, int);
char* typeStr(enum room_type);
void printRooms(struct Room*, int); // for debugging

char* createDirectory();
void createRoomFiles();


/* ****************************************************************************
 * Description: creates the rooms and initializes values
 * @param n: number of rooms
 * ***************************************************************************/
struct Room* initRooms(int n) {
  int selected[MAX_ROOMS] = { 0 };  // for tracking taken names 

  // create array of rooms
  struct Room* rooms = (struct Room*) malloc(sizeof(struct Room) * n);

  int i;
  for (i = 0; i < n; i++) {
    // determine and assign room name
    int temp; // name for room
    do {  // keep selecting random name if name has been taken
      temp = rand() % MAX_ROOMS;
    } while (selected[temp] != 0);
    selected[temp] = 1;
    rooms[i].name = temp;   // assign name to room once determined 
    
    // printf("initRooms: rooms[i].name = %d\n", rooms[i].name);
    // initialize connections for each room
    rooms[i].numConn = 0;
    // initialize room_type to MID, by default
    rooms[i].type = MID_ROOM;
  }

  setRoomType(rooms, n);
  return rooms;
}


/* ****************************************************************************
 * Description: selects the START and END rooms
 * @param rooms: array of rooms
 * @param n: number of rooms
 * ***************************************************************************/
void setRoomType(struct Room* rooms, int n) {
  // determine start and end rooms
  int start = rand() % NUM_ROOMS;
  int end;
  // if same room for start is selected for end, keep randomly selecting
  do {
    end = rand() % NUM_ROOMS;
  } while (start == end);

  // start and end rooms have been determined. Assign them to the rooms
  rooms[start].type = START_ROOM;
  rooms[end].type = END_ROOM;
  
  return;
}


/* ****************************************************************************
 * Description: creates a directory and returns the name of the directory
 * @param n: number of rooms
 * ***************************************************************************/
char* createDirectory() {
  int permission = 0755;  // permissions for directory
  char dir[BUFFER]; // name of directory being created
  sprintf(dir, "%s%d", DIRPREFIX, getpid());  // set name of directory
  // mkdir(dir, permission); // make directory and set permissions
  return dir;
}

/* ****************************************************************************
 * MAIN FUNCTION
 * ***************************************************************************/
int main() {
  srand(time(0));   // use current time to seed for random generator
  struct Room* rooms = initRooms(NUM_ROOMS);

  // create directory for rooms
  char* directory = createDirectory();

  // generate 7 room files

  printRooms(rooms, NUM_ROOMS);  // debugging

  //free rooms
  free(rooms);
  return 0;

}



/* ****************************************************************************
 * Description: prints information for rooms, used for debuggin
 * @param rooms
 * @param n: number of rooms
 * ***************************************************************************/
void printRooms(struct Room* rooms, int n) {
  int i;
  for (i = 0; i < n; i++) {
    struct Room room = rooms[i];
    printf("ROOM NAME: %s\n", ROOM_NAMES[room.name]);
    printf("ROOM TYPE: %s\n\n", typeStr(room.type));
  }
  return;
}

char* typeStr(enum room_type type) {
  char* str[BUFFER];
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
