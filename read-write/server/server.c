#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_signal.h"
#include "my_socket.h"
#include "logUtil.h"
#include "readn.h"
#include "get_num.h"

#define HEADER_BYTE_DEFAULT  512
#define BODY_BYTE_DEFAULT    512
#define REPLY_BYTE_DEFAULT  1448

int debug = 0;
int set_so_sndbuf_size = 0;
volatile sig_atomic_t has_usr1 = 0;

int child_proc(int connfd, int header_byte_size, int body_byte_size, int reply_byte_size, int use_no_delay, int use_quick_ack)
{
    int n;
    
    char *header_buf = malloc(header_byte_size);
    if (header_buf == NULL) {
        err(1, "malloc for request_buf");
    }
    char *body_buf = malloc(body_byte_size);
    if (body_buf == NULL) {
        err(1, "malloc for body_buf");
    }
    char *reply_buf = malloc(reply_byte_size);
    if (reply_buf == NULL) {
        err(1, "malloc for reply_buf");
    }

    if (use_no_delay) {
        fprintfwt(stderr, "use_no_delay\n");
        set_so_nodelay(connfd);
    }

    pid_t pid = getpid();
    fprintfwt(stderr, "server: pid: %d\n", pid);

    for ( ; ; ) {
        if (use_quick_ack) {
#ifdef __linux__
            int qack = 1;
            setsockopt(connfd, IPPROTO_TCP, TCP_QUICKACK, &qack, sizeof(qack));
#endif
        }

        /**** read header packet ****/
        n = readn(connfd, header_buf, header_byte_size);
        if (n < 0) {
            err(1, "readn");
        }
        else if (n == 0) {
            fprintfwt(stderr, "server: read EOF\n");
            exit(0);
        }
        fprintfwt(stderr, "server: read header packet\n");

        /**** read body packet ****/
        n = readn(connfd, body_buf, body_byte_size);
        if (n < 0) {
            err(1, "readn");
        }
        else if (n == 0) {
            fprintfwt(stderr, "server: read EOF\n");
            exit(0);
        }
        fprintfwt(stderr, "server: read body packet\n");

        /**** write reply packet ****/
        n = write(connfd, reply_buf, reply_byte_size);
        if (n < 0) {
            fprintfwt(stderr, "server: %s\n", strerror(errno));
            exit(0);
        }
        fprintfwt(stderr, "server: wrote reply packet\n");
    }

    return 0;
}

void sig_chld(int signo)
{
    pid_t pid;
    int   stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        ;
    }
    return;
}

int usage(void)
{
    char *msg =
"Usage: server\n"
"-D      use TCP_NODELAY socket option\n"
"-H N    header byte size (default: 512)\n"
"-B N    body byte size   (default: 512)\n"
"-R N    reply byte size  (default: 1024)\n"
"-p port port number (1234)\n"
"-q      enable quick ack\n";

    fprintf(stderr, "%s", msg);

    return 0;
}

int main(int argc, char *argv[])
{
    int port = 1234;
    pid_t pid;
    struct sockaddr_in remote;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int listenfd;

    int use_no_delay      = 0;
    int use_quick_ack     = 0;
    int header_byte_size  = HEADER_BYTE_DEFAULT;
    int body_byte_size    = BODY_BYTE_DEFAULT;
    int reply_byte_size   = REPLY_BYTE_DEFAULT;

    int c;
    while ( (c = getopt(argc, argv, "DqH:B:R:dhp:")) != -1) {
        switch (c) {
            case 'D':
                use_no_delay = 1;
                break;
            case 'q':
                use_quick_ack = 1;
                break;
            case 'H':
                header_byte_size = get_num(optarg);
                break;
            case 'B':
                body_byte_size = get_num(optarg);
                break;
            case 'R':
                reply_byte_size = get_num(optarg);
                break;
            case 'd':
                debug += 1;
                break;
            case 'h':
                usage();
                exit(0);
            case 'p':
                port = strtol(optarg, NULL, 0);
                break;
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    my_signal(SIGCHLD, sig_chld);
    my_signal(SIGPIPE, SIG_IGN);

    listenfd = tcp_listen(port);
    if (listenfd < 0) {
        errx(1, "tcp_listen");
    }

    for ( ; ; ) {
        int connfd = accept(listenfd, (struct sockaddr *)&remote, &addr_len);
        if (connfd < 0) {
            err(1, "accept");
        }
        
        pid = fork();
        if (pid == 0) { //child
            if (close(listenfd) < 0) {
                err(1, "close listenfd");
            }
            if (child_proc(connfd, header_byte_size, body_byte_size, reply_byte_size, use_no_delay, use_quick_ack) < 0) {
                errx(1, "child_proc");
            }
            exit(0);
        }
        else { // parent
            if (close(connfd) < 0) {
                err(1, "close for accept socket of parent");
            }
        }
    }
        
    return 0;
}
