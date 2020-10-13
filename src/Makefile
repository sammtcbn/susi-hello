ALL_BIN=susi-hello

ALL_OBJ=susi-hello.o

all: $(ALL_BIN)

susi-hello: $(ALL_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

susi-hello: susi-hello.cpp
	$(CC) $(CFLAGS) -c $<
