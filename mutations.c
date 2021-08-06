/*
 *
 * Script to detect mutations. Written by Brian Chen for use by Wu Lab, Rowan University.
 * Version 3.0, last updated 7/22/2021
 * This script is not to be shared without express written permission of the script creator.
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<limits.h>

unsigned int m, n;
unsigned int perline = 72; //was going to attempt to put the newline character every 73 characters in outfile, but recursive is annoying and I don't want to have to rewrite the output to an iterative version...
char *current; //does this need to be global? I honestly don't remember...
FILE *outfile, *csvfile, *listfile;

void printoperations(unsigned int operations[m][n], char* identifier, char* reference);
int min(unsigned int diagonal, unsigned int left, unsigned int up);
void initialize(unsigned int cost[m][n], unsigned int operations[m][n]);
void editmatrices(unsigned int m, unsigned int n, char* reference, char* current, char* identifier);

/*for bug testing purposes only
void printoperationsmatrix(unsigned int operations[m][n]){
  unsigned int i, j;
  for(i = 0; i < m; i++){
    for(j = 0; j < n; j++){
      printf("%u ", operations[i][j]);
    }
    printf("\n");
  }
  return;
}
for bug testing purposes only*/

int main (int argc, char* argv[]) {

  //filepaths
  char *infilepath = argv[1];
  char *outfilepath = argv[2];
  if(argv[3]){
    perline = (unsigned int) strtoul(argv[3], NULL, 10);
  }

  /*Change these as necessary*/
  size_t big_size = 2000; //length of longest sequence
  size_t line_size = 200; //length of longest line (or file name/path) + ".csv\0"

  //some dynamic memory allocation...
  char *str = malloc(line_size * sizeof(char));
  char *reference = malloc(big_size * sizeof(char));
  current = malloc(big_size * sizeof(char));

  /*make csv file path based on outfilepath*/
  char *csvfilepath = malloc(line_size * sizeof(char));
  csvfilepath[0] = '\0';
  snprintf(csvfilepath, line_size, "%s", outfilepath);
  csvfilepath = strtok(csvfilepath, ".");
  strcat(csvfilepath, ".csv\0");

  csvfile = fopen(csvfilepath, "w");
  if (!csvfile) {
    printf("Error opening/creating '%s'.\n", csvfilepath);
    exit(0);
  }

  char *listfilepath = malloc(line_size * sizeof(char));
  listfilepath[0] = '\0';
  snprintf(listfilepath, line_size, "%s", csvfilepath);
  listfilepath = strtok(listfilepath, ".");
  strcat(listfilepath, "_list.csv");

  listfile = fopen(listfilepath, "w");
  if (!listfile) {
    printf("Error opening/creating '%s'.\n", listfilepath);
    exit(0);
  }

  free(csvfilepath);
  free(listfilepath);
  /*free the csvfilepath string since no longer needed*/

  //open some files for read/write
  FILE *infile = fopen(infilepath, "r");
  if (!infile) {
    printf("Error opening/creating '%s'.\n", infilepath);
    exit(0);
  }
  outfile = fopen(outfilepath, "w");
  if (!outfile) {
    printf("Error opening/creating '%s'.\n", outfilepath);
    exit(0);
  }

  //two bools to let the loop know when to start saving into current + start running the edit distance comparison
  bool first = true;
  bool second = true;

  int nchar;
  char *identifier = malloc(line_size * sizeof(char));

  unsigned int i, count = 0; //because I was too lazy
  while(nchar = getline(&str, &line_size, infile), nchar != -1){
    str[nchar-2] = '\0'; //not sure why, but the aligned .fa input file I was given requires nchar-2 instead of nchar-1

    if (str[0] == '>') { //check if is a header

      if (count == 1) {
        m = strlen(reference)+1;
        for (i = 0; i < m - 1; i++) {
          if (i < m-2) {
            fprintf(csvfile,"%c,", reference[i]); //print each char to csv with a comma.
          } else {
            fprintf(csvfile,"%c\n", reference[i]);
          }
        }
      }

      if (!first && !second) { //don't call this the first time we see a header

        n = strlen(current)+1;
        editmatrices(m, n, reference, current, identifier);

      }

      snprintf(identifier, line_size, "%s", str);
      fprintf(csvfile, "%s,", str); //print identifier to csv
      fprintf(outfile, "%s\n", str); //print header to outfile

      if (second) {

        fputs("0,",csvfile); //print that it obviously takes 0 edits to reach the reference sequence...
        m = strlen(reference)+1;
        second = false; //tells it to call edit matrices the third time we see a header

      }

      count++;

      if (count == 2) {
        first = false;
      }

      if (nchar = getline(&str, &line_size, infile), nchar != -1) {

        str[nchar-2] = '\0'; //not sure why, but the aligned .fa input file I was given requires nchar-2 instead of nchar-1

        if (!first) {

          current[0] = '\0';
          strcpy(current, str);

        } else {

          fprintf(outfile, "%s\n", str); //print the first sequence
          reference[0] = '\0';
          strcpy(reference, str);

        }

      } else {

        printf("You only had a header and nothing else...?\n");

      }

    } else {

      if (!first) {

        strcat(current, str);

      } else {

        fprintf(outfile, "%s\n", str); //print the first sequence
        strcat(reference, str);

      }

    }

  }


  //call this one more time for the last one
  n = strlen(current)+1;
  editmatrices(m, n, reference, current, identifier);

  free(identifier);
  free(str);
  free(reference);
  free(current);
  fclose(infile);
  fclose(outfile);
  fclose(csvfile);
  fclose(listfile);

  return 1;

}


/*
 *
 * METHODS
 *
 */


void printoperations(unsigned int operations[m][n], char* identifier, char* reference) { //iterative
  unsigned int i, j, k = 0, x = 0, z = 0;
  //create an array of n*2 size that is full of 4s
  unsigned int lensum = n+m; //matrix traceback is at most size n+m
  unsigned int array[lensum];
  for(i = 0; i<lensum; i++){
    array[i] = 4;
  }

  //fill array such that it is [4,4,4,4,4,4...4,4,4,3,2,1,0,2,3,0,1,3,2...,3] (fours are not real operators)
  i = m-1;
  j = n-1;
  while(i != 0 && j != 0){
    k++;
    switch(operations[i][j]){
      case 0:
        array[lensum-k] = 0;
        i--;
        j--;
        break;
      case 1:
        array[lensum-k] = 1;
        j--;
        break;
      case 2:
        array[lensum-k] = 2;
        i--;
        break;
      case 3:
        array[lensum-k] = 3;
        i--;
        j--;
        break;
      default:
        printf("Operation not recognized. Returned value is %u at i=%u, j=%u.\n", operations[i][j], i, j);
        //printoperationsmatrix(operations);
        array[lensum-k] = 5;
        i = 0;
        j = 0;
        break;
    }
  }

  k = lensum - k; //this should be the first index that is not a four

  j = 0;

  for(i = k; i<lensum; i++){
    if(j > 0 && j % perline == 0){
      fputs("\n", outfile);
    }
    j++;
    switch(array[i]){
      case 0:
        //printf("mismatch\t");
        fprintf(listfile, "%s,%c%u%c\n", identifier, reference[z], z, current[x]);
        fprintf(outfile, "%c", current[x]); //print to outfile
        if (x < n-2){
          fprintf(csvfile, "%c,", current[x]); //print to csv
        } else {
          fprintf(csvfile, "%c\n", current[x]);
        }
        x++;
        ++z;
        break;
      case 1:
        //printf("insertion\t");
        fprintf(outfile, "%c", current[x]); //print to outfile
        if (x < n-2){
          fprintf(csvfile, "%c,", current[x]); //print to csv
        } else {
          fprintf(csvfile, "%c\n", current[x]);
        }
        x++;
        break;
      case 2:
        //printf("deletion\t");
        //it was specifically requested that we pad the deletions with "-"
        fputs("-", outfile); //print to outfile
        //fputs("-,", csvfile); we don't pad the csv right??
        break;
      case 3:
        //printf("copy\t");
        fputs("-", outfile); //print to outfile
        if (x < n-2){
          fputs("-,", csvfile); //print to csv
        } else {
          fputs("-\n", csvfile); //print to csv
        }
        x++;
        ++z;
        break;
      default:
        printf("Value stored in array[%u]: %u is not recognized. 4 indicates index issues, 5 invalid operation, everything else is unknown.\n", i, array[i]);
        break;
    }
  }
}

int min(unsigned int diagonal, unsigned int left, unsigned int up) {
  unsigned int arr[3] = {diagonal, left, up};
  unsigned int min = UINT_MAX;
  int index = -1;

  for(unsigned int i = 0; i<3; i++){
    if (arr[i] < min){
      min = arr[i];
      index = i;
    }
  }
  return index;
}

void initialize(unsigned int cost[m][n], unsigned int operations[m][n]) {
  unsigned int i;
  for(i=0;i<m;i++){
    cost[i][0] = i;
    operations[i][0] = 1;
  }
  for(i=0;i<n;i++){
    cost[0][i] = i;
    operations[0][i] = 2;
  }
  operations[0][0] = 3;
}

void editmatrices(unsigned int m, unsigned int n, char* reference, char* current, char* identifier) {
  unsigned int i, j;
  /*
  unsigned int cost[m][n];
  unsigned int operations[m][n];
  */
  //allocate on the heap
  unsigned int (*cost)[n] = malloc(sizeof(unsigned int[m][n]));
  unsigned int (*operations)[n] = malloc(sizeof(unsigned int[m][n]));

  initialize(cost, operations);
  for(i=1; i<m; i++){
    for(j=1; j<n; j++){
      if(reference[i-1] == current[j-1]){ //copy
        cost[i][j] = cost[i-1][j-1];
        operations[i][j] = 3; //this is 3 because I forgot =.-
      } else {
        switch(min(cost[i-1][j-1],cost[i][j-1],cost[i-1][j])){
          case 0: //mismatch
            cost[i][j] = cost[i-1][j-1] + 1;
            operations[i][j] = 0;
            break;
          case 1: //left
            cost[i][j] = cost[i][j-1] + 1;
            operations[i][j] = 1;
            break;
          case 2:
            cost[i][j] = cost[i-1][j] +1;
            operations[i][j] = 2;
            break;
          default:
            printf("Somehow the min() function returned an invalid index.\n");
            return;
        }
      }
    }
  }
  fprintf(csvfile,"%u,", cost[m-1][n-1]); //print edit distance to csv (note this includes in/dels and not just mismatches)
  printoperations(operations, identifier, reference);
  fputs("\n", outfile);
  free(cost);
  free(operations);
}
