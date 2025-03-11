#ifndef __lib_h__
#define __lib_h__

#define SYS_WRITE 0 // 可修改 可自定义
#define STD_OUT 0 // 可修改 可自定义

#define SYS_READ 1 // 可修改 可自定义
#define STD_IN 0 // 可修改 可自定义
#define STD_STR 1 // 可修改 可自定义

#define MAX_BUFFER_SIZE 256

void printf(const char *format,...);
void sleep(unsigned int seconds);
char getChar();
void getStr(char *str, int size);

struct TimeInfo {
    int second;
    int minute;
    int hour;
    int m_day;
    int month;
    int year;
};

void now(struct TimeInfo* tm_info);

#endif
