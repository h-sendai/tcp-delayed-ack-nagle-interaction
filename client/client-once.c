#include <sys/ioctl.h>
#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_socket.h"
#include "readn.h"
#include "logUtil.h"
#include "get_num.h"

#define REQUEST_BYTE_DEFAULT 512
#define REPLY_BYTE_DEFEULT  1448

int usage()
{
    char msg[] = "Usage: client [options] remote_host\n"
                 "Connect to server and read data.  Display through put before exit.\n"
                 "\n"
                 "options:\n"
                 "-D      use TCP_NODELAY option\n"
                 "-s N    specify sleep_usec (defaut: 1000000)\n"
                 "-r N    request byte size (default: 512)\n"
                 "-R N    reply byte size (default: 1448)\n"
                 "-p PORT port number (default: 1234)\n"
                 "-h      display this help\n";

    fprintf(stderr, "%s", msg);
    return 0;
}

int main(int argc, char *argv[])
{
    int c;
    int n;
    char *remote;
    int port = 1234;
    int sleep_usec = 1000000;
    int use_nodelay = 0;
    int sockfd;
    int request_byte_size = REQUEST_BYTE_DEFAULT;
    int reply_byte_size   = REPLY_BYTE_DEFEULT;

    while ( (c = getopt(argc, argv, "Dhp:r:R:s:")) != -1) {
        switch (c) {
            case 'D':
                use_nodelay = 1;
                break;
            case 'r':
                request_byte_size = get_num(optarg);
                break;
            case 'R':
                reply_byte_size = get_num(optarg);
                break;
            case 'h':
                usage();
                exit(0);
                break;
            case 'p':
                port = strtol(optarg, NULL, 0);
                break;
            case 's':
                sleep_usec = strtol(optarg, NULL, 0);
                break;
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage();
        exit(1);
    }
    remote  = argv[0];

    sockfd = tcp_socket();

    if (connect_tcp(sockfd, remote, port) < 0) {
        errx(1, "connect_tcp");
    }
    
    if (use_nodelay) {
        if (set_so_nodelay(sockfd) < 0) {
            errx(1, "set_so_nodelay");
        }
    }

    char *request_buf = malloc(request_byte_size);
    if (request_buf == NULL) {
        err(1, "malloc for write_buf");
    }
    char *reply_buf  = malloc(reply_byte_size);
    if (reply_buf == NULL) {
        err(1, "malloc for read_buf");
    }

    for (int i = 0; i < 1; ++i) {
        /**** 1st data ****/
        n = write(sockfd, request_buf, request_byte_size);
        if (n < 0) {
            err(1, "client: write");
        }
        if (n == 0) {
            fprintfwt(stderr, "client: write return 0\n");
            exit(0);
        }
        fprintfwt(stderr, "client: wrote 1st request data\n");

        /**** 2nd data ****/
        n = write(sockfd, request_buf, request_byte_size);
        if (n < 0) {
            err(1, "client: write");
        }
        if (n == 0) {
            fprintfwt(stderr, "client: write return 0\n");
            exit(0);
        }
        fprintfwt(stderr, "client: wrote 2nd data\n");

        /**** read reply ****/
        n = readn(sockfd, reply_buf, reply_byte_size);
        if (n < 0) {
            errx(1, "readn");
        }
        else if (n == 0) {
            fprintfwt(stderr, "client: EOF\n");
        }
        fprintfwt(stderr, "client: got reply\n");

        usleep(sleep_usec);
    }

    return 0;
}
