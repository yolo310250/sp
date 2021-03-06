#include "asm.h"

Pair dList[] = {
  {"", "000"}, {"M", "001"}, {"D", "010"}, {"MD", "011"},
  {"A","100"}, {"AM","101"}, {"AD","110"}, {"AMD","111"}
};

Pair cList[] = {
  {"0",   "101010"}, {"1",   "111111"}, {"-1",  "111010"},
  {"D",   "001100"}, {"!D",  "001101"}, {"-D",  "001111"},
  {"D+1", "011111"}, {"D-1", "001110"},
  {"A",   "110000"},
  {"!A",  "110001"},
  {"-A",  "110011"},
  {"D-A", "010011"},
  {"D+A", "000010"},
  {"D&A", "000000"},
  {"D|A", "010101"},
  {"A+1", "110111"},
  {"A-1", "110010"},
  {"A-D", "000111"},
  // Extension instruction
  {"D<<A", "100000"},
  {"D>>A", "100001"},
  {"D*A",  "100010"},
  {"D/A",  "100011"},
  {"D%A",  "100100"},
  {"D<A",  "100101"},
  {"D<=A", "100110"},
  {"D>A",  "100111"},
  {"D>=A", "101000"},
  {"D==A", "101001"},
  {"D!=A", "101011"},
  {"D^A",  "101100"},
  // Software interrupt : 避開 {"D+1", "011111"} {"D-A", "010011"} {"D|A", "010101"}
  {"puts", "010000"},
  {"gets", "010001"},
  {"puti", "010010"},
  {"putf", "011000"},
  {"addf", "011001"},
  {"subf", "011010"},
  {"mulf", "011011"},
  {"divf", "011100"},
};

Pair jList[] = {
  {"",   "000"}, {"JGT","001"}, {"JEQ","010"}, {"JGE","011"},
  {"JLT","100"}, {"JNE","101"}, {"JLE","110"}, {"JMP","111"}
};


int addr[SYM_SIZE] = {
  0, 1, 2, 3,
  4, 5, 6, 7,
  8, 9, 10, 11, 
  12, 13, 14, 15,
  16384, 24576, 
  0, 1, 2, 3, 4
};

Pair symList[] = {
  {"R0",&addr[0]},{"R1",&addr[1]},{"R2",&addr[2]},{"R3",&addr[3]}, 
  {"R4",&addr[4]},{"R5",&addr[5]},{"R6",&addr[6]},{"R7",&addr[7]},
  {"R8",&addr[8]}, {"R9",&addr[9]}, {"R10",&addr[10]}, {"R11",&addr[11]},
  {"R12",&addr[12]}, {"R13",&addr[13]}, {"R14",&addr[14]}, {"R15",&addr[15]},
  {"SCREEN",&addr[16]}, {"KBD",&addr[17]}, {"SP",&addr[18]}, {"LCL",&addr[19]}, 
  {"ARG",&addr[20]}, {"THIS",&addr[21]}, {"THAT",&addr[22]}
};

Map dMap, cMap, jMap, symMap;
int varTop = 16;

void symAdd(Map *map, char *label, int address) {
  addr[map->top] = address;
  Pair *p = mapAdd(map, stAdd(label), &addr[map->top]);
  printf("  key=%s *value=%d top=%d\n", p->key, *(int*)p->value, map->top);
}

void symDump(Map *map) {
  printf("======= SYMBOL TABLE ===========\n");
  printf("map->top = %d size=%d\n", map->top, map->size);
  for (int i=0; i<map->size; i++) {
    Pair *p = &map->table[i];
    if (p->key != NULL)
      printf("%d: %s, %d\n", i, p->key, *(int*) p->value);
  }
}

int parse(string line, Code *c) {
  replace(line, "\r\t\n", ' ');
  char *p;
  char *pend = strstr(line, "//");
  if (pend != NULL) *pend = '\0';

  c->a[0] = c->d[0] = c->c[0] = c->label[0] = c->j[0] = '\0';
  c->size = 1;
  // printf("line=%s\n", line);
  for (p = line; *p!='\0'; p++) {
    if (*p!=' ') break;
  }
  // printf("p=%s", p);
  if (*p == '(') {
    c->type = 'L';
    c->size = 0;
    char str[100]; float f; int n;
    if (sscanf(p, "(%[^)])%s", c->label, c->data) >=2) {
      printf("label=%s data=%s\n", c->label, c->data);
      if (sscanf(c->data, "\"%s\"", str) == 1) c->size = strlen(str);
      else if (sscanf(c->data, "%f", &f) == 1) c->size = 2;
      else if (sscanf(c->data, "%d", &n) == 1) c->size = 1;
    }
  } else if (*p == '@') {
    c->type = 'A';
    sscanf(p, "@%[^\r\n ]", c->a);
  } else {
    c->type = 'C';
    if (strchr(p, '=') != NULL) {
      sscanf(p, "%[^=]=%[^;]", c->d, c->c);
    } else if (strchr(p, ';') != NULL) {
      sscanf(p, "%[^;];%s", c->c, c->j);
    } else {
      sscanf(p, "%s", c->c);
      if (c->c[0]=='\0') return 0;
    }
    replace(c->c, " ", '\0');
  }
  // printf("  code:type=%c label=%s a=%s d=%s c=%s j=%s\n", c->type, c->label, c->a, c->d, c->c, c->j);
  return 1;
}

void comp2code(char *comp, char *ccode, char *ami) {
  // printf("comp=|%s|\n", comp);
  char *aComp = comp, mComp[100], iComp[100], *code;
  code = mapLookup(&cMap, aComp); // A: 10xxxxxx
  if (code != NULL) { strcpy(ami, "10"); strcpy(ccode, code); return; }
  strcpy(mComp, comp); replace(mComp, "M", 'A');
  code = mapLookup(&cMap, mComp); // M: 11xxxxxx
  if (code != NULL) { strcpy(ami, "11"); strcpy(ccode, code); return; }
  strcpy(iComp, comp); replace(iComp, "I", 'A');
  code = mapLookup(&cMap, iComp); // I: 01xxxxxx
  if (code != NULL) { strcpy(ami, "01"); strcpy(ccode, code); return; }
  assert(0);
}

void code2binary(Code *code, int16_t *bin) {
  char bstr[100];
  // printf("code2binary()");
  if (code->type=='A') { // A 指令： ＠number || @symbol
    int A;
    if (isdigit(code->a[0])) {
      A = atoi(code->a);
      bin[0] = A;
      // itob(A, bstr);
    } else {
      char *symbol = code->a;
      int* addrPtr = mapLookup(&symMap, symbol);
      if (addrPtr == NULL) { // 宣告變數
        symAdd(&symMap, symbol, varTop); // 新增一個變數
        A = varTop ++;
      } else { // 已知變數 (標記) 位址
        A = *addrPtr;
      }
      bin[0] = A;
      // itob(A, binary);
    }
  } else if (code->type == 'C') { // C 指令
    char ami[3], ccode[10];
    if (code->d[0] != '\0') { // d=comp
      char *dcode = mapLookup(&dMap, code->d);
      // printf("  comp=%s\n", code->c);
      comp2code(code->c, ccode, ami);
      sprintf(binary, "11%s%s%s000", ami, ccode, dcode);
    } else { // comp;j
      comp2code(code->c, ccode, ami);
      char *jcode = mapLookup(&jMap, code->j);
      sprintf(binary, "11%s%s000%s", ami, ccode, jcode);      
    }
  } else if (code->type == 'L') { // LABEL
    char str[100]; int n; float f;
    if (sscanf(c->data, "%s", str) == 1) {
      
    }
  }
}

void pass1(string inFile) {
  printf("============= PASS1 ================\n");
  char line[100]="";
  FILE *fp = fopen(inFile, "r");
  int address = 0;
  while (fgets(line, sizeof(line), fp)) {
    Code code;
    if (!parse(line, &code)) continue;
    printf("%02d:%s\n", address, line);
    if (code.label[0] != '\0') {
      symAdd(&symMap, code.label, address);
    } else {
      address += code.size;
    }
  }
  fclose(fp);
}

void pass2(string inFile, string hackFile, string binFile) {
  printf("============= PASS2 ================\n");
  char line[100], binary[17];
  FILE *fp = fopen(inFile, "r");
  FILE *hfp = fopen(hackFile, "w");
  FILE *bfp = fopen(binFile, "wb");
  while (fgets(line, sizeof(line), fp)) {
    Code code;
    // printf("line=%s\n", line);
    if (!parse(line, &code)) continue;
    if (code.label[0] != '\0') {
      printf("(%s)\n", code.label);
    } else {
      code2binary(&code, binary);
      uint16_t b = btoi(binary);
      printf("  %-20s %s %04x\n", line, binary, b);
      fprintf(hfp, "%s\n", binary);
      fwrite(&b, sizeof(b), 1, bfp);
    }
  }
  fclose(fp);
  fclose(hfp);
  fclose(bfp);
}

void assemble(string file) {
  char inFile[100], hackFile[100], binFile[100];
  sprintf(inFile, "%s.asm", file);
  sprintf(hackFile, "%s.hack", file);
  sprintf(binFile, "%s.bin", file);
  symDump(&symMap);
  pass1(inFile);
  symDump(&symMap);
  pass2(inFile, hackFile, binFile);
}

// run: ./asm <file> 
// notice : <file> with no extension.
int main(int argc, char *argv[]) {
  // 建置表格
  mapNew(&dMap, 37); mapAddAll(&dMap, dList, ARRAY_SIZE(dList));
  mapNew(&cMap, 79); mapAddAll(&cMap, cList, ARRAY_SIZE(cList));
  mapNew(&jMap, 23); mapAddAll(&jMap, jList, ARRAY_SIZE(jList));
  mapNew(&symMap, SYM_SIZE); mapAddAll(&symMap, symList, ARRAY_SIZE(symList));
  stInit();
  // 組譯器主要函數開始
  assemble(argv[1]);
  // 釋放表格
  mapFree(&dMap);
  mapFree(&cMap);
  mapFree(&jMap);
  mapFree(&symMap);
}
