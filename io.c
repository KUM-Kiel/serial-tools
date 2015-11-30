#include "io.h"

#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

typedef struct {
  fd_set wantread, wantwrite, canread, canwrite;
  int wantreadmax, wantwritemax;
} io_internal;

static io_internal internal;
static int initialized = 0;

#define max(a, b) ((a) >= (b) ? (a) : (b))

static void init()
{
  FD_ZERO(&internal.wantread);
  FD_ZERO(&internal.wantwrite);
  FD_ZERO(&internal.canread);
  FD_ZERO(&internal.canwrite);
  internal.wantreadmax = -1;
  internal.wantwritemax = -1;
  initialized = 1;
}

int io_wantread(int fd)
{
  if (!initialized) init();
  if (fd >= FD_SETSIZE) return -1;
  FD_SET(fd, &internal.wantread);
  if (fd > internal.wantreadmax)
    internal.wantreadmax = fd;
  return 0;
}

int io_dontwantread(int fd)
{
  int i;
  if (!initialized) init();
  if (fd >= FD_SETSIZE) return -1;
  if (FD_ISSET(fd, &internal.wantread)) {
    FD_CLR(fd, &internal.wantread);
    if (fd == internal.wantreadmax) {
      i = fd;
      while (i >= 0) {
        if (FD_ISSET(i, &internal.wantread)) break;
        --i;
      }
      internal.wantreadmax = i;
    }
  }
  return 0;
}

int io_canread(int fd)
{
  if (!initialized) init();
  return FD_ISSET(fd, &internal.canread);
}

int io_wantwrite(int fd)
{
  if (!initialized) init();
  if (fd >= FD_SETSIZE) return -1;
  FD_SET(fd, &internal.wantwrite);
  if (fd > internal.wantwritemax)
    internal.wantwritemax = fd;
  return 0;
}

int io_dontwantwrite(int fd)
{
  int i;
  if (!initialized) init();
  if (fd >= FD_SETSIZE) return -1;
  if (FD_ISSET(fd, &internal.wantwrite)) {
    FD_CLR(fd, &internal.wantwrite);
    if (fd == internal.wantwritemax) {
      i = fd;
      while (i >= 0) {
        if (FD_ISSET(i, &internal.wantwrite)) break;
        --i;
      }
      internal.wantwritemax = i;
    }
  }
  return 0;
}

int io_canwrite(int fd)
{
  if (!initialized) init();
  return FD_ISSET(fd, &internal.canwrite);
}

#define SEC 1000000L;
int io_wait(unsigned long long timeout)
{
  struct timeval t;
  if (internal.wantreadmax < 0 && internal.wantwritemax < 0) return -1;
  memcpy(&internal.canread, &internal.wantread, sizeof(fd_set));
  memcpy(&internal.canwrite, &internal.wantwrite, sizeof(fd_set));
  t.tv_sec = timeout / SEC;
  t.tv_usec = timeout % SEC;
  return select(
    max(internal.wantreadmax, internal.wantwritemax) + 1,
    internal.wantreadmax < 0 ? 0 : &internal.canread,
    internal.wantwritemax < 0 ? 0 : &internal.canwrite,
    0,
    timeout ? &t : 0
  );
}

int io_nonblock(int fd)
{
  int flags = fcntl(fd, F_GETFD);
  if (flags == -1) return -1;
  return fcntl(fd, F_SETFD, flags | O_NONBLOCK);
}

ssize_t io_read(int fd, void *buffer, size_t len)
{
  ssize_t r = read(fd, buffer, len);
  if (r == -1 && errno != EAGAIN) return -3;
  return r;
}

ssize_t io_write(int fd, const void *buffer, size_t len)
{
  ssize_t w = write(fd, buffer, len);
  if (w == -1 && errno != EAGAIN) return -3;
  return w;
}

ssize_t io_read_sync(int fd, void *buffer, size_t len)
{
  fd_set fds;
  ssize_t r;
  unsigned char *b = buffer;
  while (len > 0) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    if (select(fd + 1, &fds, 0, 0, 0) < 0) return -3;
    r = read(fd, b, len);
    if (r < 0 && errno != EAGAIN) {
      return -3;
    } else if (r > 0) {
      b += r;
      len -= r;
    } else {
      break;
    }
  }
  return b - (unsigned char *) buffer;
}

ssize_t io_write_sync(int fd, const void *buffer, size_t len)
{
  fd_set fds;
  ssize_t w;
  const unsigned char *b = buffer;
  while (len > 0) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    if (select(fd + 1, 0, &fds, 0, 0) < 0) return -3;
    w = write(fd, b, len);
    if (w < 0 && errno != EAGAIN) {
      return -3;
    } else if (w > 0) {
      b += w;
      len -= w;
    } else {
      break;
    }
  }
  return b - (const unsigned char *) buffer;
}

int io_close(int fd)
{
  return close(fd);
}
