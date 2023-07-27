PROG  = client
PROG2 = client-once
CPPFLAGS += -I.
CFLAGS = -g -O2 -Wall -std=gnu99

all: $(PROG) $(PROG2)

OBJS += my_socket.o
OBJS += readn.o
OBJS += logUtil.o
OBJS += get_num.o
OBJS += $(PROG).o

OBJS2 += my_socket.o
OBJS2 += readn.o
OBJS2 += logUtil.o
OBJS2 += get_num.o
OBJS2 += $(PROG2).o

$(PROG):  $(OBJS)
$(PROG2): $(OBJS2)

clean:
	rm -f *.o $(PROG) $(PROG2)
