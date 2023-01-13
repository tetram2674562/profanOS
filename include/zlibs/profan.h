#ifndef PROFAN_LIB_ID
#define PROFAN_LIB_ID 1002

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

// int setting_get(char name[])
// void assemble_path(char old[], char new[], char result[])

#define setting_get ((int (*)(char[])) get_func_addr(PROFAN_LIB_ID, 2))
#define assemble_path ((void (*)(char[], char[], char[])) get_func_addr(PROFAN_LIB_ID, 3))

#endif
