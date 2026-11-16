# Real-Time-Image-Transmission

This project sends an image to an ESP32-S3 for image processing using three different methods: PUGA, histogram equalization, and region growing. After processing, the ESP32 sends the three resulting images back to the computer.

How to Run
Follow these steps to execute the project:

-Upload the Proyecto_3_metodos code to the ESP32-S3. You can configure your Wi-Fi credentials in the ESP32 code.
-Run Proyect_3_metodos on your computer. Make sure to configure the ESP32's IP address in the code.
-Once the process is complete, you will be able to view the three images received from the ESP32.


To calculate the PSNR (Peak Signal-to-Noise Ratio), run calcular_psnr_3.

The resulting images are saved as:

crecimiento.jpg
Ecualizada.jpg
puga.jpg
You can also find the PSNR results in captura.png.

Required Libraries
Install the following Python libraries: numpy, matplotlib, socket, opencv-python (cv2)
