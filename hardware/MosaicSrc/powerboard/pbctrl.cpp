/* Power Board test program */

#include "pbif.h"
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define BOARD_NAME_LEN 128

typedef struct options_s {
  char  board[BOARD_NAME_LEN];
  bool  readState;
  int   IthN;
  float IthVal;
  float Vbias;
  int   VoutN;
  float VoutVal;
  int   storeVout;
  bool  storeAllVout;
  bool  restoreAllVout;
  bool  on;
  bool  off;
  int   switchCh;
} options_t;
options_t OPTIONS;

void dump(unsigned char *buffer, int size)
{
  int i, j;

  for (i = 0; i < size;) {
    for (j = 0; j < 16; j++) {
      printf(" %02x", buffer[i]);
      i++;
    }
    printf("\n");
  }
}

void print_help()
{
  printf("pbctrl [options] board_address\n"
         "\n"
         "Options list:\n"
         "\t -state\n"
         "\t -Ith <channel> <value[A]>\n"
         "\t -Vbias <value[V]> - Note: MUST be negative\n"
         "\t -Vout <channel> <value[V]>\n"
         "\t -store <channel>\n"
         "\t -storeall\n"
         "\t -restoreall\n"
         "\t -on\n"
         "\t -off\n"
         "\t -onch <channel>\n"
         "\t -offch <channel>\n"
         "\n");
}

/*
        Read options from command line
        return -1 if error, 0 if OK
*/
int readopt(int argc, char *argv[])
{
  int pc = 1;

  if (argc < 3) return -1;

  errno = 0; //  needed for strto* functions

  OPTIONS.readState      = false;
  OPTIONS.IthN           = -1;
  OPTIONS.Vbias          = 1;
  OPTIONS.VoutN          = -1;
  OPTIONS.storeVout      = -1;
  OPTIONS.storeAllVout   = false;
  OPTIONS.restoreAllVout = false;
  OPTIONS.on             = false;
  OPTIONS.off            = false;
  OPTIONS.switchCh       = -1;

  while (pc < argc) {
    if (strcmp(argv[pc], "-state") == 0) {
      OPTIONS.readState = true;
    }
    else if (strcmp(argv[pc], "-Ith") == 0) {
      if (pc >= (argc - 2)) return -1;
      OPTIONS.IthN   = strtol(argv[++pc], NULL, 10);
      OPTIONS.IthVal = strtof(argv[++pc], NULL);
      if (OPTIONS.IthN < 0 || OPTIONS.IthN > 15 || OPTIONS.IthVal < 0) return -1;
    }
    else if (strcmp(argv[pc], "-Vbias") == 0) {
      if (pc >= (argc - 1)) return -1;
      OPTIONS.Vbias = strtof(argv[++pc], NULL);
      if (OPTIONS.Vbias > 0) return -1;
    }
    else if (strcmp(argv[pc], "-Vout") == 0) {
      if (pc >= (argc - 2)) return -1;
      OPTIONS.VoutN   = strtol(argv[++pc], NULL, 10);
      OPTIONS.VoutVal = strtof(argv[++pc], NULL);
      if (OPTIONS.VoutN < 0 || OPTIONS.VoutN > 15 || OPTIONS.VoutVal < 0) return -1;
    }
    else if (strcmp(argv[pc], "-store") == 0) {
      if (pc >= (argc - 1)) return -1;
      OPTIONS.storeVout = strtol(argv[++pc], NULL, 10);
      if (OPTIONS.storeVout < 0) return -1;
    }
    else if (strcmp(argv[pc], "-storeall") == 0) {
      OPTIONS.storeAllVout = true;
    }
    else if (strcmp(argv[pc], "-restoreall") == 0) {
      OPTIONS.restoreAllVout = true;
    }
    else if (strcmp(argv[pc], "-on") == 0) {
      OPTIONS.on = true;
    }
    else if (strcmp(argv[pc], "-off") == 0) {
      OPTIONS.off = true;
    }
    else if (strcmp(argv[pc], "-onch") == 0) {
      OPTIONS.on = true;
      if (pc >= (argc - 1)) return -1;
      OPTIONS.switchCh = strtol(argv[++pc], NULL, 10);
      if (OPTIONS.switchCh < 0) return -1;
    }
    else if (strcmp(argv[pc], "-offch") == 0) {
      OPTIONS.off = true;
      if (pc >= (argc - 1)) return -1;
      OPTIONS.switchCh = strtol(argv[++pc], NULL, 10);
      if (OPTIONS.switchCh < 0) return -1;
    }
    else {
      break;
    }
    pc++;
  }
  if (pc >= argc || argv[pc][0] == '-') return -1;

  strncpy(OPTIONS.board, argv[pc], BOARD_NAME_LEN - 1);

  return 0;
}

void printState(powerboard::pbstate_t *pbStat)
{
  printf("\nPower board state:\n");

  printf("T:%5.1f ", pbStat->T);
  printf("\n");

  for (int i = 0; i < 16; i++) {
    printf("CH%02d:%s ", i, (pbStat->chOn & 1 << i) ? "ON " : "Off");
    printf("Vset:%4.2f ", pbStat->Vout[i]);
    printf("Vmon:%4.2f ", pbStat->Vmon[i]);
    printf("Imon:%5.3f\n", pbStat->Imon[i]);
  }
  printf("VmonBias: %4.2f ImonBias: %4.2f\n", pbStat->Vbias, pbStat->Ibias);
  printf("Bias status:");
  for (int i = 0; i < 8; i++)
    printf(" %d:%s", i, (pbStat->biasOn & 1 << i) ? "ON " : "Off");

  printf("\n\n");
}

int main(int argc, char **argv)
{
  powerboard::pbstate pbStat;

  if (readopt(argc, argv) < 0) {
    print_help();
    exit(0);
  }

  try {
    PBif *      board = new PBif(OPTIONS.board);
    powerboard *pb    = board->pb;

    // check board connection
    if (!pb->isReady()) {
      printf("Power board unconnected or off\n");
      exit(0);
    }

    if (OPTIONS.IthN >= 0) pb->setIth(OPTIONS.IthN, OPTIONS.IthVal);
    if (OPTIONS.Vbias <= 0) pb->setVbias(OPTIONS.Vbias);
    if (OPTIONS.VoutN >= 0) pb->setVout(OPTIONS.VoutN, OPTIONS.VoutVal);
    if (OPTIONS.storeVout >= 0) pb->storeVout(OPTIONS.storeVout);
    if (OPTIONS.storeAllVout) pb->storeAllVout();
    if (OPTIONS.restoreAllVout) pb->restoreAllVout();
    if (OPTIONS.on) {
      if (OPTIONS.switchCh == -1) {
        pb->onAllVout();
        pb->onAllVbias();
      }
      else {
        pb->onVout(OPTIONS.switchCh);
      }
    }
    if (OPTIONS.off) {
      if (OPTIONS.switchCh == -1) {
        pb->offAllVout();
        pb->offAllVbias();
      }
      else {
        pb->offVout(OPTIONS.switchCh);
      }
    }
    if (OPTIONS.readState) {
      pb->startADC();
      // wait 100 ms to get first convertion
      usleep(100000);

      pb->getState(&pbStat);
      printState(&pbStat);
    }

    delete board;
  }
  catch (std::exception &e) {
    cout << e.what() << endl;
  }

  return 0;
}
