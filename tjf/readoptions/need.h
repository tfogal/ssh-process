#ifndef TJF_NEEDIO_H
#define TJF_NEEDIO_H

/* Hacky wrappers around system calls that are required: if they fail, an error
 * message is printed and the program terminates, do not pass go, do not
 * collect $200 */

extern int needopen(const char* filename, int bits);
extern void needclose(int fd);

#endif /* TJF_NEEDIO_H */
