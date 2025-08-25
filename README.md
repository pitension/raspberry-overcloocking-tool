# RPi Overclock GUI

Aplicación gráfica para overclocking de Raspberry Pi escrita en C++ con GTKmm.

## Características

- Ajuste de frecuencia CPU/GPU
- Control de voltaje
- Monitoreo de temperatura en tiempo real
- Detección de throttling
- Perfiles preconfigurados

## Requisitos

- Raspberry Pi (probado en RPi 400)
- GTKmm 3.0
- sudo/root privileges

## Instalación

```bash
# Instalar dependencias
sudo apt-get install libgtkmm-3.0-dev

# Compilar
make

# Ejecutar (necesita root)
sudo ./overpi

Uso
Selecciona un perfil de la lista desplegable

Haz clic en "Aplicar en Caliente" para cambios temporales

Usa "Aplicar Permanentemente" para cambios en el inicio

Monitorea las métricas del sistema en tiempo real

Precauciones
⚠️ ADVERTENCIA: El overclocking extremo puede dañar tu hardware.
Usa siempre refrigeración adecuada y comprende los riesgos.


### Script de instalación de dependencias (scripts/install_dependencies.sh)
```bash
#!/bin/bash
echo "Instalando dependencias para RPi Overclock GUI..."
sudo apt-get update
sudo apt-get install -y libgtkmm-3.0-dev
echo "Dependencias instaladas correctamente."