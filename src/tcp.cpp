#include "tcp.hpp"

void report_peer_connected(const struct sockaddr_in* sa, socklen_t salen) {
    char hostbuf[NI_MAXHOST];
    char portbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)sa, salen, hostbuf, NI_MAXHOST, portbuf, NI_MAXSERV, 0) == 0) {
        printf("peer (%s, %s) connected\n", hostbuf, portbuf);
    } else {
        printf("peer (unknonwn) connected\n");
    }
}

// accepting ip connections
void ipserver_listen(int listening_port, thread_t* server_thread){
	struct timeval timeout;      
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;
	
	int listenfd;
	int acceptfd;
	struct sockaddr_in listen_addr; 
	struct sockaddr_in accept_addr; 
	
	int t;
	
    if (( listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		fprintf(stderr,"Listen socket creation failed.\r\n");
		exit(1);
	}

	int so_reuseaddr = 1;

    if (( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&so_reuseaddr, sizeof(so_reuseaddr))) == -1){
		fprintf(stderr,"Setting listen socket options failed.\r\n");
		exit(1);
	}

    if (( setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))) == -1){
		fprintf(stderr,"Setting listen socket options failed.\r\n");
		exit(1);
	}
	#ifdef NO_TCP_DELAY
		int tcpnodelaypar = 1;
		if (( setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, (void*)&tcpnodelaypar, sizeof(tcpnodelaypar))) == -1){
			fprintf(stderr,"Setting listen socket TCP_NODELAY failed.\r\n");
			exit(1);
		}
	#endif

	listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(listening_port);

    if (( bind(listenfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr))) == -1){
		fprintf(stderr,"Binding address to socket failed.\r\n");
		exit(1);
	}
	
	// try to specify max of 32 pending connections
    if (listen(listenfd, 32) < 0){
		fprintf(stderr,"Listening to socket failed.\r\n");
		exit(1);
	}; 

	int connect_count = 0;

	*server_thread = (std::thread(ipserver, listenfd));
}

// ip server thread
void ipserver(int listenfd){
	
	unsigned int header0;
	unsigned int destnode;
	unsigned int wordsize;
	void *packet_buffer;
	unsigned int dest_thread;
	
	// gasnet_ip_host_t *connection_data = (gasnet_ip_host_t*)data;

	int epollfd = epoll_create1(0);
	if (epollfd < 0){
		fprintf(stderr,"Error creating epoll.\r\n");
		exit(1);
	}

	struct epoll_event accept_event;
  	accept_event.data.fd = listenfd;
	accept_event.events = EPOLLIN;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &accept_event) < 0) {
		fprintf(stderr, "epoll_ctl EPOLL_CTL_ADD");
		exit(1);
	}

	struct epoll_event* events = (struct epoll_event*) calloc(MAX_TCP_FDS, sizeof(struct epoll_event));
	if (events == NULL) {
		fprintf(stderr, "Unable to allocate memory for epoll_events");
		exit(1);
	}

	// int read_fd = connection_data->server_fd;
	
	while(1){

		int nready = epoll_wait(epollfd, events, MAX_TCP_FDS, -1);
		for (int i = 0; i < nready; i++) {
			if (events[i].events & EPOLLERR) {
				fprintf(stderr, "epoll_wait returned EPOLLERR");
				exit(1);
			}

			// The listening socket is ready; this means a new peer is connecting.
			if (events[i].data.fd == listenfd) {
				struct sockaddr_in peer_addr;
				socklen_t peer_addr_len = sizeof(peer_addr);
				int newsockfd = accept(listenfd, (struct sockaddr*)&peer_addr,
					&peer_addr_len);
				if (newsockfd < 0) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						// This can happen due to the nonblocking socket mode; in this
						// case don't do anything, but print a notice (since these events
						// are extremely rare and interesting to observe...)
						printf("accept returned EAGAIN or EWOULDBLOCK\n");
					} else {
						fprintf(stderr, "accept");
						exit(1);
					}
				} else {
					int flags = fcntl(newsockfd, F_GETFL, 0);
					if (flags < 0) {
						ON_ERROR("Could not get server socket flags: %s\n", strerror(errno))
					}
					int err = fcntl(newsockfd, F_SETFL, flags | O_NONBLOCK);
					if (err < 0){
						ON_ERROR("Could set server socket to be non blocking: %s\n", strerror(errno));
					}
					if (newsockfd >= MAX_TCP_FDS) {
						fprintf(stderr, "socket fd (%d) >= MAX_TCP_FDS (%d)", newsockfd, MAX_TCP_FDS);
						exit(1);
					}
                    
                    report_peer_connected(&peer_addr, peer_addr_len);

					// fd_status_t status =
					// 	on_peer_connected(newsockfd, &peer_addr, peer_addr_len);
					struct epoll_event event = {0};
					event.data.fd = newsockfd;
					event.events |= EPOLLIN;

					if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newsockfd, &event) < 0) {
						fprintf(stderr, "epoll_ctl EPOLL_CTL_ADD");
						exit(1);
					}
				}
			} else {
				// A peer socket is ready.
				if (events[i].events & EPOLLIN) {
					// Ready for reading.
					int fd = events[i].data.fd;
					char buf[1024];
					// fd_status_t status = on_peer_ready_recv(fd, buf);
					struct epoll_event event = {0};
					event.data.fd = fd;
					event.events |= EPOLLIN;
					if (event.events == 0) {
						printf("socket %d closing\n", fd);
						if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
							fprintf(stderr, "epoll_ctl EPOLL_CTL_DEL");
							exit(1);
						}
						close(fd);
					} else if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) < 0) {
						fprintf(stderr, "epoll_ctl EPOLL_CTL_MOD");
						exit(1);
					}
				} else if (events[i].events & EPOLLOUT) {
					// Ready for writing.
					int fd = events[i].data.fd;
					// fd_status_t status = on_peer_ready_send(fd);
					struct epoll_event event = {0};
					event.data.fd = fd;

					event.events |= EPOLLIN;
					if (event.events == 0) {
						printf("socket %d closing\n", fd);
						if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
							fprintf(stderr, "epoll_ctl EPOLL_CTL_DEL");
							exit(1);
						}
						close(fd);
					} else if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) < 0) {
						fprintf(stderr, "epoll_ctl EPOLL_CTL_MOD");
						exit(1);
					}
				}
			}
		}
	}
}
