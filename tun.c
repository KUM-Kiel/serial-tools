#include "tun.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int tun_alloc(char *dev, const char *ip, const char *netmask)
{
  struct ifreq ifr;
  int fd, err, s;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    return -1;

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
   *        IFF_TAP   - TAP device
   *        IFF_NO_PI - Do not provide packet information */
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

  if (*dev)
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if ((err = ioctl(fd, TUNSETIFF, &ifr)) < 0) {
    close(fd);
    return err;
  }
  strcpy(dev, ifr.ifr_name);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  ifr.ifr_addr.sa_family = AF_INET;
  struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;

  inet_pton(AF_INET, ip, &addr->sin_addr);
  ioctl(s, SIOCSIFADDR, &ifr);

  inet_pton(AF_INET, netmask, &addr->sin_addr);
  ioctl(s, SIOCSIFNETMASK, &ifr);

  ioctl(s, SIOCGIFFLAGS, &ifr);
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

  ioctl(s, SIOCSIFFLAGS, &ifr);
  close(s);

  return fd;
}
