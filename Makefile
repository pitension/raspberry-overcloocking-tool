CC = g++
CFLAGS = -std=c++11 -Wall -Wextra
LIBS = `pkg-config gtkmm-3.0 --cflags --libs`
TARGET = overpi
SRC = src/overpi.cpp

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	chmod +x /usr/local/bin/$(TARGET)

.PHONY: clean install