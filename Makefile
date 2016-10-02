vpath %.c src
vpath %.h src

#CFLAGS = -I src
OBJDIR = ./objects
LIBDIR = -L/Applications/MATLAB_R2015b.app/bin/maci64

PROG := OFDMA_promac
SRCS := main.c bufferManager.c frameGen.c idle.c initialization.c result.c success.c setting.c probability.c perModel.c
OBJS := $(addprefix $(OBJDIR)/, $(SRCS:%.c=%.o))
DEPS := $(SRCS:%.c=%.d)
LIBS := -leng -lmx

CC := gcc

all: $(PROG)

-include $(DEPS)

$(PROG): $(OBJS)
	$(CC) -Wall -g -O0 -o $@ $^ $(LIBDIR) $(LIBS)

$(OBJDIR)/%.o: %.c
	@[ -d $(OBJDIR) ]
	$(CC) -Iinclude -g -MMD -MP -o $@ -c $<

clean:
	rm -f $(OBJS) $(DEPS)

#	gcc -Wall -g -O0 $^ -o $@

#main.o: src/main.c
#	gcc -g -c $^

#bufferManager.o: src/bufferManager.c
#	gcc -g -c $^

#collision.o: src/collision.c
#	gcc -g -c $^

#frameGen.o: src/frameGen.c
#	gcc -g -c $^

#idle.o: src/idle.c
#	gcc -g -c $^

#initialization.o: src/initialization.c
#	gcc -c -g $^

#result.o: src/result.c
#	gcc -g -c $^

#success.o: src/success.c
#	gcc -g -c $^

#debug.o: src/debug.c
#	gcc -g -c $^

#setting.o: src/setting.c
#	gcc -g -c $^

#clean:
#	rm main.o bufferManager.o collision.o frameGen.o idle.o initialization.o result.o success.o debug.o
