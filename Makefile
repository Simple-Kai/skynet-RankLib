Name = RankCore.so
Root = ../../../# server/
Include = -I$(Root)/skynet/3rd/lua/ -I./ -I$(Root)/skynet/skynet-src
CC = gcc $(Include)
ToPath = ../../public/luaclib/$(Name)

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,${SRC})

all:$(Name)
$(Name):$(OBJ)
	$(CC) -o $(Name) $(OBJ) -shared
	$(CC) -o $(ToPath) $(OBJ) -shared
	rm -rf *.o
${OBJ}:${SRC}
	$(CC) -c ${SRC}

clean:
	rm -rf *.o $(Name)


