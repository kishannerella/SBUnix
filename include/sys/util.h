#ifndef _UTIL_H
#define _UTIL_H

#define bit(value, flag) ((value & flag) == flag)
#define bis(value, flag) (value |= flag)
#define bic(value, flag) (value &= ~flag)

//void sleep(uint32_t sec);
uint64_t min(uint64_t a, uint64_t b);
void memcpy(char* dest, char* src, uint64_t bytes);
void strncpy(char* dest, char* src, uint32_t len);
uint32_t strlen(char *str);
void strcpy(char* dest, char* src);
int strcmp(char *str1, char *str2);
#endif

