#include <xc.h>
#include <math.h>
#include <string.h>
#define FCY 40000000UL
#include <libpic30.h>
#include "ssd1306.h"
#include "i2c_hal.h"

#pragma config FNOSC = FRC
#pragma config FWDTEN = OFF
#pragma config OSCIOFNC = OFF
#pragma config FCKSM = CSECMD
#pragma config POSCMD = NONE
#pragma config IESO = OFF
#pragma config ICS = PGD1
#pragma config JTAGEN = OFF

void ConfigureOscillator(void) {
    PLLFBD = 38;
    CLKDIVbits.PLLPOST = 0;
    CLKDIVbits.PLLPRE = 0;
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);
    while(OSCCONbits.COSC != 1);
    while(OSCCONbits.LOCK != 1);
}

typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    int16_t x, y;
} Point2D;

typedef struct {
    uint8_t v1, v2;
} Edge;

#define NUM_VERTICES 8
#define NUM_EDGES 12
#define SCALE 10.0f         
#define DISTANCE 100.0f

#define SCREEN_CENTER_X 64
#define SCREEN_CENTER_Y 32

Point3D cubeVertices[NUM_VERTICES] = {
    {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
    {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
};

Edge cubeEdges[NUM_EDGES] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

Point3D transformedVertices[NUM_VERTICES];
Point2D projectedVertices[NUM_VERTICES];
Point2D previousVertices[NUM_VERTICES];

float angleX = 0;
float angleY = 0;
float angleZ = 0;

float moveTime = 0;
int16_t centerX = 64;
int16_t centerY = 32;
int16_t prevCenterX = 64;
int16_t prevCenterY = 32;

// Variables para el tamaño dinámico del cubo
float cubeHalfWidth = 15.0f;   // Se actualizará dinámicamente
float cubeHalfHeight = 15.0f;  // Se actualizará dinámicamente

uint8_t prev_minX, prev_maxX, prev_minY, prev_maxY;
uint8_t curr_minX, curr_maxX, curr_minY, curr_maxY;

#define LUT_SIZE 64
float sinLUT[LUT_SIZE];
float cosLUT[LUT_SIZE];

// Velocidades de movimiento
float velX = 2.0f, velY = 1.5f;

void initLUT(void) {
    for(int i = 0; i < LUT_SIZE; i++) {
        float angle = (float)i * 6.28318f / (float)LUT_SIZE;
        sinLUT[i] = sinf(angle);
        cosLUT[i] = cosf(angle);
    }
}

float fastSin(float angle) {
    while(angle < 0) angle += 6.28318f;
    while(angle > 6.28318f) angle -= 6.28318f;
    int index = (int)((angle / 6.28318f) * LUT_SIZE) % LUT_SIZE;
    return sinLUT[index];
}

float fastCos(float angle) {
    return fastSin(angle + 1.5708f);
}

Point3D rotateX(Point3D p, float angle) {
    Point3D result;
    float c = fastCos(angle);
    float s = fastSin(angle);
    result.x = p.x;
    result.y = p.y * c - p.z * s;
    result.z = p.y * s + p.z * c;
    return result;
}

Point3D rotateY(Point3D p, float angle) {
    Point3D result;
    float c = fastCos(angle);
    float s = fastSin(angle);
    result.x = p.x * c + p.z * s;
    result.y = p.y;
    result.z = -p.x * s + p.z * c;
    return result;
}

Point3D rotateZ(Point3D p, float angle) {
    Point3D result;
    float c = fastCos(angle);
    float s = fastSin(angle);
    result.x = p.x * c - p.y * s;
    result.y = p.x * s + p.y * c;
    result.z = p.z;
    return result;
}

Point2D project(Point3D p) {
    Point2D result;
    float factor = DISTANCE / (DISTANCE + p.z);
    result.x = centerX + (int16_t)(p.x * factor);
    result.y = centerY + (int16_t)(p.y * factor);
    return result;
}

// Función para actualizar el tamaño del cubo basado en su proyección actual
void updateCubeSize(void) {
    int16_t minX = 127, maxX = 0;
    int16_t minY = 63, maxY = 0;
    
    // Encontrar los límites del cubo proyectado
    for(int i = 0; i < NUM_VERTICES; i++) {
        if(projectedVertices[i].x < minX) minX = projectedVertices[i].x;
        if(projectedVertices[i].x > maxX) maxX = projectedVertices[i].x;
        if(projectedVertices[i].y < minY) minY = projectedVertices[i].y;
        if(projectedVertices[i].y > maxY) maxY = projectedVertices[i].y;
    }
    
    // Calcular el radio desde el centro con margen de seguridad
    cubeHalfWidth = ((maxX - minX) / 2.0f) + 3;  // +3 píxeles de margen
    cubeHalfHeight = ((maxY - minY) / 2.0f) + 3; // +3 píxeles de margen
}

// Función mejorada para actualizar la posición con límites dinámicos
void updatePosition(void) {
    prevCenterX = centerX;
    prevCenterY = centerY;
    
    // Calcular nueva posición
    float newCenterX = centerX + velX;
    float newCenterY = centerY + velY;
    
    // Verificar límites horizontales considerando el tamaño del cubo
    if(newCenterX - cubeHalfWidth < 2) {
        velX = -velX;
        newCenterX = 2 + cubeHalfWidth;
    } else if(newCenterX + cubeHalfWidth > 125) {
        velX = -velX;
        newCenterX = 125 - cubeHalfWidth;
    }
    
    // Verificar límites verticales considerando el tamaño del cubo
    if(newCenterY - cubeHalfHeight < 2) {
        velY = -velY;
        newCenterY = 2 + cubeHalfHeight;
    } else if(newCenterY + cubeHalfHeight > 61) {
        velY = -velY;
        newCenterY = 61 - cubeHalfHeight;
    }
    
    // Aplicar la nueva posición
    centerX = (int16_t)newCenterX;
    centerY = (int16_t)newCenterY;
}

void calculateBoundingBox(Point2D* vertices, uint8_t* minX, uint8_t* maxX, uint8_t* minY, uint8_t* maxY) {
    *minX = 127;
    *maxX = 0;
    *minY = 63;
    *maxY = 0;
    
    for(int i = 0; i < NUM_VERTICES; i++) {
        // Asegurarse de que los vértices estén dentro de los límites de la pantalla
        int16_t x = vertices[i].x;
        int16_t y = vertices[i].y;
        
        if(x < 0) x = 0;
        if(x > 127) x = 127;
        if(y < 0) y = 0;
        if(y > 63) y = 63;
        
        if(x < *minX) *minX = x;
        if(x > *maxX) *maxX = x;
        if(y < *minY) *minY = y;
        if(y > *maxY) *maxY = y;
    }
    
    // Añadir margen de seguridad
    if(*minX > 2) *minX -= 2;
    if(*minY > 2) *minY -= 2;
    if(*maxX < 125) *maxX += 2;
    if(*maxY < 61) *maxY += 2;
}

void erasePreviousCube(void) {
    SSD1306_SetDirtyTracking(0);
    
    for(int i = 0; i < NUM_EDGES; i++) {
        uint8_t v1 = cubeEdges[i].v1;
        uint8_t v2 = cubeEdges[i].v2;
        
        Point2D p1 = previousVertices[v1];
        Point2D p2 = previousVertices[v2];
        
        if(p1.x >= 0 && p1.x < 128 && p1.y >= 0 && p1.y < 64 &&
           p2.x >= 0 && p2.x < 128 && p2.y >= 0 && p2.y < 64) {
            SSD1306_DrawLine(p1.x, p1.y, p2.x, p2.y, SSD1306_BLACK);
        }
    }
    
    // Borrar vértices
    for(int i = 0; i < NUM_VERTICES; i++) {
        Point2D p = previousVertices[i];
        if(p.x > 1 && p.x < 126 && p.y > 1 && p.y < 62) {
            SSD1306_SetPixel(p.x, p.y, SSD1306_BLACK);
            SSD1306_SetPixel(p.x+1, p.y, SSD1306_BLACK);
            SSD1306_SetPixel(p.x, p.y+1, SSD1306_BLACK);
            SSD1306_SetPixel(p.x+1, p.y+1, SSD1306_BLACK);
        }
    }
    
    SSD1306_SetDirtyTracking(1);
}

void renderCube(void) {
    float pulse = 1.0f + 0.35f * fastSin(angleY * 2.0f);
    
    // Transformar y proyectar vértices
    for(int i = 0; i < NUM_VERTICES; i++) {
        Point3D p;
        p.x = cubeVertices[i].x * SCALE * pulse;
        p.y = cubeVertices[i].y * SCALE * pulse;
        p.z = cubeVertices[i].z * SCALE * pulse;
        
        p = rotateX(p, angleX);
        p = rotateY(p, angleY);
        p = rotateZ(p, angleZ);
        
        transformedVertices[i] = p;
        projectedVertices[i] = project(p);
    }
    
    SSD1306_SetDirtyTracking(0);
    
    // Dibujar aristas
    for(int i = 0; i < NUM_EDGES; i++) {
        uint8_t v1 = cubeEdges[i].v1;
        uint8_t v2 = cubeEdges[i].v2;
        
        Point2D p1 = projectedVertices[v1];
        Point2D p2 = projectedVertices[v2];
        
        // Verificar que las líneas estén dentro de la pantalla
        if(p1.x >= 0 && p1.x < 128 && p1.y >= 0 && p1.y < 64 &&
           p2.x >= 0 && p2.x < 128 && p2.y >= 0 && p2.y < 64) {
            SSD1306_DrawLine(p1.x, p1.y, p2.x, p2.y, SSD1306_WHITE);
        }
    }
    
    // Dibujar vértices (puntos más grandes)
    for(int i = 0; i < NUM_VERTICES; i++) {
        Point2D p = projectedVertices[i];
        if(p.x > 1 && p.x < 126 && p.y > 1 && p.y < 62) {
            SSD1306_SetPixel(p.x, p.y, SSD1306_WHITE);
            SSD1306_SetPixel(p.x+1, p.y, SSD1306_WHITE);
            SSD1306_SetPixel(p.x, p.y+1, SSD1306_WHITE);
            SSD1306_SetPixel(p.x+1, p.y+1, SSD1306_WHITE);
        }
    }
    
    SSD1306_SetDirtyTracking(1);
}

int main(void) {
    ConfigureOscillator();
    AD1PCFGL = 0xFFFF;
    
    I2C_Init();
    __delay_ms(100);
    SSD1306_Init();
    initLUT();
    
    // Pantalla de inicio
    SSD1306_Clear();
    SSD1306_DrawString(18, 20, "MOVING CUBE", SSD1306_WHITE, SSD1306_BLACK);
    SSD1306_DrawString(10, 35, "Pulse & Motion", SSD1306_WHITE, SSD1306_BLACK);
    SSD1306_Display();
    __delay_ms(2000);
    
    SSD1306_Clear();
    SSD1306_Display();
    
    // Inicializar vértices previos
    for(int i = 0; i < NUM_VERTICES; i++) {
        previousVertices[i].x = 64;
        previousVertices[i].y = 32;
    }
    
    prev_minX = prev_maxX = 64;
    prev_minY = prev_maxY = 32;
    
    // Bucle principal
    while(1) {
        // Borrar el cubo anterior
        erasePreviousCube();
        
            // Calcular transformaciones del cubo
        float pulse = 1.0f + 0.35f * fastSin(angleY * 2.0f);
        
        // Transformar y proyectar vértices para obtener el tamaño actual
        for(int i = 0; i < NUM_VERTICES; i++) {
            Point3D p;
            p.x = cubeVertices[i].x * SCALE * pulse;
            p.y = cubeVertices[i].y * SCALE * pulse;
            p.z = cubeVertices[i].z * SCALE * pulse;
            
            p = rotateX(p, angleX);
            p = rotateY(p, angleY);
            p = rotateZ(p, angleZ);
            
            transformedVertices[i] = p;
            projectedVertices[i] = project(p);
        }
        
        // Actualizar el tamaño del cubo basado en la proyección actual
        updateCubeSize();
        
        // Actualizar la posición con los límites correctos
        updatePosition();
        
        // Re-proyectar con la posición corregida si cambió
        if(centerX != prevCenterX || centerY != prevCenterY) {
            for(int i = 0; i < NUM_VERTICES; i++) {
                projectedVertices[i] = project(transformedVertices[i]);
            }
        }
        
        // Renderizar el cubo en su nueva posición
        renderCube();
        
        // Calcular las áreas que necesitan actualización
        calculateBoundingBox(previousVertices, &prev_minX, &prev_maxX, &prev_minY, &prev_maxY);
        calculateBoundingBox(projectedVertices, &curr_minX, &curr_maxX, &curr_minY, &curr_maxY);
        
        // Determinar el área total que necesita actualización
        uint8_t final_minX = (prev_minX < curr_minX) ? prev_minX : curr_minX;
        uint8_t final_maxX = (prev_maxX > curr_maxX) ? prev_maxX : curr_maxX;
        uint8_t final_minY = (prev_minY < curr_minY) ? prev_minY : curr_minY;
        uint8_t final_maxY = (prev_maxY > curr_maxY) ? prev_maxY : curr_maxY;
        
        // Asegurar que los valores estén dentro de los límites de la pantalla
        if(final_minX > 127) final_minX = 0;
        if(final_maxX > 127) final_maxX = 127;
        if(final_minY > 63) final_minY = 0;
        if(final_maxY > 63) final_maxY = 63;
        
        uint8_t width = final_maxX - final_minX + 1;
        uint8_t height = final_maxY - final_minY + 1;
        
        // Actualizar solo la región modificada
        SSD1306_ClearDirty();
        SSD1306_MarkDirty(final_minX, final_minY, width, height);
        SSD1306_DisplayDirty();
        
        // Guardar los vértices actuales como previos para el siguiente frame
        for(int i = 0; i < NUM_VERTICES; i++) {
            previousVertices[i] = projectedVertices[i];
        }
        
        // Actualizar ángulos de rotación
        angleX += 0.03f;
        angleY += 0.06f;
        angleZ += 0.02f;
        
        // Mantener los ángulos dentro del rango 0-2π
        if(angleX > 6.28318f) angleX -= 6.28318f;
        if(angleY > 6.28318f) angleY -= 6.28318f;
        if(angleZ > 6.28318f) angleZ -= 6.28318f;
        
        // Pequeña variación en la velocidad para movimiento más orgánico (opcional)
        // Puedes comentar estas líneas si prefieres velocidad constante
        velX += (fastSin(angleY * 0.5f) * 0.05f);
        velY += (fastCos(angleX * 0.5f) * 0.05f);
        
        // Limitar las velocidades máximas
        if(velX > 3.0f) velX = 3.0f;
        if(velX < -3.0f) velX = -3.0f;
        if(velY > 2.5f) velY = 2.5f;
        if(velY < -2.5f) velY = -2.5f;
        
        // Asegurar velocidad mínima para evitar que se detenga
        if(velX > 0 && velX < 1.0f) velX = 1.0f;
        if(velX < 0 && velX > -1.0f) velX = -1.0f;
        if(velY > 0 && velY < 0.8f) velY = 0.8f;
        if(velY < 0 && velY > -0.8f) velY = -0.8f;
        
        // Delay para controlar la velocidad de animación
        __delay_ms(20);
    }
    
    return 0;
}