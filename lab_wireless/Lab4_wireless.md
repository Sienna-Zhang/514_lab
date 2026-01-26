## Lab 4: Wireless (BLE)

## Lab Prerequisites

Before this lab, make sure you have the following ready:

* Have this repository cloned on your laptop locally. Pull the latest updates of the main branch. EVERYONE must know how to do!
* **Seeeduino XIAO ESP32S3**. If would be better if you've got an Bluetooth antenna.
* **USB-C cable**: Make sure you can connect ESP32 to your laptop
* **HC-SR04 Ultrasonic Distance Sensor**

Students will be grouped into twos to accomplish this lab.

## Section 1: MCU as BLEScanner

Refer to the documentation about [Bluetooth for Seeeduino XIAO](https://wiki.seeedstudio.com/xiao_esp32s3_bluetooth/#ble-scanner). Use the code on this documentation and have it run on your ESP32S3. It will print out all the detected Bluetooth devices in your physical environment on your serial monitor.

Take screenshot of your serial monitor displaying the number of Bluetooth devices detected during the scan.

## Section 2: Establishing Bluetooth Connection

Use the code on this GitHub repository for your [server](https://github.com/GIXLabs/TECHIN514_W24/tree/main/Lab4_wireless/xiao-bluetooth-server) and [client](https://github.com/GIXLabs/TECHIN514_W24/tree/main/Lab4_wireless/xiao-bluetooth-client) device.

Use PlatformIO to pick the folder as project and open it. One student will upload the server code to their ESP32S3, and the other uploads the client's code.

Change the UUID of both codes. You can generate one on [this website](https://www.uuidgenerator.net/). As long as your server and client codes' UUIDs match, it doesn't matter what specific values you are using. But please make sure that you are using a different one from all other groups' devices.

Please also change your BLEDevice's name to avoid any confusions (this is super helpful if you're connecting your MCU with a mobile phone or laptop).

Upload your codes on your ESP32s, and take a screenshot of the client’s serial monitor showing the successful connection. Ensure the server device’s name is included in the screenshot.

### 4.3 Sensor Device Signal Processing and Data Transmission (Server Device, 30 Points)

Use a breadboard (or any other ways you like) to connect the HC-SR04 ultrasonic distance sensor to your MCU. The sensor's datasheet is [here](https://www.handsontec.com/dataspecs/HC-SR04-Ultrasonic.pdf).

See the TODOs in your **SERVER** device's code. Fill in the codes for the following functions properly:

- Using a DSP algorithm (e.g. low-pass filter, moving average, etc.) )you want to use to denoise the raw data collected. Print both the raw data and denoised on the serial monitor.
- Transmit the data over BLE **ONLY IF the denoised distance readout is less than 30cmcertain conditions**.

Take a screenshot of the serial monitor on your server device showing the raw and denoised distance value collected from the HC-SR04 distance sensor.

### 4.4 Display Device Signal Processing and Data Receiving (Client Device, 40 Points)

See the TODOs in your **CLIENT** device's code. Fill in the codes for the following functions properly:

- Calcualte the maximum and minimum distance data received since connection to your server device.
- Print your **currently receiving** data, **maximum**, and **minimum** data on your serial monitor everytime you received new data.

Let the readout of the sensor on your server device change a bit. Make sure you've sent the data at least 10 times. Then take a screenshot of your client device's serial monitor, including all three data mentioned above.
