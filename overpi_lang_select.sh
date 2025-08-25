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