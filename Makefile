CXXFLAGS = -O2 -Wall -std=c++11 ${shell pkg-config --cflags gtkmm-3.0}
LDFLAGS = ${shell pkg-config --libs gtkmm-3.0}
BIN = xr25_diag
OBJS = XR25streamreader.o

all: ${BIN}

clean:
	rm -f *~ \#*\# *.o ${BIN}
.PHONY: all clean

${BIN}: ${OBJS}
	g++ ${LDFLAGS} -o $@ $^

%.o: %.cc
	g++ -c ${CXXFLAGS} -o $@ $^
