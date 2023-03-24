#ifndef clox_lines_info_h
#define clox_lines_info_h

typedef struct {
    int capacity;
    int count;
    int* lines;
    int* counts;
} LinesInfo;

void initLinesInfo(LinesInfo* lines_info);
void freeLinesInfo(LinesInfo* lines_info);
void writeLinesInfo(LinesInfo* lines_info, int line);
int getLine(LinesInfo* lines_info, int byte_idx);

#endif