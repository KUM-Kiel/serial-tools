#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "io.h"
#include "options.h"
#include "rs232.h"
#include "tun.h"

static void error(const char *e)
{
  fprintf(stderr, "%s\n", e);
  exit(1);
}

static int is_command(const char *a, const char *b)
{
  int i = 0, la = 0, lb = 0;
  while (a[i]) {
    if (a[i] == '/') la = i + 1;
    ++i;
  }
  i = 0;
  while (b[i]) {
    if (b[i] == '/') lb = i + 1;
    ++i;
  }
  return !strcmp(a + la, b + lb);
}

static const char *program = "tunslip";
static void usage(const char *opt, const char *arg, int l)
{
  if (is_command(program, "trillium")) {
    fprintf(stderr,
      "\n"
      "Trillium opens a connection to a Nanometrics seismometer.\n"
      "\n"
      "Upon startup, the IP address and model info is printed to the console.\n"
      "While the program is running, you can type 'NMX' to display the IP address\n"
      "of the currently connected seismometer.\n"
      "\n"
      "# Usage\n"
      "\n"
      "  %s [TTY]\n"
      "\n"
      "  TTY defaults to /dev/ttyUSB0.\n"
      /*"\n"
      "# Troubleshooting\n"
      "\n"
      "You must probably be root in order to run the program.\n"*/
      "\n",
      program);
  } else {
    fprintf(stderr,
      "\n"
      "Usage: %s [-f|--fast] [-s|--slow] [-t|--trillium] /dev/ttyXXX\n",
      program);
  }
  exit(1);
}

#define END     192 /* indicates end of packet */
#define ESC     219 /* indicates byte stuffing */
#define ESC_END 220 /* ESC ESC_END means END data byte */
#define ESC_ESC 221 /* ESC ESC_ESC means ESC data byte */

static const char *trillium_model(int model_number)
{
  switch (model_number) {
    case 20: return "Trillium 120";
    case 21: return "Trillium 240";
    case 22: return "Trillium 240 OBS";
    case 23: return "Trillium Compact";
    case 24: return "Titan";
    case 28: return "Trillium Compact OBS";
    case 29: return "Trillium Compact All Terrain";
    case 43: return "Trillium Compact OBS";
    default: return "Unknown Model";
  }
}

enum {
  COMMAND_DEFAULT = 0,
  COMMAND_INVALID,
  COMMAND_N,
  COMMAND_NM,
  COMMAND_NMX
};

int main(int argc, char **argv)
{
  char dev[32] = "slip%d";
  char *tty_path = "/dev/ttyUSB0";

  int tty, tun;
  int speed = 19200;
  int trillium = 0;
  ssize_t r;
  unsigned char ip_packet[4096], slip_packet[4096], buffer[4096];
  int received = 0, esc = 0, i, j;
  int stdin_state = COMMAND_DEFAULT;

  /* Parse options. */
  program = argv[0];
  if (is_command(argv[0], "trillium")) {
    trillium = 1;
  }
  parse_options(&argc, &argv, OPTIONS(
    FLAG('f', "fast", speed, 115200),
    FLAG('s', "slow", speed, 9600),
    FLAG('t', "trillium", trillium, 1),
    FLAG_CALLBACK('h', "help", usage)
  ));

  if (trillium) {
    speed = 9600;
  }

  /* Check for TTY argument. */
  if (argc >= 2) {
    tty_path = argv[1];
  }

  /* Open TTY. */
  if ((tty = rs232_open(tty_path, speed)) == -1) {
    fprintf(stderr, "%s\n", "Could not open TTY.");
    if (getuid()) {
      fprintf(stderr, "%s\n", "Please try again as root!");
    }
    return 1;
  }

  /* Open TUN. */
  if ((tun = tun_alloc(dev, "2.1.0.0", "255.0.0.0")) < 0) {
    fprintf(stderr, "%s\n", "Could not open TUN.");
    if (getuid()) {
      fprintf(stderr, "%s\n", "Please try again as root!");
    }
    return 1;
  }

  io_nonblock(0);
  io_nonblock(tun);
  io_nonblock(tty);

  if (trillium) {
    io_write_sync(tty, "\300NMX\300", 5);
  }

  /* IO loop. */
  while (1) {
    io_wantread(0);
    io_wantread(tun);
    io_wantread(tty);

    if (io_wait(0) < 0) error("IO error. Could not wait.");

    /* Read commands from stdin. */
    if (io_canread(0)) {
      r = io_read(0, buffer, sizeof(buffer));
      if (r == -3) {
        error("IO error. Could not read stdin.");
      } else if (r > 0) {
        /* Data. */
        for (i = 0; i < r; ++i) {
          switch (stdin_state) {
            case COMMAND_DEFAULT:
              stdin_state = (buffer[i] == 'N' || buffer[i] == 'n') ? COMMAND_N : COMMAND_INVALID;
              break;
            case COMMAND_N:
              stdin_state = (buffer[i] == 'M' || buffer[i] == 'm') ? COMMAND_NM : COMMAND_INVALID;
              break;
            case COMMAND_NM:
              stdin_state = (buffer[i] == 'X' || buffer[i] == 'x') ? COMMAND_NMX : COMMAND_INVALID;
              break;
            case COMMAND_NMX:
              if (buffer[i] == '\n') {
                io_write_sync(tty, "\300NMX\300", 5);
                stdin_state = COMMAND_DEFAULT;
              } else if (buffer[i] != '\r') {
                stdin_state = COMMAND_INVALID;
              }
          }
          if (buffer[i] == '\n') {
            stdin_state = COMMAND_DEFAULT;
          }
        }
      } else if (r == 0) {
        /* Closed. */
        printf("\033[KClosed.\n");
        goto end;
      }
    }

    /* Write TUN data to RS232. */
    if (io_canread(tun)) {
      r = io_read(tun, buffer, sizeof(buffer));
      if (r == -3) {
        error("IO error. Could not read TUN.");
      } else if (r > 0) {
        /* Data. */
        j = 0;
        if (j < sizeof(slip_packet)) slip_packet[j++] = END;
        for (i = 0; i < r; ++i) {
          switch (buffer[i]) {
            case END:
              if (j < sizeof(slip_packet)) slip_packet[j++] = ESC;
              if (j < sizeof(slip_packet)) slip_packet[j++] = ESC_END;
              break;
            case ESC:
              if (j < sizeof(slip_packet)) slip_packet[j++] = ESC;
              if (j < sizeof(slip_packet)) slip_packet[j++] = ESC_ESC;
              break;
            default:
              if (j < sizeof(slip_packet)) slip_packet[j++] = buffer[i];
              break;
          }
        }
        if (j < sizeof(slip_packet)) slip_packet[j++] = END;
        if (io_write_sync(tty, slip_packet, j) < 0) error("IO error. Culd not write TTY.");
      } else if (r == 0) {
        /* Closed. */
        printf("\033[KClosed.\n");
        goto end;
      }
    }

    /* Write RS232 data to TUN. */
    if (io_canread(tty)) {
      r = io_read(tty, buffer, sizeof(buffer));
      if (r == -3) {
        error("IO error. Could not read TTY.");
      } else if (r > 0) {
        /* Data. */
        for (i = 0; i < r; ++i) {
          switch (buffer[i]) {
            case END:
              if (received) {
                if (received == 8 && !memcmp(ip_packet, "NMX", 3)) {
                  printf("%s http://2.%d.%d.%d/\n", trillium_model(ip_packet[3]), (int) ip_packet[3], (int) ip_packet[4], (int) ip_packet[5]);
                  fflush(stdout);
                } else {
                  if (io_write_sync(tun, ip_packet, received) < 0) error("IO error. Could not write TUN.");
                }
                received = 0;
              }
              esc = 0;
              break;
            case ESC:
              esc = 1;
              break;
            case ESC_END:
              if (received < sizeof(ip_packet)) {
                ip_packet[received++] = esc ? END : ESC_END;
              }
              esc = 0;
              break;
            case ESC_ESC:
              if (received < sizeof(ip_packet)) {
                ip_packet[received++] = esc ? ESC : ESC_ESC;
              }
              esc = 0;
              break;
            default:
              if (received < sizeof(ip_packet)) {
                ip_packet[received++] = buffer[i];
              }
              esc = 0;
              break;
          }
        }
      } else if (r == 0) {
        /* Closed. */
        printf("\033[KClosed.\n");
        goto end;
      }
    }
  }

end:
  io_close(tty);
  return 0;
}
