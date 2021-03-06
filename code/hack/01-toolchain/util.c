#include "util.h"

int isDebug = 0;

void argHandle(int argc, char *argv[], int argMin, char *msg) {
  if (argc < argMin) {
    printf(msg);
    exit(1);
  }
  for (int i=0; i<argc; i++) {
    if (eq(argv[i], "-d")) isDebug = 1;
  }
}

int readText(char *fileName, char *text, int size) {
  FILE *file = fopen(fileName, "r");
  int len = fread(text, 1, size, file);
  text[len] = '\0';
  fclose(file);
  return len;
}

// ================= Binary ==================
static char* h2b[] = {
  "0000", "0001", "0010", "0011",
  "0100", "0101", "0110", "0111", 
  "1000", "1001", "1010", "1011",
  "1100", "1101", "1110", "1111"};

static char hexDigits[] = "0123456789ABCDEF";

char *nextToken(char *str, char *spliter, char *sp) {
  char *p = str;
  while (!strchr(spliter, *p)) p++;
  *sp = *p;
  *p = '\0';
  return str;
}

void hexDump16(uint16_t *words, int len) {
  for (int i=0; i<len; i++) {
    printf("%04X", words[i]);
  }
}
/*
int error(char *msg) {
  printf("Error: %s", msg);
  assert(0);
}
*/
void replace(char *str, char *set, char t) {
  for (char *p = str; *p!= '\0'; p++) {
    if (strchr(set, *p) != NULL) *p = t;
  }
}

void htob(char* hex, char* binary) {
  for (int i=0; hex[i] != '\0'; i++) {
    char *ptr = strchr(hexDigits, hex[i]);
    assert(ptr != NULL);
    char h = ptr - hexDigits;
    sprintf(&binary[4*i], "%s", h2b[h]);
  }
}

void itob(int i, char* binary) {
  char hex[100];
  sprintf(hex, "%04X", i);
  htob(hex, binary);
}

int btoi(char* binary) {
  int result = 0;
  for (char *p = binary; *p != '\0'; p++) {
    result <<= 1;
    if (*p == '1') result += 1;
  }
  return result;
}