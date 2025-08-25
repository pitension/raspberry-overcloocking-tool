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