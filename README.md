RPi Overclock GUI
Graphical application for Raspberry Pi overclocking written in C++ with GTKmm.

Features
CPU/GPU frequency adjustment

Voltage control

Real-time temperature monitoring

Throttling detection

Pre-configured profiles

Multi-language support (English/Spanish)

Requirements
Raspberry Pi (tested on RPi 400)

GTKmm 3.0

Superuser privileges (sudo)

Installation
bash
# Install dependencies
sudo apt-get install libgtkmm-3.0-dev

# Compile
make

# Compile translations
make translations

# Install
sudo make install
Usage
Direct execution:
bash
# Run with interactive language selector
./overpi_lang_select.sh

# Run directly in Spanish
sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi

# Run directly in English
sudo LANG=C LC_ALL=C ./overpi
From compiled code:
bash
# Use the language selection script
chmod +x overpi_lang_select.sh
./overpi_lang_select.sh

# Or run manually
sudo ./overpi
Functionality
Select a profile from the dropdown list

Click "Apply Hot" for temporary changes

Use "Apply Permanently" for boot-time changes

Monitor system metrics in real-time

Precautions
⚠️ WARNING: Extreme overclocking can damage your hardware.
Always use adequate cooling and understand the risks.

Included Scripts
Language Selector (overpi_lang_select.sh)
bash
#!/bin/bash
echo "Select language / Seleccionar idioma:"
echo "1) English"
echo "2) Español"
read -p "Choice [1-2]: " choice

case $choice in
    1) sudo LANG=C LC_ALL=C ./overpi ;;
    2) sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi ;;
    *) echo "Invalid choice, using English"; sudo LANG=C LC_ALL=C ./overpi ;;
esac
Dependency Installer (scripts/install_dependencies.sh)
bash
#!/bin/bash
echo "Installing dependencies for RPi Overclock GUI..."
sudo apt-get update
sudo apt-get install -y libgtkmm-3.0-dev
echo "Dependencies installed successfully."
Translation Compiler (scripts/compile_translations.sh)
bash
#!/bin/bash
echo "Compiling translations..."

# Create output directories
sudo mkdir -p /usr/share/locale/es/LC_MESSAGES/
sudo mkdir -p /usr/share/locale/en/LC_MESSAGES/

# Compile Spanish
msgfmt po/es/overpi.po -o /usr/share/locale/es/LC_MESSAGES/overpi.mo

# Compile English
msgfmt po/en/overpi.po -o /usr/share/locale/en/LC_MESSAGES/overpi.mo

echo "Translations compiled successfully!"
echo "Spanish: /usr/share/locale/es/LC_MESSAGES/overpi.mo"
echo "English: /usr/share/locale/en/LC_MESSAGES/overpi.mo"
Makefile
makefile
CC = g++
CFLAGS = -std=c++11 -Wall -Wextra
LIBS = `pkg-config gtkmm-3.0 --cflags --libs` -lintl
TARGET = overpi
SRC = overpi.cpp

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

translations:
	@echo "Compiling translations..."
	@mkdir -p /usr/share/locale/es/LC_MESSAGES/
	@mkdir -p /usr/share/locale/en/LC_MESSAGES/
	msgfmt po/es/overpi.po -o /usr/share/locale/es/LC_MESSAGES/overpi.mo
	msgfmt po/en/overpi.po -o /usr/share/locale/en/LC_MESSAGES/overpi.mo
	@echo "Translations compiled!"

install: $(TARGET) translations
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	sudo cp overpi_lang_select.sh /usr/local/bin/
	sudo chmod +x /usr/local/bin/overpi_lang_select.sh

run:
	./overpi_lang_select.sh

run-es:
	sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi

run-en:
	sudo LANG=C LC_ALL=C ./overpi

clean:
	rm -f $(TARGET)

.PHONY: clean install translations run run-es run-en
Project Structure
text
overpi/
├── src/                 # Source code
│   └── overpi.cpp      # Main source file
├── po/                 # Translations
│   ├── es/             # Spanish
│   │   └── overpi.po   # Spanish translation
│   └── en/             # English
│       └── overpi.po   # English translation
├── scripts/            # Utility scripts
│   └── install_dependencies.sh
├── overpi_lang_select.sh # Language selector
├── Makefile           # Build system
└── README.md          # This file
Useful Commands
bash
# Quick compile and run
make && ./overpi_lang_select.sh

# Compile only
make

# Translations only
make translations

# System-wide installation
sudo make install

# Clean build
make clean
Troubleshooting
Locale errors:
bash
# If UTF-8 locales fail, use:
sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi   # Spanish
sudo LANG=C LC_ALL=C ./overpi             # English
Permission errors:
bash
# Ensure execution permissions
chmod +x overpi_lang_select.sh
chmod +x scripts/*.sh
License
GNU General Public License v3.0

Contributing
Contributions are welcome. Please ensure to:

Update translations in /po/

Test both languages

Update this README

Support
If you encounter issues:

Verify all dependencies are installed

Run with sudo for overclocking permissions

Use the language selector to avoid locale issues

Important!: Always monitor temperatures when using extreme overclocking profiles.


********************************************

ESPAÑOL / SPANISH 

********************************************

RPi Overclock GUI
Aplicación gráfica para overclocking de Raspberry Pi escrita en C++ con GTKmm.

Características
Ajuste de frecuencia CPU/GPU

Control de voltaje

Monitoreo de temperatura en tiempo real

Detección de throttling

Perfiles preconfigurados

Soporte para múltiples idiomas (Español/Inglés)

Requisitos
Raspberry Pi (probado en RPi 400)

GTKmm 3.0

Permisos de superusuario (sudo)

Instalación
bash
# Instalar dependencias
sudo apt-get install libgtkmm-3.0-dev

# Compilar
make

# Compilar traducciones
make translations

# Instalar
sudo make install
Uso
Ejecución directa:
bash
# Ejecutar con selector de idioma interactivo
./overpi_lang_select.sh

# Ejecutar directamente en español
sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi

# Ejecutar directamente en inglés
sudo LANG=C LC_ALL=C ./overpi
Desde el código compilado:
bash
# Usar el script de selección de idioma
chmod +x overpi_lang_select.sh
./overpi_lang_select.sh

# O ejecutar manualmente
sudo ./overpi
Funcionalidades
Selecciona un perfil de la lista desplegable

Haz clic en "Aplicar en Caliente" para cambios temporales

Usa "Aplicar Permanentemente" para cambios en el inicio

Monitorea las métricas del sistema en tiempo real

Precauciones
⚠️ ADVERTENCIA: El overclocking extremo puede dañar tu hardware.
Usa siempre refrigeración adecuada y comprende los riesgos.

Scripts incluidos
Selector de Idioma (overpi_lang_select.sh)
bash
#!/bin/bash
echo "Select language / Seleccionar idioma:"
echo "1) English"
echo "2) Español"
read -p "Choice [1-2]: " choice

case $choice in
    1) sudo LANG=C LC_ALL=C ./overpi ;;
    2) sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi ;;
    *) echo "Invalid choice, using English"; sudo LANG=C LC_ALL=C ./overpi ;;
esac
Instalador de Dependencias (scripts/install_dependencies.sh)
bash
#!/bin/bash
echo "Instalando dependencias para RPi Overclock GUI..."
sudo apt-get update
sudo apt-get install -y libgtkmm-3.0-dev
echo "Dependencias instaladas correctamente."
Compilador de Traducciones (scripts/compile_translations.sh)
bash
#!/bin/bash
echo "Compiling translations..."

# Crear directorios de salida
sudo mkdir -p /usr/share/locale/es/LC_MESSAGES/
sudo mkdir -p /usr/share/locale/en/LC_MESSAGES/

# Compilar español
msgfmt po/es/overpi.po -o /usr/share/locale/es/LC_MESSAGES/overpi.mo

# Compilar inglés
msgfmt po/en/overpi.po -o /usr/share/locale/en/LC_MESSAGES/overpi.mo

echo "Translations compiled successfully!"
echo "Spanish: /usr/share/locale/es/LC_MESSAGES/overpi.mo"
echo "English: /usr/share/locale/en/LC_MESSAGES/overpi.mo"
Makefile
makefile
CC = g++
CFLAGS = -std=c++11 -Wall -Wextra
LIBS = `pkg-config gtkmm-3.0 --cflags --libs` -lintl
TARGET = overpi
SRC = overpi.cpp

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

translations:
	@echo "Compiling translations..."
	@mkdir -p /usr/share/locale/es/LC_MESSAGES/
	@mkdir -p /usr/share/locale/en/LC_MESSAGES/
	msgfmt po/es/overpi.po -o /usr/share/locale/es/LC_MESSAGES/overpi.mo
	msgfmt po/en/overpi.po -o /usr/share/locale/en/LC_MESSAGES/overpi.mo
	@echo "Translations compiled!"

install: $(TARGET) translations
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	sudo cp overpi_lang_select.sh /usr/local/bin/
	sudo chmod +x /usr/local/bin/overpi_lang_select.sh

run:
	./overpi_lang_select.sh

run-es:
	sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi

run-en:
	sudo LANG=C LC_ALL=C ./overpi

clean:
	rm -f $(TARGET)

.PHONY: clean install translations run run-es run-en
Estructura del proyecto
text
overpi/
├── src/                 # Código fuente
│   └── overpi.cpp      # Archivo principal
├── po/                 # Traducciones
│   ├── es/             # Español
│   │   └── overpi.po   # Traducción española
│   └── en/             # Inglés
│       └── overpi.po   # Traducción inglesa
├── scripts/            # Scripts utilitarios
│   └── install_dependencies.sh
├── overpi_lang_select.sh # Selector de idioma
├── Makefile           # Sistema de compilación
└── README.md          # Este archivo
Comandos útiles
bash
# Compilar y ejecutar rápido
make && ./overpi_lang_select.sh

# Solo compilar
make

# Solo traducciones
make translations

# Instalar system-wide
sudo make install

# Limpiar compilación
make clean
Solución de problemas
Error de locales:
bash
# Si fallan los locales UTF-8, usar:
sudo LANG=C.utf8 LC_ALL=C.utf8 ./overpi   # Español
sudo LANG=C LC_ALL=C ./overpi             # Inglés
Error de permisos:
bash
# Asegurar permisos de ejecución
chmod +x overpi_lang_select.sh
chmod +x scripts/*.sh
Licencia
GNU General Public License v3.0

Contribuir
Las contribuciones son bienvenidas. Por favor, asegúrate de:

Actualizar las traducciones en /po/

Probar ambos idiomas

Actualizar este README

Soporte
Si encuentras problemas:

Verifica que todas las dependencias están instaladas

Ejecuta con sudo para permisos de overclocking

Usa el selector de idioma para evitar problemas de locale

¡Importante!: Siempre monitoriza las temperaturas cuando uses perfiles de overclocking extremo.