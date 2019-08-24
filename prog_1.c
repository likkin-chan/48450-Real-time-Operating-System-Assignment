/***********************************************************************************/
/* Project: Semaphore and Pipe for real time file reading/writing
 *
 * Purpose: Take the text file "data.txt" as input and separate its file header and 
 *			    content, then write the content into a new text file "src.txt" 
 *
 * Author: 	Lijian Chen
 */
/***********************************************************************************/

/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc prog_1.c -o prog_1 -lpthread -lrt

*/
#include  <pthread.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <semaphore.h>
#include  <sys/time.h>

// Define the message size
#define MESSAGE_SIZE 255
// Define the end of header
#define EOH "end_header"

/* --- Structs --- */
typedef struct ThreadParams {
  int pipeFile[2];
  sem_t sem_read, sem_justify, sem_write;
  char message[255];
  pthread_mutex_t lock;
  int end_of_file; // End of file flag - Set 1 if it is the end of the file
} ThreadParams;

/* --- Prototypes --- */
/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void* ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void* ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void* ThreadC(void *params);

/* --- Global Variable --- */
pthread_t tid[3];

/* --- Main Code --- */
int main(int argc, char const *argv[]) {
  struct timeval t1; // t2 - Comment out the unused variable t2
  gettimeofday(&t1, NULL);  // Start Timer
  int err;
  pthread_attr_t attr;
  ThreadParams params;

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);
  
  // Check the pipe validation
  if(pipe(params.pipeFile) < 0)
  {
    perror("Error opening pipe");
    exit(0); 
  }

  // Create Threads
  if(err = pthread_create(&(tid[0]), &attr, &ThreadA, (void*)(&params)))
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  // Create Thread B
  if(err = pthread_create(&(tid[1]), &attr, &ThreadB, (void*)(&params)))
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  // Create Thread C
  if(err = pthread_create(&(tid[2]), &attr, &ThreadC, (void*)(&params)))
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  
  // Wait on threads to finish
  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);
  
  // Closes all the pipes
  close(params.pipeFile[0]);
  close(params.pipeFile[1]);

  // Kill the threads execution
  pthread_cancel(tid[0]);
  pthread_cancel(tid[1]);
  pthread_cancel(tid[2]);

  return 0;
}

/* --- Data Initialization --- */
void initializeData(ThreadParams *params) {
  // Initialize Sempahores
  sem_init(&(params->sem_read), 0, 1);
  sem_init(&(params->sem_write), 0, 0);
  sem_init(&(params->sem_justify), 0, 0);
  
  // Initialize the value of end_of_file to 0
  params->end_of_file = 0;
}

/* --- Thread A Implementation --- */
void* ThreadA(void *params) {
  ThreadParams *TP = params;
  
  // Store the data.txt into the local variable filename
  static const char file[] = "data.txt";
  
  // Open the file to read
  FILE *reader = fopen(file, "r");
  // Check the file validation
  if(!reader)
  {
    perror(file);
    exit(0);
  }

  // Wait until the read semaphore finish
  while(!sem_wait(&(TP->sem_read)))
  {
    if(fgets(TP->message, MESSAGE_SIZE, reader) == NULL)
    {
      // Set the end_of_file flag to 1 if it has reach the end of file
      TP->end_of_file = 1;	
      sem_post(&(TP->sem_write));
      break;
    }
    // Write whatever is in the reader into the writing pipe
    write(TP->pipeFile[1], TP->message, strlen(TP->message)+1);
    // Signal the write semaphore
    sem_post(&(TP->sem_write));
  }

  // Close the writing pipe
  close(TP->pipeFile[1]);
  // Close the file
  fclose(reader);

  return NULL;
}

/* --- Thread B Implementation --- */
void* ThreadB(void *params) {
  ThreadParams *TP = params;

  // Wait until the write semaphore finish
  while(!sem_wait(&(TP->sem_write)))
  {
    // Read from the reading pipe into the message container
    read(TP->pipeFile[0], TP->message, MESSAGE_SIZE);
    // Signal the justify semaphore
    sem_post(&(TP->sem_justify));
   
    if(TP->end_of_file == 1)
    {
      break;
    }
  }

  // Close the reading pipe
  close(TP->pipeFile[0]);

  return NULL;
}

/* --- Thread C Implementation --- */
void* ThreadC(void *params) {
  ThreadParams *TP = params;
  
  // Initialize the line counter
  int count_lines = 0;
  // Initialize the end of file flag
  int eoh_flag = 0;
  
  // Store the src.txt into the local variable filename
  static const char file[] = "src.txt";

  // Open the file to write
  FILE *writer = fopen(file, "w");
  // Check the file validation
  if(!writer)
  {
    perror(file);
    exit(0);
  }
  
  // Wait until the justify semaphore finish
  while(!sem_wait(&(TP->sem_justify)))
  {
    if(eoh_flag)
    {
      // Put the data in the message container into writer file
      fputs(TP->message, writer);
      // Increment the line counter
      count_lines++;
    
      if(TP->end_of_file == 1)
      {
        break;
      }
    }
    else if(strstr(TP->message, EOH))
    {
      eoh_flag = 1;
    }
    // Signal the read semaphore
    sem_post(&(TP->sem_read));
  }

  // Close the file
  fclose(writer);

  return NULL;
}
