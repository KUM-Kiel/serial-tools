#include "rs232.h"
#include "io.h"
#include "options.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *inlog = 0, *outlog = 0, *errorlog = 0;

static void error(const char *e)
{
  if (errorlog) {
    fprintf(errorlog, "%s\n", e);
    fflush(errorlog);
  }
  fprintf(stderr, "%s\n", e);
  exit(1);
}

static const char *program = "rs232cat";
static void usage(const char *opt, const char *arg, int l)
{
  fprintf(stderr, "Usage: %s [-f|--fast] /dev/ttyXXX\n", program);
  exit(1);
}

int main(int argc, char **argv)
{
  int tty;
  int speed = 19200;
  int log = 0, i, transform_n = 0;
  ssize_t r;
  unsigned char buffer[1024];

  /* Parse options. */
  program = argv[0];
  parse_options(&argc, &argv, OPTIONS(
    FLAG('f', "fast", speed, 115200),
    FLAG('s', "slow", speed, 9600),
    FLAG(0, "4800", speed, 4800),
    FLAG(0, "9600", speed, 9600),
    FLAG(0, "19200", speed, 19200),
    FLAG(0, "57600", speed, 57600),
    FLAG(0, "115200", speed, 115200),
    FLAG('l', "log", log, 1),
    FLAG('t', "transform", transform_n, 1),
    FLAG_CALLBACK('h', "help", usage)
  ));

  /* Check for TTY argument. */
  if (argc < 2) usage(0, 0, 0);

  /* Open TTY. */
  if ((tty = rs232_open(argv[1], speed)) == -1) error("Could not open TTY.");

  io_nonblock(0);
  io_nonblock(1);
  io_nonblock(tty);

  /* Open logfiles. */
  if (log) {
    inlog = fopen("rs232in.log", "ab");
    outlog = fopen("rs232out.log", "ab");
    errorlog = fopen("rs232error.log", "ab");
    if (!inlog || !outlog || !errorlog) error("Could not open logfiles.");
  }

  /* IO loop. */
  while (1) {
    io_wantread(0);
    io_wantread(tty);

    if (io_wait() < 0) error("IO error.");

    /* Write stdin data to RS232. */
    if (io_canread(0)) {
      r = io_read(0, buffer, sizeof(buffer));
      if (r == -3) {
        error("IO error.");
      } else if (r > 0) {
        /* Data. */
        if (inlog) {
          fwrite(buffer, r, 1, inlog);
          fflush(inlog);
        }
        if (transform_n) {
          for (i = 0; i < r; ++i) {
            if (buffer[i] == '\n') {
              if (io_write_sync(tty, "\r\n", 2) < 0) error("IO error.");
            } else {
              if (io_write_sync(tty, buffer + i, 1) < 0) error("IO error.");
            }
          }
        } else {
          if (io_write_sync(tty, buffer, r) < 0) error("IO error.");
        }
      } else if (r == 0) {
        /* Closed. */
        printf("\n");
        goto end;
      }
    }

    /* Write RS232 data to stdout. */
    if (io_canread(tty)) {
      r = io_read(tty, buffer, sizeof(buffer));
      if (r == -3) {
        error("IO error.");
      } else if (r > 0) {
        /* Data. */
        if (outlog) {
          fwrite(buffer, r, 1, outlog);
          fflush(outlog);
        }
        if (io_write_sync(1, buffer, r) < 0) error("IO error.");
      } else if (r == 0) {
        /* Closed. */
        printf("\n");
        goto end;
      }
    }
  }

end:
  io_close(tty);
  return 0;
}
