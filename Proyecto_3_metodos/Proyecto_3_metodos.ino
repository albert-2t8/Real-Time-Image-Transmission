#include <WiFi.h>
#include "esp_heap_caps.h"  
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <math.h>




const char* ssid     = "WI-FI";
const char* password = "aaaaaaaa";

WiFiServer server(5000);

TFT_eSPI tft = TFT_eSPI();  

void setup() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 4);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);

    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
    }
    tft.println("\nConectado a WiFi");
    tft.print("IP address:\t");
    tft.println(WiFi.localIP());
    server.begin();
    delay(5000);
    
}

void loop() {
    WiFiClient client = server.available();
    tft.setCursor(0, 0, 4);
    tft.fillScreen(TFT_BLACK);
    if (client) {
        tft.println("Cliente conectado\n");

        // Recibir dimensiones
        int alto, ancho;
        client.readBytes((char*)&alto, sizeof(int));
        client.readBytes((char*)&ancho, sizeof(int));
        tft.printf("Ancho: %d Alto: %d", ancho, alto);

        // Recibir semillas
        int semilla_x, semilla_y;
        client.readBytes((char*)&semilla_x, sizeof(int));
        client.readBytes((char*)&semilla_y, sizeof(int));
        tft.printf("\nseed_x: %d seed_y: %d", semilla_x, semilla_y);


        
        // Reservar memoria en PSRAM

        uint32_t** imagen = (uint32_t**)heap_caps_malloc(sizeof(uint32_t*) * alto, MALLOC_CAP_SPIRAM);
        if (!imagen) {
            tft.println("\nFallo al reservar PSRAM para filas.");
            return;
        }

        for (int i = 0; i < alto; i++) {
            imagen[i] = (uint32_t*)heap_caps_malloc(sizeof(uint32_t) * ancho, MALLOC_CAP_SPIRAM);
            if (!imagen[i]) {
                tft.printf("Fallo al reservar PSRAM para fila %d\n", i);
                return;
            }
        }

        // Construir imagen
       
        bool imagen_recibida = false;

        while (!imagen_recibida) {
          int contador = 0;

          for (int fila = 0; fila < alto; fila++) {
            for (int col = 0; col < ancho; col++) {
                while (client.available() < sizeof(uint32_t)) {
                   delay(1);  // Espera pasiva
                }
                uint32_t valor_pixel;
                client.readBytes((char*)&valor_pixel, sizeof(uint32_t));
                imagen[fila][col] = valor_pixel;
                contador++;
              }
          }

          if (contador == alto * ancho) {
            imagen_recibida = true;
            tft.println("\nImagen recibida.");
            //tft.printf("\nNumero de pixels: %d", contador);
          } else {
            tft.println("\nNo se recibieron todos los píxeles.");
            // Espera a que el cliente vuelva a enviar
           }
        }

        ///////// Procedimiento 1

        uint32_t** imagen_ecualizada = (uint32_t**)heap_caps_malloc(sizeof(uint32_t*) * alto, MALLOC_CAP_SPIRAM);
        if (!imagen_ecualizada) {
            tft.println("\nFallo al reservar PSRAM para filas ecualizada.");
            return;
        }


        for (int i = 0; i < alto; i++) {
            imagen_ecualizada[i] = (uint32_t*)heap_caps_malloc(sizeof(uint32_t) * ancho, MALLOC_CAP_SPIRAM);
            if (!imagen_ecualizada[i]) {
                tft.printf("Fallo al reservar PSRAM para fila  ecualizada %d\n", i);
                return;
            }
        }



        /// Procedimiento 2
        uint32_t** bordes = (uint32_t**)heap_caps_malloc(sizeof(uint32_t*) * alto, MALLOC_CAP_SPIRAM);
        if (!bordes) {
            tft.println("\nFallo al reservar PSRAM para Procedimiento_2.");
            return;
        }

        for (int i = 0; i < alto; i++) {
            bordes[i] = (uint32_t*)heap_caps_malloc(sizeof(uint32_t) * ancho, MALLOC_CAP_SPIRAM);
            if (!bordes[i]) {
                tft.printf("Fallo al reservar PSRAM para fila  Procedimiento_2 %d\n", i);
                return;
            }
        }


        

        

        // Ecualizar imagen
        Ecualizar_imagen(imagen_ecualizada, imagen, alto, ancho);

      

        ///////////////////////////////////////////////////////////////////////////////////////////7
        //// Enviar imagen 

        uint8_t* bufferFila = (uint8_t*)malloc(ancho * sizeof(uint32_t));
        for (int fila = 0; fila < alto; fila++) {
            memcpy(bufferFila, (uint8_t*)imagen_ecualizada[fila], ancho * sizeof(uint32_t));
            client.write(bufferFila, ancho * sizeof(uint32_t));  
           }
        free(bufferFila);

        tft.println("Imagen 1 enviada.");

        // Liberar memoria
        for (int i = 0; i < alto; i++) {
            free(imagen_ecualizada[i]);
        }
        free(imagen_ecualizada);

        ///////// Algoritmo Puga

        algoritmoPuga(bordes, imagen, alto, ancho);

         ////////////////////////////////////////////////////////////////////////////////
         ///////////////// Enviar segundo procedimiento

        uint8_t* bufferFila_1 = (uint8_t*)malloc(ancho * sizeof(uint32_t));
        for (int fila = 0; fila < alto; fila++) {
            memcpy(bufferFila_1, (uint8_t*)bordes[fila], ancho * sizeof(uint32_t));
            client.write(bufferFila_1, ancho * sizeof(uint32_t));  
           }
        free(bufferFila_1);  

        tft.println("Imagen 2 enviada.");

        // Liberar memoria
        for (int i = 0; i < alto; i++) {
            free(bordes[i]);
        }
        free(bordes);



        /////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////// Procedimiento 4

        uint32_t** imagen_crecimiento = (uint32_t**)heap_caps_malloc(sizeof(uint32_t*) * alto, MALLOC_CAP_SPIRAM);
        if (!imagen_crecimiento) {
            tft.println("\nFallo al reservar PSRAM para filas ecualizada.");
            return;
        }


        for (int i = 0; i < alto; i++) {
            imagen_crecimiento[i] = (uint32_t*)heap_caps_malloc(sizeof(uint32_t) * ancho, MALLOC_CAP_SPIRAM);
            if (!imagen_crecimiento[i]) {
                tft.printf("Fallo al reservar PSRAM para fila  ecualizada %d\n", i);
                return;
            }
        }





        crecimiento_imagen(imagen_crecimiento, imagen, alto, ancho, semilla_x, semilla_y);

        uint8_t* bufferFila_3 = (uint8_t*)malloc(ancho * sizeof(uint32_t));
        for (int fila = 0; fila < alto; fila++) {
            memcpy(bufferFila_3, (uint8_t*)imagen_crecimiento[fila], ancho * sizeof(uint32_t));
            client.write(bufferFila_3, ancho * sizeof(uint32_t));  
           }
        free(bufferFila_3);

        tft.println("Imagen 4 enviada.");

        // Liberar memoria
        for (int i = 0; i < alto; i++) {
            free(imagen_crecimiento[i]);
        }
        free(imagen_crecimiento);

        // Liberar imagen
        for (int i = 0; i < alto; i++) {
            free(imagen[i]);
        }
        free(imagen);



    }
    delay(10000);
}


void Ecualizar_imagen(uint32_t** imagen_ecualizada, uint32_t**imagen, int alto, int ancho) {

        uint32_t histograma[256] = {0};
        float probabilidad[256] = {0};
        float suma_prob[256] = {0};
        uint8_t transforma[256] = {0};

        // Calcular histograma
        for(int fila=0; fila<alto; fila++){
          for(int col=0; col<ancho; col++){
            uint8_t pixel=imagen[fila][col];
            histograma[pixel]++;

          }

        }

        // Calcular probabilidad
        int total_pixeles=ancho*alto;
        for(int i=0; i<256; i++){
          probabilidad[i]=(float)histograma[i]/total_pixeles;
        }

        // Calcular suma acumulada
        float acumulado=0;
        for(int i=0; i<256; i++){
          acumulado += probabilidad[i];
          suma_prob[i]=acumulado;
        }

        // Crear tabla de transformación
        for(int i=0; i<256; i++){
          transforma[i]=(uint8_t)(256.0*suma_prob[i]);
        }

        // Aplicar transformación
        for(int fila=0; fila<alto; fila++){
          for(int col=0; col<ancho; col++){
            uint8_t pixel=imagen[fila][col];
            imagen_ecualizada[fila][col]=transforma[pixel];

          }

        }
  
}




void algoritmoPuga(uint32_t** bordes, uint32_t**imagen, int alto, int ancho) {
  // Calcular el valor máximo del píxel
  float PXG = 0;
  for (int i = 0; i < alto; i++) {
    for (int j = 0; j < ancho; j++) {
      if (imagen[i][j] > PXG) {
        PXG = imagen[i][j];
      }
    }
  }

  float U1 = (PXG / 4.0) * 1.8;
  float U2 = (PXG / 4.0) * 1.1;

  // Inicializar bordes
  for (int i = 0; i < alto; i++) {
    for (int j = 0; j < ancho; j++) {
      bordes[i][j] = 0;
    }
  }

  // Aplicar algoritmo
  for (int i = 1; i < alto - 1; i++) {
    for (int j = 1; j < ancho - 1; j++) {
      uint8_t NM[9];
      NM[0] = imagen[i-1][j-1];
      NM[1] = imagen[i-1][j];
      NM[2] = imagen[i-1][j+1];
      NM[3] = imagen[i][j-1];
      NM[4] = imagen[i][j];
      NM[5] = imagen[i][j+1];
      NM[6] = imagen[i+1][j-1];
      NM[7] = imagen[i+1][j];
      NM[8] = imagen[i+1][j+1];

      // Primer umbral
      if (NM[4] < U1) {
        if (NM[3] > U1 && NM[5] < U1) {
          bordes[i][j] = 0;
          bordes[i-1][j] = 1;
        } else if (NM[3] < U1 && NM[5] > U1) {
          bordes[i-1][j] = 1;
        } else if (NM[1] > U1 && NM[7] < U1) {
          bordes[i][j] = 0;
          bordes[i][j-1] = 1;
        } else if (NM[1] < U1 && NM[7] > U1) {
          bordes[i-1][j] = 1;
        } else {
          bordes[i][j] = 0;
        }
      } else {
        bordes[i][j] = 0;
      }

      // Segundo umbral
      if (NM[4] < U2) {
        if (NM[3] > U2 && NM[5] < U2) {
          if (bordes[i-1][j] == 0) {
            bordes[i-1][j] = 0;
            bordes[i-1][j-1] = 1;
          } else {
            bordes[i-1][j-1] = 1;
          }
        } else if (NM[3] < U2 && NM[5] > U2) {
          if (bordes[i][j-1] == 0) {
            bordes[i][j] = 0;
            bordes[i][j-1] = 1;
          } else {
            bordes[i][j] = 1;
          }
        } else if (NM[1] > U2 && NM[7] < U2) {
          if (bordes[i][j-1] == 0) {
            bordes[i][j] = 0;
          } else {
            bordes[i][j] = 1;
          }
        } else if (NM[1] < U2 && NM[7] > U2) {
          if (bordes[i][j-1] == 0) {
            bordes[i-1][j] = 0;
          } else {
            bordes[i-1][j] = 1;
          }
        }
      }
    }
  }
}




void crecimiento_imagen(uint32_t** imagen_crecimiento, uint32_t** imagen, int alto, int ancho, int semilla_x, int semilla_y) {
    int tolerancia = 15;
    int max_iter = 1000;

    
    bool** region = (bool**)malloc(alto * sizeof(bool*));
    for (int i = 0; i < alto; i++) {
        region[i] = (bool*)calloc(ancho, sizeof(bool));
    }

    
    region[semilla_y][semilla_x] = true;
    imagen_crecimiento[semilla_y][semilla_x] = 255;
    float suma_intensidades = imagen[semilla_y][semilla_x];
    int num_pixeles = 1;

    for (int iter = 0; iter < max_iter; iter++) {
        bool cambio = false;
        for (int y = 1; y < alto - 1; y++) {
            for (int x = 1; x < ancho - 1; x++) {
                if (region[y][x]) {
                    // Revisar vecinos 4-conectados
                    int dx[4] = {1, -1, 0, 0};
                    int dy[4] = {0, 0, 1, -1};
                    for (int i = 0; i < 4; i++) {
                        int nx = x + dx[i];
                        int ny = y + dy[i];
                        if (!region[ny][nx]) {
                            int intensidad = imagen[ny][nx];
                            float media = suma_intensidades / num_pixeles;
                            if (abs(intensidad - media) <= tolerancia) {
                                region[ny][nx] = true;
                                imagen_crecimiento[ny][nx] = 255;
                                suma_intensidades += intensidad;
                                num_pixeles++;
                                cambio = true;
                            }
                        }
                    }
                }
            }
        }
        if (!cambio) break;
    }

    // Liberar memoria
    for (int i = 0; i < alto; i++) {
        free(region[i]);
    }
    free(region);
}
