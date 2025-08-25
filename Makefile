CC = g++
CFLAGS = -std=c++11 -Wall -Wextra
LIBS = `pkg-config gtkmm-3.0 --cflags --libs` -lintl
TARGET = overpi
SRC = overpi.cpp
PO_DIR = po

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

translations:
	@echo "Compiling translations..."
	@mkdir -p /usr/share/locale/es/LC_MESSAGES/
	@mkdir -p /usr/share/locale/en/LC_MESSAGES/
	msgfmt $(PO_DIR)/es/overpi.po -o /usr/share/locale/es/LC_MESSAGES/overpi.mo
	msgfmt $(PO_DIR)/en/overpi.po -o /usr/share/locale/en/LC_MESSAGES/overpi.mo
	@echo "Translations compiled!"

install: $(TARGET) translations
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean install translations