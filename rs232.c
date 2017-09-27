#include "rs232.h"

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

int rs232_open(const char *path, unsigned long baud_rate)
{
  struct termios tty;
  speed_t speed;
  int fd;

  switch (baud_rate) {
    case 2400L: speed = B2400; break;
    case 4800L: speed = B4800; break;
    case 9600L: speed = B9600; break;
    case 19200L: speed = B19200; break;
    case 57600L: speed = B57600; break;
    case 115200L: speed = B115200; break;
    case 230400L: speed = B230400; break;
    case 460800L: speed = 460800; break;
    case 500000L: speed = 500000L; break;
    case 1000000L: speed = 1000000L; break;
    default: return -1;
  }

  /* Open TTY. */
  fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) return -1;

  /* Set attributes. */
  memset(&tty, 0, sizeof(tty));
  if (tcgetattr(fd, &tty) != 0) goto fail;

  /* Input flags - Turn off input processing
   * convert break to null byte, no CR to NL translation,
   * no NL to CR translation, don't mark parity errors or breaks
   * no input parity check, don't strip high bit off,
   * no XON/XOFF software flow control */
  tty.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
  /* Output flags - Turn off output processing
   * no CR to NL translation, no NL to CR-NL translation,
   * no NL to CR translation, no column 0 CR suppression,
   * no Ctrl-D suppression, no fill characters, no case mapping,
   * no local output processing */
  tty.c_oflag = 0;
  /* No line processing:
   * echo off, echo newline off, canonical mode off,
   * extended input processing off, signal chars off */
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
  /* Turn off character processing
   * clear current char size mask, no parity checking,
   * no output processing, force 8 bit input */
  tty.c_cflag &= ~(CSIZE | PARENB);
  tty.c_cflag |= CS8;
  /* One input byte is enough to return from read()
   * Inter-character timer off */
  tty.c_cc[VMIN]  = 1;
  tty.c_cc[VTIME] = 2;
  /* Set baud rate */
  if (cfsetospeed(&tty, speed) < 0 || cfsetispeed(&tty, speed) < 0) goto fail;

  /*tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag = (tty.c_cflag & (~CSIZE)) | CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);*/

  if (tcsetattr(fd, TCSANOW, &tty) != 0) goto fail;
  return fd;
fail:
  close(fd);
  return -1;
}
