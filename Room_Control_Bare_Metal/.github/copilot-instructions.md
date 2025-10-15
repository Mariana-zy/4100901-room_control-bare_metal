# Copilot Instructions for Room_Control_Bare_Metal

## Visión General
Este proyecto es un firmware bare-metal para la placa STM32L476RGTX, orientado al control de habitaciones. El código está organizado en módulos por funcionalidad (GPIO, NVIC, RCC, UART, Systick, etc.), siguiendo una arquitectura modular y separando drivers, lógica principal y arranque.

## Estructura Clave
- `Src/`: Implementaciones de módulos (ej. `gpio.c`, `nvic.c`, `uart.c`).
- `Inc/`: Cabeceras asociadas a cada módulo.
- `Startup/`: Código de arranque específico del microcontrolador.
- `build/`: Archivos generados por CMake y binarios.
- `CMakeLists.txt`, `CMakePresets.json`: Configuración de compilación.

## Flujo de Desarrollo
- **Compilación y Flasheo:**
  - Usar la tarea `Build + Flash` para compilar y grabar el firmware en la placa.
  - El binario principal se genera en `build/debug/Room_Control_Bare_Metal.bin`.
  - El flasheo utiliza `STM32_Programmer_CLI` vía SWD.
- **Depuración:**
  - El entorno soporta depuración con GDB y tareas de CMake.
- **Limpieza:**
  - Ejecutar la tarea `CMake: clean rebuild` para limpiar y recompilar desde cero.

## Convenciones Específicas
- Cada módulo tiene su archivo `.c` en `Src/` y su `.h` en `Inc/`.
- El archivo `main.c` es el punto de entrada y orquesta la inicialización de periféricos.
- Los nombres de funciones siguen el patrón `<modulo>_<función>` (ejemplo: `gpio_init`).
- No se utiliza RTOS ni HAL: todo el acceso a hardware es directo y explícito.
- Los archivos `.s` y `.s.obj` son para rutinas de arranque y bajo nivel.

## Ejemplo de Flujo de Inicialización
1. `main.c` llama a funciones de inicialización de cada módulo.
2. Los módulos configuran registros directamente (ver `gpio.c`, `rcc.c`).
3. El sistema entra en el bucle principal (`while(1)` en `main.c`).

## Integraciones y Dependencias
- **Herramientas externas:**
  - `STM32_Programmer_CLI` para flasheo.
  - `gcc-arm-none-eabi` para compilación cruzada.
- **No hay dependencias de software externas** (sin HAL, sin RTOS, sin librerías de terceros).

## Ejemplo de Comando de Flasheo
```powershell
STM32_Programmer_CLI --connect port=swd --download build/debug/Room_Control_Bare_Metal.bin -hardRst -rst --start
```

## Recomendaciones para Agentes AI
- Priorizar la modularidad y el acceso directo a registros.
- Mantener la separación entre lógica de inicialización y bucle principal.
- Seguir el patrón de nombres y la estructura de archivos existente.
- Validar cambios compilando y flasheando con las tareas predefinidas.

## Archivos Clave para Referencia
- `Src/main.c`, `Src/gpio.c`, `Inc/gpio.h`, `Startup/startup_stm32l476rgtx.s`, `CMakeLists.txt`

---
¿Hay alguna sección que requiera mayor detalle o aclaración?