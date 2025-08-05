#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

#define MSG_ERROR(file, line, msg, ...)    fprintf(stderr, "%s:%d:\033[31m error:\033[0m " msg "\n", file, line, ##__VA_ARGS__)
#define MSG_WARNING(file, line, msg, ...)  fprintf(stderr, "%s:%d:\033[35m warning:\033[0m " msg "\n", file, line, ##__VA_ARGS__)
#define MSG_FATAL(file, line, msg, ...)    do { fprintf(stderr, "%s:%d:\033[91m fatal error:\033[0m " msg "\n", file, line, ##__VA_ARGS__); exit(1); } while (0)
#define MSG_INFO(file, line, msg, ...)     fprintf(stdout, "%s:%d:\033[37m info:\033[0m " msg "\n", file, line, ##__VA_ARGS__)

#endif