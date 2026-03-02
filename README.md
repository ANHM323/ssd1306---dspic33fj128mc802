
# SSD1306 para dsPIC33FJ128MC802

Librería optimizada para controlar pantallas OLED **SSD1306 (128x64)** desde el microcontrolador **dsPIC33FJ128MC802**. Incluye primitivas gráficas básicas (líneas, círculos, rectángulos, texto) y está diseñada para comunicación **I2C a 400kHz** con algoritmos eficientes de computación gráfica. Perfecta para proyectos embebidos que requieren interfaces visuales compactas y rápidas.

## ✨ Características
- **Comunicación I2C optimizada** a 400kHz (máxima velocidad estable para dsPIC33).
- **Primitivas gráficas**: líneas (Bresenham), círculos, rectángulos, bitmap.
- **Texto**: Fuentes fijas de 6x8 y 8x8 píxeles.
- **Buffer de pantalla completo** (1024 bytes) con funciones de renderizado rápido.
- **Bajo consumo de RAM/Flash**: ~2KB Flash, ~1.2KB RAM.
- **Compatible con XC16** (MPLAB X IDE) y MikroC PRO.
- **Ejemplos incluidos** para inicialización y demos gráficas.

## 📋 Requisitos
- Microcontrolador: **dsPIC33FJ128MC802** (familia PIC 16-bit).
- Pantalla: **SSD1306 128x64** (I2C, 3.3V).
- Herramientas: MPLAB X IDE v6+, XC16 v1.70+.
- Pines I2C: SDA/SCL configurables (por defecto RB0/RB1).

## 🚀 Instalación Rápida
```
1. Copia la carpeta 'ssd1306' a tu proyecto MPLAB X.
2. Incluye en main.c:
   #include "ssd1306.h"
3. Configura I2C en tu código:
   ssd1306_init();
4. ¡Listo para dibujar!
```

## 💻 Uso Básico
```c
#include "ssd1306.h"

int main(void) {
    system_init();  // Tu init de dsPIC
    ssd1306_init(); // Inicializa pantalla
    
    ssd1306_clear();           // Limpia buffer
    ssd1306_draw_string(0, 0, "Hola dsPIC!");  // Texto
    ssd1306_draw_line(10, 10, 120, 50);        // Línea
    ssd1306_draw_circle(64, 32, 20);           // Círculo
    ssd1306_update();          // Envía a pantalla
    
    while(1);  // Loop principal
}
```

## 📖 API Principal
| Función | Descripción | Parámetros |
|---------|-------------|------------|
| `ssd1306_init()` | Inicializa I2C y pantalla | - |
| `ssd1306_clear()` | Limpia buffer (negro) | - |
| `ssd1306_update()` | Refresca pantalla (~10ms) | - |
| `ssd1306_draw_pixel(x,y)` | Píxel individual | x:0-127, y:0-63 |
| `ssd1306_draw_line(x1,y1,x2,y2)` | Línea Bresenham | Coordenadas |
| `ssd1306_draw_circle(x,y,r)` | Círculo outline | Centro + radio |
| `ssd1306_draw_string(x,y,"txt")` | Texto 6x8 | Pos + cadena |
| `ssd1306_draw_bitmap(x,y,bmp,w,h)` | Imagen bitmap | Datos + dims |

## ⚙️ Configuración Avanzada
Edita `ssd1306_config.h`:
```c
#define SSD1306_I2C_ADDR    0x3C  // Dirección I2C
#define SSD1306_SDA_PIN     LATB0 // Pin SDA
#define SSD1306_SCL_PIN     LATB1 // Pin SCL
#define FONT_SIZE           6     // 6px o 8px
```

## 🔧 Ejemplos
- `demo_basic.c`: Pruebas de todas las primitivas.
- `demo_clock.c`: Reloj digital analógico.
- `demo_bars.c`: Barras de progreso para monitoreo.

## 📄 Licencia
Esta librería está licenciada bajo **MIT License**.  
Ver archivo [LICENSE](LICENSE) para detalles completos.  
```
Copyright (c) 2026 [Tu Nombre]
Licensed under the MIT License.
```

## 🤝 Contribuir
1. Fork el repositorio.
2. Crea feature branch (`git checkout -b feature/nueva-primitiva`).
3. Commit cambios (`git commit -m 'Agrega rotación de texto'`).
4. Push al branch (`git push origin feature/nueva-primitiva`).
5. Abre Pull Request.

## 🐛 Problemas
- **Pantalla no enciende**: Verifica pull-ups 4.7kΩ en SDA/SCL.
- **Datos corruptos**: Reduce velocidad I2C a 100kHz en config.
- **Texto pequeño**: Cambia `FONT_SIZE` a 8.

¡Reporta issues en GitHub!

---

**Desarrollado para proyectos embebidos con dsPIC33 · Optimizado para XC16 · 100% ANSI C**