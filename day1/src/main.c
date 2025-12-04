#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
typedef struct timeval timeval;

#define BUFFERSIZE 10
char filename[] = "input.txt";

static int file_readline(FILE *file, char *buffer, unsigned buffersize) {
  memset(buffer, 0, BUFFERSIZE * sizeof(char));
  // for (int i = 0; i < buffersize; i++) {
  //   buffer[i] = 0;
  // }

  int c;
  for (int i = 0; i < buffersize; i++) {
    c = fgetc(file);
    if (c == EOF) {
      return 0;
    }

    c = (char)c;
    if (c == '\n') {
      break;
    }
    buffer[i] = c;
  }

  return 1;
}

static int parse_int(char *buffer) {
  unsigned char current = buffer[0] - '0';
  int i = 1;
  int result = 0;

  while (current <= '9') {
    result = result * 10 + current;
    current = (unsigned char)buffer[i++] - (unsigned char)'0';
  }

  return result;
}

typedef enum {
  Left = -1,
  Right = 1,
} Direction;

int main() {
  timeval start, end;
  gettimeofday(&start, NULL);
  FILE *input = fopen(filename, "r");
  int rotation_curr = 50;
  int rotation_prev = 50;
  unsigned num_zeros_p1 = 0;
  unsigned num_zeros_p2 = 0;

  char buffer[BUFFERSIZE] = {0};
  while (file_readline(input, buffer, BUFFERSIZE)) {
    Direction dir = (buffer[0] == 'L') ? Left : Right;
    int amount = parse_int(&buffer[1]);
    int clicks = 0;

    if (dir == Right) {
      clicks = (rotation_curr + amount) / 100;
      rotation_curr = (rotation_curr + amount) % 100;
    } else {
      if (rotation_curr == 0) {
        clicks = amount / 100;
      } else if (amount >= rotation_curr) {
        clicks = (amount - rotation_curr) / 100 + 1;
      }
      rotation_curr = (rotation_curr - amount) % 100;
    }
    while (rotation_curr < 0) {
      rotation_curr += 100;
    }

    num_zeros_p2 += clicks;

    if (rotation_curr == 0) {
      num_zeros_p1 += 1;
    }

    // printf("%5s, dir: %2d, amout: %2d, rot: %2d, clicks: %4d\n", buffer, dir,
    //        amount, rotation_curr, num_zeros_p2);
    rotation_prev = rotation_curr;
  }

  gettimeofday(&end, NULL);
  double dt = end.tv_usec - start.tv_usec;
  printf("time: %f µs\n", dt);
  printf("\x1b[33mResult part 1: %d\x1b[0m\n", num_zeros_p1);
  printf("\x1b[33mResult part 2: %d\x1b[0m\n", num_zeros_p2);
  fclose(input);
  return 0;
}
