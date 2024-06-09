//
// key_linux.c ... one key, raw mode, non blocking input
//


#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "key_in.h"

void changemode(int dir)
{
  static struct termios oldt, newt;

  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}

int kbhit (void)
{
  struct timeval tv;
  fd_set rdfs;

  tv.tv_sec = 0;
  tv.tv_usec = 10000;

  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);

  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  //printf("%d", f);fflush(stdout);
  return FD_ISSET(STDIN_FILENO, &rdfs);

}

int key_in(void) {
    int ch;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;  // 10 millisec
    while (!kbhit()) {
        nanosleep(&ts, &ts);
    }
  	ch = getchar();
    return ch;
}
