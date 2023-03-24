#include <stdlib.h>
#include <stdio.h>

#include "lines_info.h"
#include "memory.h"

void initLinesInfo(LinesInfo* lines_info) {
    lines_info->capacity = 0;
    lines_info->count = 0;
    lines_info->lines = NULL;
    lines_info->counts = NULL;
}
void freeLinesInfo(LinesInfo* lines_info) {
    FREE_ARRAY(int, lines_info->lines, lines_info->capacity);
    FREE_ARRAY(int, lines_info->counts, lines_info->capacity);
    initLinesInfo(lines_info);
}

static bool checkSameLine(LinesInfo* lines_info, int line) {
    if (lines_info->count == 0) {
        return false;
    }

    int last_line_idx = lines_info->count - 1;
    int last_line = lines_info->lines[last_line_idx];
    return last_line == line;
}
static void updateLinesInfoSize(LinesInfo* lines_info, int line) {
    if (lines_info->capacity >= lines_info->count + 1) {
        return;
    }

    int old_capacity = lines_info->capacity;
    lines_info->capacity = GROW_CAPACITY(old_capacity);
    lines_info->counts = GROW_ARRAY(int, lines_info->counts,
        old_capacity, lines_info->capacity);
    lines_info->lines = GROW_ARRAY(int, lines_info->lines,
        old_capacity, lines_info->capacity);
}
void writeLinesInfo(LinesInfo* lines_info, int line) {
    if (checkSameLine(lines_info, line)) {
        lines_info->counts[lines_info->count - 1]++;
        return;
    }
    
    updateLinesInfoSize(lines_info, line);

    lines_info->lines[lines_info->count] = line;
    lines_info->counts[lines_info->count] = 1;
    lines_info->count++;
}
int getLine(LinesInfo* lines_info, int byte_idx) {
    int idx = 0;
    for (int i = 0; i < lines_info->count; ++i) {
        idx += lines_info->counts[i];
        if (byte_idx < idx) {
            return lines_info->lines[i];
        }
    }

    printf("can't get line number for byte %d", byte_idx);
    exit(EXIT_FAILURE);    
}