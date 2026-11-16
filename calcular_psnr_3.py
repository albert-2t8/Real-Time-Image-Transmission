# -*- coding: utf-8 -*-
"""
Created on Mon May 19 00:13:27 2025

@author: alber
"""

import numpy as np
import matplotlib.pyplot as plt
import cv2

plt.close('all')


    
imagen= cv2.imread("Ecualiza_02.jpg")
image= cv2.cvtColor(imagen, cv2.COLOR_BGR2HSV)
imagen_redimensionada = cv2.resize(image, (640, 640)) 
imagen_hsv=imagen_redimensionada[:,:,2]
alto,ancho=imagen_hsv.shape   

#####################################################################################
#### Cargar imagenes de la ESP32

imagen_esp1= cv2.imread("puga.jpg", cv2.COLOR_BGR2GRAY)
imagen_esp2= cv2.imread("Ecualizada.jpg", cv2.COLOR_BGR2GRAY)
imagen_esp4= cv2.imread("crecimiento.jpg", cv2.COLOR_BGR2GRAY)
    
    
    
def calcular_psnr(im_hr, im_lr):
    
    im_hr = im_hr.astype(np.float32)
    im_lr = im_lr.astype(np.float32)
    
    diff=im_hr-im_lr
    mse=np.mean(np.square(diff))
    
    max_pixel=255.0
    
    psnr_1=10*np.log10(max_pixel**2/mse)
    
    return psnr_1




def algoritmo_puga(imagen,imagen_1, ancho, alto):
    
    PXG = np.max(imagen)  
    U1 = (PXG / 4) * 1.8
    U2 = (PXG / 4) * 1.1
    
    
    for i in range(1, alto - 1):
        for j in range(1, ancho - 1):
            ventana = imagen[i-1:i+2, j-1:j+2]
            
            
            NM = ventana.flatten()
            
            if NM[4] < U1:
                if NM[3] > U1 and NM[4] < U1 and NM[5] < U1:
                    bordes[i, j] = 0  
                    bordes[i-1, j] = 1  
                elif NM[3] < U1 and NM[4] < U1 and NM[5] > U1:
                    bordes[i-1, j] = 1  
                elif NM[1] > U1 and NM[4] < U1 and NM[7] < U1:
                    bordes[i, j] = 0  
                    bordes[i, j-1] = 1  
                elif NM[1] < U1 and NM[4] < U1 and NM[7] > U1:
                    bordes[i-1, j] = 1  
                else:
                    bordes[i, j] = 0  
            else:
                bordes[i, j] = 0  
            
            if NM[4] < U2:
                if NM[3] > U2 and NM[4] < U2 and NM[5] < U2:
                    if bordes[i-1, j] == 0:
                        bordes[i-1, j] = 0  
                        bordes[i-1, j-1] = 1
                    else:
                        bordes[i-1, j-1] = 1  
                elif NM[3] < U2 and NM[4] < U2 and NM[5] > U2:
                    if bordes[i, j-1] == 0:
                        bordes[i, j] = 0  
                        bordes[i, j-1] = 1  
                    else:
                        bordes[i, j] = 1  
                elif NM[1] > U2 and NM[4] < U2 and NM[7] < U2:
                    if bordes[i, j-1] == 0:
                        bordes[i, j] = 0  
                    else:
                        bordes[i, j] = 1  
                elif NM[1] < U2 and NM[4] < U2 and NM[7] > U2:
                    if bordes[i, j-1] == 0:
                        bordes[i-1, j] = 0  
                    else:
                        bordes[i-1, j] = 1  
    
    return bordes



def ecualizar_imagen(imagen_hsv,alto, ancho):
    #########################################################################################################
    # Histograma
    histograma = [0] * 256
    for y in range(alto):
        for x in range(ancho):
            pixel = imagen_hsv[y, x]
            histograma[pixel] += 1
    
    #########################################################################################################
    # Probabilidad y suma acumulada
    total = ancho * alto
    probabilidad = [h / total for h in histograma]
    
    suma_prob = []
    acumulado = 0
    for p in probabilidad:
        acumulado += p
        suma_prob.append(acumulado)
    
    #########################################################################################################
    # Transformación
    transforma = [int(255 * s) for s in suma_prob]
    
    # Aplicar la transformación a cada píxel
    ecualizada = np.zeros_like(imagen_hsv, dtype=np.uint8)
    for y in range(alto):
        for x in range(ancho):
            pixel = imagen_hsv[y, x]
            ecualizada[y, x] = transforma[pixel]
    
    #########################################################################################################
    return ecualizada






def crecimiento_imagen(imagen, semilla_x, semilla_y, alto, ancho):
    tolerancia=15
    max_iter=1000
    
    region = np.zeros((alto, ancho), dtype=bool)
    imagen_crecimiento = np.zeros((alto, ancho), dtype=np.uint8)

    region[semilla_y, semilla_x] = True
    imagen_crecimiento[semilla_y, semilla_x] = 255
    suma_intensidades = float(imagen[semilla_y, semilla_x])
    num_pixeles = 1

    for _ in range(max_iter):
        cambio = False
        for y in range(1, alto - 1):
            for x in range(1, ancho - 1):
                if region[y, x]:
                    # Vecinos 4-conectados
                    for dx, dy in [(1,0), (-1,0), (0,1), (0,-1)]:
                        nx, ny = x + dx, y + dy
                        if not region[ny, nx]:
                            intensidad = int(imagen[ny, nx])
                            media = suma_intensidades / num_pixeles
                            if abs(intensidad - media) <= tolerancia:
                                region[ny, nx] = True
                                imagen_crecimiento[ny, nx] = 255
                                suma_intensidades += intensidad
                                num_pixeles += 1
                                cambio = True
        if not cambio:
            break

    return imagen_crecimiento





##################################### PUGA #########################################
bordes = np.zeros_like(imagen_hsv, dtype=int)
edges_puga = algoritmo_puga(imagen_hsv,bordes,alto, ancho)
imagen_1_uint8 = (edges_puga / edges_puga.max() * 255).astype(np.uint8)


psnr_valor=calcular_psnr(imagen_esp1, imagen_1_uint8)
print(f"PSNR de la imagen Puga generada: {psnr_valor:.2f} dB")


##################################### Ecualización de histograma #########################3
ecualizada=ecualizar_imagen(imagen_hsv, alto, ancho)


psnr_valor_1=calcular_psnr(ecualizada, imagen_esp2)
print(f"PSNR de la imagen ecualizada generada: {psnr_valor_1:.2f} dB")



################################ Crecimiento de región #######################################
crecimiento_region=crecimiento_imagen(imagen_hsv, 277, 177, alto, ancho)
psnr_valor_3=calcular_psnr(crecimiento_region, imagen_esp4)
print(f"PSNR de la imagen crecimiento de region generada: {psnr_valor_3:.2f} dB")



plt.figure()
plt.subplot(1,2,1)
plt.imshow(edges_puga, cmap='gray')
plt.title("Puga en python")
plt.axis('off')
plt.subplot(1,2,2)
plt.imshow(imagen_esp1, cmap='gray')
plt.title("Puga en ESP32")
plt.axis('off')


plt.figure()
plt.subplot(1,2,1)
plt.imshow(ecualizada, cmap='gray')
plt.title("Ecualización en python")
plt.axis('off')
plt.subplot(1,2,2)
plt.imshow(imagen_esp2, cmap='gray')
plt.title("Ecualización en ESP32")
plt.axis('off')


plt.figure()
plt.subplot(1,2,1)
plt.imshow(imagen_hsv, cmap='gray')
plt.imshow(crecimiento_region, cmap='gray', alpha=0.4)
plt.title("Crecimiento de región en python")
plt.axis('off')
plt.subplot(1,2,2)
plt.imshow(imagen_hsv, cmap='gray')
plt.imshow(imagen_esp4, cmap='gray', alpha=0.4)
plt.title("Crecimiento de region en ESP32")
plt.axis('off')
