#include "readn.h"
#include "my_socket.h"
#include "logUtil.h"

extern int use_quick_ack;
extern int verbose;

ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
#ifdef __linux__
        if (use_quick_ack) {
            if (set_so_quickack(fd, 1) < 0) {
                exit(1);
            }
        }
        if (verbose && (! use_quick_ack)) {
            int quick_ack_state = get_so_quickack(fd);
            if (quick_ack_state == 1) {
                fprintfwt(stderr, "server: (info) quickack enabled (not specified in commandline)\n");
            }
        }
#endif
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}
