#pragma once

#include <sys/types.h>

extern int io_wantread(int fd);
extern int io_dontwantread(int fd);
extern int io_canread(int fd);

extern int io_wantwrite(int fd);
extern int io_dontwantwrite(int fd);
extern int io_canwrite(int fd);

extern int io_wait(unsigned long long timeout);

extern int io_nonblock(int fd);

extern ssize_t io_read(int fd, void *buffer, size_t len);
extern ssize_t io_write(int fd, const void *buffer, size_t len);

extern ssize_t io_read_sync(int fd, void *buffer, size_t len);
extern ssize_t io_write_sync(int fd, const void *buffer, size_t len);

extern int io_close(int fd);
