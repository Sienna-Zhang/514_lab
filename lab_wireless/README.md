# Lab 4 - ESP32 Wireless Lab

## Lab Instructions

In this lab, you'll learn the basics about how to use BLE (Bluetooth Low-Energy) to connect two Seeeduino XIAO ESP32S3 microcontrollers. You can also use these techniques to connector your MCU to your mobile phone, latptops, etc.

Follow the instruction written in[ Lab4: Wireless.md](https://github.com/GIXLabs/TECHIN514_W24/blob/main/Lab4_wireless/Lab4_wireless.md) to:

* Use BLE libraries to scan the bluetooth devices with ESP32S3
* Setting two MCUs as a BLE server and a BLE client, and make them connected
* Transmitting streaming data from the server device to the client with BLE
* Develop your own DSP techniques on your MCU to handle the streaming data

Some useful external resources:

* [Bluetooth on ESP32S3](https://wiki.seeedstudio.com/xiao_esp32s3_bluetooth)
* [HC-SR04 Datasheet](https://www.handsontec.com/dataspecs/HC-SR04-Ultrasonic.pdf)
* [Moving average (Wikipedia)](https://en.wikipedia.org/wiki/Moving_average)

## Lab Requirements

### 4.1 BLE Scan (10 Points)

Identify Bluetooth devices in the vicinity.

* Take screenshot of your serial monitor displaying the number of Bluetooth devices detected during the scan.

### 4.2 Establishing Bluetooth Connection (20 Points)

Establish a Bluetooth connection with your partner's ESP32.

- Take a screenshot of the client’s serial monitor showing the successful connection. Ensure the server device’s name is included in the screenshot.

### 4.3 Sensor Device Signal Processing and Data Transmission (Server Device, 30 Points)

Collect and process distance data using the HC-SR04 sensor

- Using a DSP algorithm (e.g. low-pass filter, moving average, etc.) )you want to use to denoise the raw data collected. Print both the raw data and denoised on the serial monitor.
- Transmit the data over BLE **ONLY IF the denoised distance readout is less than 30cmcertain conditions**.
- Take a screenshot of the serial monitor on your server device showing the raw and denoised distance value collected from the HC-SR04 distance sensor.

### 4.4 Display Device Signal Processing and Data Receiving (Client Device, 40 Points)

Receive, process, and display distance data on the client device.

- Calcualte the maximum and minimum distance data received since connection to your server device. Print your **currently receiving** data, **maximum**, and **minimum** data on your serial monitor everytime you received new data.
- Let the readout of the sensor on your server device change a bit. Make sure you've sent the data at least 10 times.
- Take a screenshot of your client device's serial monitor, including all three data mentioned above.

**Note**: Ensure that all screenshots are clear, and the required data or indicators are appropriately highlighted or circled. Follow the specific instructions for each section closely to meet the criteria for full points.

## What to be submitted on Canvas

Submit a single PDF containing your Team member names nad all screenshots as specified below.

1. The link to your edited code is on GitHub. You can either create a new repo or push the updates to your existing repo for all labs (recommended).
2. Screenshot of your serial monitor displaying the **number of Bluetooth devices detected** using your MCU as BLEScanner
3. Screenshot of the serial monitor of your clicent device to show successfull connection with your server device (make sure the server device's name is included).
4. Screenshot of the serial monitor of your server device to show the raw and denoised sensor data.
5. Screenshot of the serial monitor of your client device to show the currently-reading, maximum, and minimum data transmitted from your server device.

Ensure the screenshots are clear and properly labeled as per the requirements.
