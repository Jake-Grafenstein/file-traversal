/*
* name: Jacob Grafenstein
* x500: grafe014
* CSELabs machine: CSEL-KH4250-47
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "codec.h"
#include "codec.c"

#define MAX_FILES 64
#define MAX_PATH 4096
#define MAX_LENGTH 1024
#define ENCODE_CHAR 3
#define DECODE_CHAR 4
#define CHAR_SIZE 1
#define HARD_LINK 1
#define NOT_HARD_LINK 0

// My encoder function takes in the inputDirectory, outputDirectory and and filename buffers, and an integer corresponding to the size of my input file. I create my filepaths and open my input and output files. I loop until I reach the end of file, using fread to read 3 characters from my input file. If I read less than 3 characters, I append 0's to the buffer. Then I encode the block using the encode block API only encoding the exact number of characters read from the input file. I then write those encoded characters to my output file. when end of file is reached, you free the memory and return the filepath of the output file.
char * encoder(const char *inputDirectory, const char *outputDirectory, char *fileName, int size) {
  FILE *inputFile, *outputFile;
  char *createInput = malloc(MAX_PATH*sizeof(char));
  char *createOutput = malloc(MAX_PATH*sizeof(char));
  uint8_t *inputBuffer = malloc(ENCODE_CHAR*sizeof(char));
  uint8_t *outputBuffer = malloc(DECODE_CHAR*sizeof(uint8_t));
  size_t myRead, myWrite;
  uint8_t ch = 0;

  // Create input to fopen
  strcpy(createInput, inputDirectory);
  strcat(createInput, "/");
  strcat(createInput, fileName);

  // Create output to fopen
  strcpy(createOutput, outputDirectory);
  strcat(createOutput, "/");
  strcat(createOutput, fileName);

  // Open the file to be encoded with read permissions
  inputFile = fopen((const char *) createInput, "r");
  if (inputFile == NULL) {
    fprintf(stderr, "Can't open input file in.list!\n");
    exit(1);
  }
  free(createInput);

  // Open the file to be write encoded file to  with write permissions
  outputFile = fopen((const char *) createOutput, "w");
  if (outputFile == NULL) {
    fprintf(stderr, "Can't open output file in.list!\n");
    exit(1);
  }

  // Loop until you've reached the end of your input file
  while (!feof(inputFile)) {

    // Read 3 characters from the file
    myRead = fread(inputBuffer, CHAR_SIZE, ENCODE_CHAR, inputFile);
    // Add zeros to the end of inputBuffer if it has less than 3
    if (myRead < ENCODE_CHAR) {
        if (myRead == 0) {
          break;
        } else if (myRead == 1) {
          inputBuffer[myRead] = ch;
          inputBuffer[myRead+1] = ch;
        } else if (myRead == 2) {
          inputBuffer[myRead] = ch;
          char c = is_valid_char(ch);
        }
      }

    // Call encode_block and store the result in outputBuffer
    myWrite = encode_block(inputBuffer, outputBuffer, myRead);
    fprintf(outputFile, "%s", outputBuffer);

  }
  // Free my memory
  free(inputBuffer);
  free(outputBuffer);

  // Write a newline to the end of the file
  uint8_t ah = 0x0a;
  fputc(ah, outputFile);
  fclose(outputFile);
  fclose(inputFile);

  return createOutput;
}

// My decoder function takes in the inputDirectory, outputDirectory, fileName buffers and and integer designating the size of my file. First I create the filepaths for my files and open them. Then I loop until I reach the end of my file, checking each individual char for its validity. If it is valid, I add it to my buffer. When I've found 4 valid characters, I decode using the decode API and write the characters to a buffer. when I've reached the end of the file, I copy my buffer to my output file and return the file path of the output file.
char * decoder(const char *inputDirectory, const char *outputDirectory, char *fileName, int size) {
  FILE *inputFile, *outputFile;
  int counter, validChar;
  uint8_t c;
  char line[MAX_PATH];
  char *createInput = malloc(MAX_PATH*sizeof(char));
  char *createOutput = malloc(MAX_PATH*sizeof(char));
  uint8_t *outputBuffer = malloc(ENCODE_CHAR*sizeof(uint8_t));
  uint8_t *inputBuffer = malloc(DECODE_CHAR*sizeof(uint8_t));
  uint8_t ch = 0x0a;
  size_t myRead, myWrite;

  // Create my input filepath to fopen
  strcpy(createInput, inputDirectory);
  strcat(createInput, "/");
  strcat(createInput, fileName);

  // create my output filepath to fopen
  strcpy(createOutput, outputDirectory);
  strcat(createOutput, "/");
  strcat(createOutput, fileName);

  // Open my input file, check if null
  inputFile = fopen((const char *) createInput, "r");
  if (inputFile == NULL) {
    fprintf(stderr, "fopen() failed for input: %s\n", strerror(errno));
  }
  free(createInput);

  // Opne my output file, check if null
  outputFile = fopen((const char *) createOutput, "w");
  if (outputFile == NULL) {
    fprintf(stderr, "fopen() failed for output: %s\n", strerror(errno));
  }

  // If the size of my file is 0, I don't need to do anything more.
  counter = 0;
  if (size == 0) {
    fclose(inputFile);
    fclose(outputFile);
    return createOutput;
  }

  // Loop until you reach the end of the input file
  while (!feof(inputFile)) {

    // Get each individual character, check if it's valid, and if so, add it to my inputBuffer. Stop when you've found 4 valid characters.
    c = fgetc(inputFile);
    validChar = is_valid_char(c);
    if (validChar == 1) {
      inputBuffer[counter] = c;
      counter++;
    } else {
      // Do Nothing
    }
    // When you've found four valid characters, call decode block on the inputbuffer and save it to the outputbuffer. Print the string to the output file.
    if (counter == 4) {
      myWrite = decode_block(inputBuffer, outputBuffer);
      fprintf(outputFile, "%s", outputBuffer);
      counter = 0;
    }
  }
  // Free my memory
  free(inputBuffer);
  free(outputBuffer);

  // Close your open files.
  fclose(inputFile);
  fclose(outputFile);
  return createOutput;
}

// My organizer function is where the bulk of the work is done. It opens my working directory, and begins looping through the contents. If it encounters a directory, it adds it to the report file (unsorted), and recursively calls the organizer function with itself as the input so that you can loop through that directory's contents. If it encounters a regular file, it first checks if the file is a hard link. If the hard links' iNode has not been processed yet, it handles it like it would any regular file; else, it simply adds a record of the hard link in the report file. If the file is not a hard link, I check to see if we are encoding or decoding, call the appropriate function, add a record of the file in the report file (unsorted). Lastly, I check if the file is a soft link and add a record of the soft link in the report file (unsorted) if so. Add a null terminator to the report file, free the memory, and returns.
void organizer(const char *input, const char *output, const char *codec, FILE *report, int *completedINodes, int iNodeCounter) {
  DIR *workingDirectory;
  struct dirent *myRead;
  struct stat inputFileStat, outputFileStat;
  int counter, oldSize, newSize, i, flag;
  char *oldSizeChar = malloc(MAX_LENGTH*sizeof(char));
  char *newSizeChar = malloc(MAX_LENGTH*sizeof(char));
  char *entry = malloc(MAX_PATH*sizeof(char));
  char *inputFilePath = malloc(MAX_PATH*sizeof(char));
  char *inputDirectoryName = malloc(MAX_PATH*sizeof(char));
  char *outputDirectoryName = malloc(MAX_PATH*sizeof(char));
  char *newInput = malloc(MAX_PATH*sizeof(char));
  char *newOutput = malloc(MAX_PATH*sizeof(char));
  mode_t myMode;
  unsigned char fileType;

 // Open working directory
  workingDirectory = opendir(input);
  if (workingDirectory == NULL) {
    perror("Could not open given directory.");
    exit(1);
  }

  // Looping through the director using readdir until you've reached NULL
  while ((myRead = readdir(workingDirectory)) != NULL) {

    // Check if the entry is the designator for cwd, the parent directory, or a text file/directory
    if (!strcmp(myRead->d_name,".")) {
      //printf("We found the designator for the current working directory.\n");
    } else if (!strcmp(myRead->d_name,"..")) {
      //printf("We found the designator for the parent directory.\n");
    } else {
      strcpy(newInput, input);
      strcpy(inputDirectoryName, input);
      strcat(inputDirectoryName, "/");
      strcat(inputDirectoryName, myRead->d_name);

      // Get the real path of the input file so you can call lstat
      realpath(inputDirectoryName, inputFilePath);
      int didItWork = lstat((const char *) inputFilePath, &inputFileStat);
      if (didItWork == -1) {
        perror("Could not get statistics about the file.");
        exit(1);
      }
      myMode = inputFileStat.st_mode;
      fileType = myRead->d_type;
      // Check if we're looking at a directory, a regular file, or a soft link
      if (fileType == DT_LNK) {
        // Add the entry to the report file (unsorted)
        strcpy(entry, myRead->d_name);
        strcat(entry, ", sym link, 0, 0\n");
        fprintf(report, "%s", entry);
      }  else if (fileType == DT_REG) {
        // Check if the file is a hard link
        if (inputFileStat.st_nlink > 1) {
          // Check if first pass of hard link
          for (i = 0; i < iNodeCounter; i++) {
            if (inputFileStat.st_ino == completedINodes[i]) {
              // Add to report file (unsorted)
              strcpy(entry, myRead->d_name);
              strcat(entry, ", hard link, 0, 0\n");
              fprintf(report, "%s", entry);
              flag = HARD_LINK;
              break;
            }
            flag = NOT_HARD_LINK;
          }
          if (flag == NOT_HARD_LINK) {
            // Get the size of the file, store it as a character, then call my encoding function
            oldSize = inputFileStat.st_size;
            sprintf(oldSizeChar, "%d", oldSize);

            // Determine if we should encode or decode the file and call the appropriate function
            if (!strcmp(codec, "-e")) {
              outputDirectoryName = encoder(newInput, output, myRead->d_name, oldSize);
            } else if (!strcmp(codec, "-d")) {
              outputDirectoryName = decoder(newInput, output, myRead->d_name, oldSize);
            } else {
              perror("Invalid command line operation, cannot understand\n");
            }

            // Get the file statistics for the newly created encoded file
            realpath(outputDirectoryName, inputFilePath);
            //printf("This is the file I'm trying to open: %s\n", inputFilePath);
            int didItWork = lstat((const char *) inputFilePath, &outputFileStat);
            if (didItWork == -1) {
              perror("This is where I am: Could not get statistics about the file.");
              exit(1);
            }

            // Get the size of the file, store it as a character, then add the entry to the report file (unsorted)
            newSize = outputFileStat.st_size;
            //printf("This is my new file size: %d\n", newSize);
            sprintf(newSizeChar, "%d", newSize);
            strcpy(entry, myRead->d_name);
            strcat(entry, ", regular file, ");
            strcat(entry, oldSizeChar);
            strcat(entry, ", ");
            strcat(entry, newSizeChar);
            strcat(entry, "\n");
            fprintf(report, "%s", entry);

            // Add to list of completedINodes
            completedINodes[iNodeCounter] = inputFileStat.st_ino;
            iNodeCounter++;
          }
        } else {
          // Get the size of the file, store it as a character, then call my encoding function
          oldSize = inputFileStat.st_size;
          sprintf(oldSizeChar, "%d", oldSize);

          // Determine if we should encode or decode the file and call the appropriate function
          if (!strcmp(codec, "-e")) {
            outputDirectoryName = encoder(newInput, output, myRead->d_name, oldSize);
          } else if (!strcmp(codec, "-d")) {
            outputDirectoryName = decoder(newInput, output, myRead->d_name, oldSize);
          } else {
            perror("Invalid command line operation, cannot understand\n");
          }

          // Get the file statistics for the newly created encoded file
          realpath(outputDirectoryName, inputFilePath);
          int didItWork = lstat((const char *) inputFilePath, &outputFileStat);
          if (didItWork == -1) {
            perror("Could not get statistics about the file.");
            exit(1);
          }

          // Get the size of the file, store it as a character, then add the entry to the report file (unsorted)
          newSize = outputFileStat.st_size;
          //printf("This is my new file size: %d\n", newSize);
          sprintf(newSizeChar, "%d", newSize);
          strcpy(entry, myRead->d_name);
          strcat(entry, ", regular file, ");
          strcat(entry, oldSizeChar);
          strcat(entry, ", ");
          strcat(entry, newSizeChar);
          strcat(entry, "\n");
          fprintf(report, "%s", entry);

          // Add iNode to completedINodes
          completedINodes[iNodeCounter] = inputFileStat.st_ino;
          iNodeCounter++;
        }
      } else if (fileType == DT_DIR) {
        // Add entry to the report file (unsorted)
        strcpy(entry, myRead->d_name);
        strcat(entry, ", directory, 0 ,0\n");
        fprintf(report, "%s", entry);

        // make a new directory in the output folder at the required depth
        strcpy(newOutput, output);
        strcat(newOutput, "/");
        strcat(newOutput, myRead->d_name);
        strcat(newOutput, "\0");
        int retval = mkdir(newOutput, 0755);
        /* If retval is 0, then directory was created successfully */
        if (retval == -1) {
          fprintf(stderr, "mkdir() failed: %s\n", strerror(errno));
          exit(EXIT_FAILURE);
        }

        // Add iNode to completedINodes
        completedINodes[iNodeCounter] = inputFileStat.st_ino;
        iNodeCounter++;
        // Recursively call my organizer function on the directory, to analyze its contents
        organizer((const char *) inputDirectoryName, newOutput, codec, report, completedINodes, iNodeCounter);
      }
    }
  }
  // Add a null terminator to the report file
  fprintf(report, "%s", "\0");


  // Free my memory
  free(oldSizeChar);
  free(newSizeChar);
  free(entry);
  free(inputFilePath);
  free(inputDirectoryName);
  free(newInput);
  free(newOutput);

  return;
}

// Function taken from University of North Texas, College of Science and Engineering: http://www.cse.unt.edu/~donr/courses/4410/NOTES/qsortExamples.html
// cmp function takes in to char double pointers and then using strcmp to compare the two single pointers to my strings
int cmp (char **str1, char **str2) {
  return strcmp(*str1,*str2);
}

// Note: usage of qsort taken from University of North Texas, College of Science and Engineering: http://www.cse.unt.edu/~donr/courses/4410/NOTES/qsortExamples.html
// sortFile takes in the report file that we originally stored the unsorted entries in, and the filePath of the report file. I dynamically allocate a 2D array on the stack for holding the strings. Then, I use fgets to store each line of the report file in the 2D array and use qsort to alphabetically sort the 2D array. I close the report file and the reopen it with write permissions, thus wiping the file and allowing me to rewrite the sorted lines into it. I finally free the allocated memory.
FILE *sortFile(FILE *report, const char *filePath) {
  char line[MAX_PATH];
  char **lineArray = (char **) malloc(MAX_FILES*sizeof(char *));
  int counter, elems, i;

  // Allocates memory for each string
  for (i = 0; i < MAX_FILES; i++) {
    lineArray[i] = malloc(MAX_LENGTH*sizeof(char));
  }

  // Adds line to the  line array
  elems = 0;
  for (counter = 0; counter < MAX_FILES; counter++) {
    if (fgets(line, sizeof(line), report) != NULL) {
      //printf("This is my line: %s", line);
      strcpy(lineArray[counter],line);
      elems++;
    }
  }

  // Calls to qsort to sort the elements of the line array
  qsort(lineArray, elems, sizeof(char *), (int (*)(const void *, const void *)) cmp);

  // Close my report file and then reopen it with write permissions
  fclose(report);
  report = fopen(filePath, "w");
  if (!report) {
    perror("fopen was not able to open the file with write permissions.");
  }
  if (report == 0) {
    perror("Cannot open file");
    exit(1);
  }

  // Use fprintf to write the strings from the line array to my report file
  for (i = 0; i < elems; i++) {
    fprintf(report, "%s", lineArray[i]);
  }

  // Free the memory
  for (i = 0; i < MAX_FILES; i++)
  {
    char* currentIntPtr = lineArray[i];
    free(currentIntPtr);
  }
  free(lineArray);

  return report;
}

// My main function checks to make sure there are a correct number of arguments, creates a folder within the output directory, creates a report file and opens it with write permissions, calls my organizer function, closes the report file and reopens it with read privileges, and calls the sort function to sort the entries in the report file. It returns 0.
int main(int argc, const char *argv[]) {
  FILE *report;
  int success, i;
  const char *input = argv[2];
  const char *output = argv[3];
  char *newOutput = malloc(MAX_PATH*sizeof(char));
  char *reportPath = malloc(MAX_LENGTH*sizeof(char));
  char *reportName = malloc(MAX_LENGTH*sizeof(char));
  int *completedINodes = malloc(MAX_FILES*sizeof(int));
  int iNodeCounter = 0;

  // Check if correct number of arguments
  if (argc != 4) {
    perror("Invalid number of command line arguments.");
    exit(1);
  }

  // Create my folder within the output directory
  char const *codec = argv[1];
  strcpy(newOutput, output);
  strcat(newOutput, "/");
  strcat(newOutput, input);
  success = mkdir(newOutput, 0755);
  if (success == -1) {
    perror("Failed to make directory.\n");
    exit(1);
  }

  // Create my report file
  strcpy(reportName, input);
  strcat(reportName, "_report.txt\0");
  strcpy(reportPath, output);
  strcat(reportPath, "/");
  strcat(reportPath, reportName);
  report = fopen((const char *) reportPath, "w");
  if (!report) {
    perror("fopen");
  }
  if (report == 0) {
    perror("Cannot open file\n");
    exit(1);
  }

  // Call my organizer function
  organizer(input, newOutput, codec, report, completedINodes, iNodeCounter);

  // Close my report file and reopen it with read permissions and feed it to the sort function
  fclose(report);
  report = fopen((const char *) reportPath, "r");
  sortFile(report, (const char *) reportPath);

  // Free my memory
  free(reportPath);
  free(reportName);

  return 0;
}
