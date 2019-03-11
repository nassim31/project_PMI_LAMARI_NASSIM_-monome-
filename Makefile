CC=gcc
CFLAGS=-O3 
LDFLAGS=-Wl,-rpath,$(PWD) 

TARGET=test_values test_perf server
 
all: $(TARGET)

libpmi.so : pmi.c
	$(CC) $(CFLAGS)  -fpic -shared $^ -o $@

test_values: test_values.c libpmi.so
	$(CC) $(CFLAGS) $(LDFLAGS) -L. test_values.c -o $@ -lpmi

test_perf: test_perf.c libpmi.so
	$(CC) $(CFLAGS) $(LDFLAGS) -L. test_perf.c -o $@ -lpmi

server:server.c 

	$(CC) server.c -o server 

clean:
	rm -fr $(TARGET) *.so

