ALL_BIN=susi-hello

ALL_OBJ=susi-hello.o

all: $(ALL_BIN)

susi-hello: $(ALL_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ -ldl

susi-hello.o: susi-hello.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(ALL_BIN) $(ALL_OBJ) *.o
