#if !defined(TCP_H_)
#define TCP_H_

#include "platforms.hpp"

#include <netinet/in.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_TCP_FDS 16

#endif // TCP_H_
