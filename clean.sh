#!/bin/bash
echo "Cleaning duplicate entries in PO files..."

# Función para limpiar duplicados
clean_po_file() {
    local file=$1
    local temp_file="${file}.temp"
    
    echo "Cleaning $file"
    awk '
    /^msgid/ {
        if (msgid_seen[$0]++) {
            skip_block = 1
            next
        } else {
            skip_block = 0
        }
    }
    /^msgstr/ {
        if (skip_block) next
    }
    !skip_block { print }
    ' "$file" > "$temp_file"
    
    mv "$temp_file" "$file"
    echo "Cleaned: $file"
}

# Limpiar ambos archivos
clean_po_file "po/en/overpi.po"
clean_po_file "po/es/overpi.po"

echo "PO files cleaned successfully!"