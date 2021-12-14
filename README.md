# Alertino

Code for arduino pump monitors

## Information to gather before starting:
- A name that will identify this monitor (I usually use the name of the fridge it is connected to)
- A wall socket where the arduino will be connected (ethernet socket ... the one for internet)
- What MAC address to assing to the device ([What is a MAC address ?](https://en.wikipedia.org/wiki/MAC_address#:~:text=A%20media%20access%20control%20address,Wi%2DFi%2C%20and%20Bluetooth.))

Here is a table containing that data for the arduinos connected so far. Try to use the next available MAC address if possible.
| Name | Socket | MAC | IP |
| ------ | ------ | ------ | ------ |


## Make a request to IST IT to reserve an IP address for the MAC address you assigned to your device
- The IP address should be like this: 10.21.64.### where last three digits are any available address. The reason for this is because all devices that are located in the lab, and are connected to the network, will have such address. If address of a different type is reserved for the device, the device will be assigned a random address and you will not know which one is it, therefor you will not be able to connect to the device.
- Once you get the response from IT about which address was assigned to your device, update the table above with the new data (edit the file directly in GitLab)

## How to prepare and install a new arduino monitor

- Get your hands on a new arduino device from machine shop
    - Arduino Uno
    - MKR GSM 14000 shield
    - Ethernet Shield
    - A battery
    - GSM antenna

- Install Arduino IDE on a PC (It is important to get your hands on a laptop for this (i will explain later))
    - Find it on [THIS](https://www.arduino.cc/en/software) link (Arduino IDE, not the laptop ...)

- Install necessery libraries in Arduino IDE: Tools -> Manage Libraries
    - MKR GSM 14000
    
        ![MKRGSM](https://user-images.githubusercontent.com/19326347/136975294-8ce5b3b5-1849-4ea4-8def-667501c34947.png)

    - Ethernet
    
        ![Ethernet](https://user-images.githubusercontent.com/19326347/136975313-720ee6dc-ae5c-4a6f-9f57-0a97671b56ef.png)
        
- Install necessery board manager in Arduino IDE: Tools -> Board -> Boards Manager
    - MKR GSM 1400
        
        ![boards](https://user-images.githubusercontent.com/13066652/137130728-25005c73-fc3b-4e63-8e0e-374e502e77fc.png)



- Clone this repository on the machine you will be using to upload code to arduino (once again, its important that this is a laptop)
    - install Git BASH if you dont have it, find it on [THIS](https://gitforwindows.org/) link
    - open Git BASH
    - clone the repository by using the following commands (make sure to change directory to where u want to clone the repo to)
    ``` 
    cd C:
    git clone https://git.ist.ac.at/finkgroup/alertino.git
    ```
    - this will create a new folder called Alertino. The path to the directory will be "C:/alertino"
    - you can move this directory if you wish to have it somewhere else

- Open downloaded code in Arduino IDE and edit the following lines:
    - alertino.ino/line 10: update to reflect the name of the arduino
    - alertino.ino/line 13: update to reflect the MAC address of the arduino

- Connect the arduino device to the ethernet wall socket, and the laptops USB port, at the same time
    - This is why you needed the latop
    - Laptop does not need to be connected to the socket in the lab

- Upload the code to the device
    - Hey, did you connect the device to the wall socket first ? This is important because the device needs to be connected to a socket in the lab so it will be assigned with the IP address that was reserved for it. If the device is not connected to a wall socket in the lab, a random IP address might be assigned to it. 
    - No, really, connect the device to wall socket before uploading code
    - Configure the type of board you are using in Arduino IDE: Tools -> Board -> Arduino SAMD -> Arduino MKR GSM 1400
    - Find out which port is arduino connected to: Press Windows key -> Search "Device Manager" -> Expand "Ports (COM and LPT)"
    - Configure the COM port you are using in Arduino IDE: Tools -> Port
    - Open serial monitor in Arduino IDE: Tools -> Serial Monitor
    - Build and upload the code

After succesfull upload, in serial monitor you should see a message saying: "server is at 10.21.64.###". This address should match the address reserved for the device in one of the earlier steps.

To check that it is working, go to any PC inside the LAB (PC connected to a wall socket in the lab, not the laptop you are holding in your hands) and open internet browser. In the address field input the IP address. You should be able to see the arduinos web server.
