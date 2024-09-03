| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

#  ECG data processing on ESP32-S3  

## Authors: Juraj Marinčić, Valentina Jelavić
## Mentor: Flip Kulić

This project was developed as part of an internship at Spyrosoft. It demonstrates how to process ECG signals using the ESP32-S3 and send the processed data over Bluetooth. The ESP target will read ECG data from the M034 module, process the data, and then output the processed data via Bluetooth. The pins for reading the ECG data and outputting the processed data can be configured via `menuconfig`. 

### Data Processing

After configuring the I/Os, the project reads raw ECG data from the M034 module, processes it and then sends the processed data via Bluetooth. The ECG sensor uses leads to detect electrical activity from the heart, which are then processed to extract meaningful ECG signals.

## How to use the project

### Hardware Required

* ESP32-S3 development board 
* M034 ECG sensor module 
* USB cable for power supply and programming
* Jumper wires to connect the ECG sensor to the ESP32-S3
* ECG leads and electrodes to attach to the subject's body

### Setting up the Hardware

* Use jumper wires to connect the M034 module to the ESP32-S3 development board. Refer to the sensor's documentation for the correct pin connections.
* Attach the ECG electrodes to the subject's body at the designated points (e.g., left arm, right arm, and left leg) to measure the heart's electrical activity.
* Connect the ECG leads from the electrodes to the M034 module.

### Configure the project

### Build and flash the project

* Set the target of the project to ESP32-S3:
```
idf.py set-target esp32s3
```
* Ensure Bluetooth is enabled in the `menuconfig` under `Component config` -> `Bluetooth`. By default, TX and RX are assigned to GPIOs 0, 2 respectively.
```
idf.py menuconfig
```
* Compile the  project:
```
idf.py build
```
* Flash the compiled project:
```
idf.py flash
```
* Power on the ESP32-S3 board and check the output:
```
idf.py monitor
```
(To exit the serial monitor, type ``Ctrl-]``.)

### Connecting to Mobile Application

* Install the mobile application on your Android device.
* Connect to ESP32-S3 via Bluetooth.
* Once connected, the data will be displayed in real-time on the application interface.

## Example Output
```
ESP_GATTS_CREATE_EVT triggered
status: `status`, service_handle `handle`, uuid: `uuid`
ESP_GATTS_ADD_CHAR_EVT
ESP_GATTS_CONNECT_EVT, conn_id `conn_id`, remote `remote_bda`
ESP_GATTS_DISCONNECT_EVT triggered
```

## Troubleshooting
For any technical queries, please open an issue on GitHub. We will get back to you soon.
