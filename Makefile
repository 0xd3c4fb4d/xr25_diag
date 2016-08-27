XR25DIAG_VERSION = 1.0.0
CXXFLAGS = -pipe -O2 -Wall -std=c++11 -DXR25DIAG_VERSION=\"${XR25DIAG_VERSION}\" \
           ${shell pkg-config --cflags gtkmm-3.0}
LDFLAGS = ${shell pkg-config --libs gtkmm-3.0} -pthread
BIN = xr25_diag
OBJS = XR25streamreader.o UI.o CairoGauge.o main.o

all: ${BIN}

clean:
	rm -f *~ \#*\# *.o ${BIN}
.PHONY: all clean

${BIN}: ${OBJS}
	g++ ${LDFLAGS} -o $@ $^

%.o: %.cc
	g++ -c ${CXXFLAGS} -o $@ $^
