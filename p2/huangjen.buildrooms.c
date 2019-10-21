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

#define DIRPREFIX "./huangjen.rooms."
#define BUFFER 32 
#define NAME_LEN 8
#define NUM_ROOMS 7 // number of rooms to create
#define MAX_ROOMS 10  // maximum number of room names to select from
#define MIN_CONN 3 // min outbound connections
#define MAX_CONN 6 // max outbound connections

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
  int outbounds[MAX_CONN];  // names of outbound connections
  int n_conn; // number of outbound connections
  enum room_type type;
};

// function declarations
struct Room* initRooms(int);
void setRoomType(struct Room*, int);
void makeConnections(struct Room*, int);
int contains(int*, int, int);
char* typeStr(enum room_type);
void printRooms(struct Room*, int); // for debugging

void createDirectory(char*);
void createRoomFiles(const struct Room*, int, const char*); 

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
    rooms[i].n_conn = 0;
    // initialize room_type to MID, by default
    rooms[i].type = MID_ROOM;
  }

  // set up start and end rooms
  setRoomType(rooms, n);

  // set up room connections
  makeConnections(rooms, n);

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
 * Description: makes connections between rooms
 * @param rooms: array of rooms
 * @param n: number of rooms
 * ***************************************************************************/
void makeConnections(struct Room* rooms, int n) {
  int i = 0;
  for (i = 0; i < n; i++) {
    struct Room* room = &rooms[i];    // current room 

    struct Room* sele;                // room to connect with curr

    // add connections if minimum number of connections hasnt been reached
    while (room->n_conn < MIN_CONN) {
      int sel;  // room to connect

      // continue to select another connection if current selection is itself
      // or if the room selected has reached its max num of connections
      // or if there already exists a connection with the selected room
      do {
        sel = rand() % n;     // index of random room to connect
        sele = &rooms[sel];   // get reference to selected room
      } while (sel == i || sele->n_conn > MAX_CONN ||
        contains(room->outbounds, room->n_conn, sele->name) != 0);
      
      // make connection between the two rooms
      // add connection from this room to selected room
      room->outbounds[room->n_conn++] = sele->name; 
      // add connection from selected room to curr room
      sele->outbounds[sele->n_conn++] = room->name;
    }
  }
  return;
}


/* ****************************************************************************
 * Description: returns 1 if array contains value and 0 otherwise
 * @param arr
 * @param n
 * @param val
 * ***************************************************************************/
int contains(int* arr, int n, int val) {
  int i;
  for (i = 0; i < n; i++) {
    if (arr[i] == val) {
      return 1;
    }
  }
  return 0;
}

/* ****************************************************************************
 * Description: creates a directory and returns the name of the directory
 * @param n: number of rooms
 * ***************************************************************************/
void createDirectory(char* dir) {
  int permission = 0755;  // permissions for directory
  //char dir[BUFFER];       // name of directory being created

  sprintf(dir, "%s%d", DIRPREFIX, getpid());  // name of directory
  mkdir(dir, permission); // make directory and set permissions
  printf("directory created: %s\n", dir);

  //return dir;
}

/* ****************************************************************************
 * Description: creates files for each room in a specified directory 
 * @param rooms
 * @param n
 * @param dir
 * ***************************************************************************/
void createRoomFiles(const struct Room* rooms, int n, const char* dir) {
  if (chdir(dir) != 0) {
    printf("error: could not access %s\n", dir);
    return;
  }

  // FILE* file;

  int i;
  for (i = 0; i < n; i++) {
    const struct Room* room = &room[i];
    char* filename = ROOM_NAMES[room->name];
    printf("room->namee = %d\n", room->name);
    printf("filename = %s\n", filename);
    /*
    file = fopen(filename, "w");    // create file with write permissions

    // write room name to file
    fprintf(file, "ROOM NAME: %s\n", ROOM_NAMES[room->name]);
    
    // write connections to file
    int j;
    for (j = 0; j < room->n_conn; j++) {
      fprintf(file, "CONNECTION %d: %s\n", j + 1, 
        ROOM_NAMES[room->outbounds[j]]);
    }
    
    // write room type to file
    fprintf(file, "ROOM TYPE: %s\n\n", typeStr(room->type));
    */
  }

  //fclose(file);
  printf("files have been written\n");
}

/* ****************************************************************************
 * Description: prints information for rooms, used for debuggin
 * @param rooms
 * @param n: number of rooms
 * ***************************************************************************/
void printRooms(struct Room* rooms, int n) {
  int i;
  for (i = 0; i < n; i++) {
    const struct Room* room = &rooms[i];
    // print room name
    printf("ROOM NAME: %s\n", ROOM_NAMES[room->name]);
    
    // print connections
    int j;
    for (j = 0; j < room->n_conn; j++) {
      printf("CONNECTION %d: %s\n", j + 1, ROOM_NAMES[room->outbounds[j]]);
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
 * MAIN FUNCTION
 * ***************************************************************************/
int main() {
  srand(time(0));   // use current time to seed for random generator
  struct Room* rooms = initRooms(NUM_ROOMS);
  printRooms(rooms, NUM_ROOMS);  // debugging

  // create directory for rooms
  char dir[BUFFER];
  createDirectory(dir);
  createRoomFiles(rooms, NUM_ROOMS, dir);

  //free rooms
  free(rooms);
  return 0;
}


