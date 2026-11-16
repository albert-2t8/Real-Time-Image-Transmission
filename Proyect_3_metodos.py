# -*- coding: utf-8 -*-
"""
Created on Sun May 18 21:31:25 2025

@author: alber
"""

import cv2
import socket
import struct
import numpy as np
import matplotlib.pyplot as plt



#################### PUGA, ECUALIZACION Y CRECIMIENTO DE REGION ##########################################


def punto(image):

    plt.style.use('dark_background')
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.imshow(image, cmap='gray' if len(image.shape)==2 else None) #
    ax.set_title('\n Selección de semillas: \n Click izquierdo: Añadir semilla\nEnter: Finalizar | Esc: Cancelar'
                 , fontsize=12, pad=20)
    plt.axis('off')
    
    try:
        seeds = plt.ginput(n=-1, timeout=0, show_clicks=True)
        
        
    except Exception:
        seeds = [] 
        
        
    semillas = np.round(seeds).astype(int) if len(seeds) > 0 else np.array([])  
    intensidad = [image[y, x] for x, y in semillas] if semillas.size > 0 else []
    
    plt.close()
    print(semillas)
    return semillas, intensidad




# Preparar imagen
imagen_color = cv2.imread("Ecualiza_02.jpg")
imagen_color = cv2.resize(imagen_color, (640, 640)) 
image= cv2.cvtColor(imagen_color, cv2.COLOR_BGR2HSV)
image_1= cv2.cvtColor(imagen_color, cv2.COLOR_BGR2RGB)
imagen_hsv=image[:,:,2]




seeds, intensidad = punto(imagen_hsv)

semilla_x=int(seeds[0][0])
semilla_y=int(seeds[0][1])

alto,ancho=imagen_hsv.shape   

imagen_vector=imagen_hsv.flatten()



esp32_Host = "000.000.00.000"  
esp32_puerto = 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((esp32_Host, esp32_puerto))

# # Enviar dimensiones
sock.send(struct.pack("II", alto, ancho))
## Enviar semilla
sock.send(struct.pack("II", semilla_x, semilla_y))



for pixel in imagen_vector:  
    sock.send(struct.pack("I", pixel))  



#####################################################################################################################
################################### RECIBIR IMAGEN ECUALIZADA ##################################################################

num_pixels = alto * ancho


datos = b''
esperados = num_pixels * 4  

while len(datos) < esperados:
    
    chunk = sock.recv(4096)
    if not chunk:
        break
    datos += chunk



vector = np.frombuffer(datos, dtype=np.uint32)
imagen = vector.reshape((alto, ancho))

# image[:,:,2]=imagen
# imagen_mejora_0= cv2.cvtColor(image, cv2.COLOR_HSV2RGB)

# plt.figure()
# plt.imshow(imagen_mejora_0, cmap='gray')
# plt.title("Imagen recibida")
# plt.axis('off')
# plt.show()


imagen_uint8 = (imagen / imagen.max() * 255).astype(np.uint8)
cv2.imwrite('Ecualizada.jpg', imagen_uint8)

plt.figure()
plt.imshow(imagen_uint8, cmap='gray')
plt.title("Ecualización de histograma")
plt.axis('off')
plt.show()

#####################################################################################################################
################################### RECIBIR IMAGEN PUGA ##################################################################



datos_1 = b''
esperados_1 = num_pixels * 4  

while len(datos_1) < esperados_1:

        chunk_1 = sock.recv(4096)
        if not chunk_1:
            break
        datos_1 += chunk_1
        
        
vector_1 = np.frombuffer(datos_1, dtype=np.uint32)
imagen_1 = vector_1.reshape((alto, ancho))


imagen_1_uint8 = (imagen_1 / imagen_1.max() * 255).astype(np.uint8)
cv2.imwrite('puga.jpg', imagen_1_uint8)

plt.figure()
plt.imshow(imagen_1_uint8, cmap='gray')
plt.title("Puga")
plt.axis('off')
plt.show()


######################################################################################################
############################# RECIBIR IMAGEN CRECIMIENTO DE REGION ####################################################

datos_3= b''

esperados_3 = num_pixels * 4  

while len(datos_3) < esperados_3:
    chunk_3 = sock.recv(4096)
    if not chunk_3:
        break
    datos_3 += chunk_3


vector_3 = np.frombuffer(datos_3, dtype=np.uint32)
imagen_ob = vector_3.reshape((alto, ancho))



imagen_ob_uint8 = (imagen_ob == 255).astype(np.uint8) * 255
cv2.imwrite('crecimiento.jpg', imagen_ob_uint8)

plt.figure()
plt.imshow(image_1, cmap='gray')
plt.imshow(imagen_ob_uint8, cmap='gray', alpha=0.4)
plt.title("Crecimiento de región")
plt.axis('off')
plt.show()



sock.close()