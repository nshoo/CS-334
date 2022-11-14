## Wireless Communication

For this lab we had to learn how to send a message from the ESP32 over wifi. The information we had to send was some continuously poled data from a pair of sensors: a photoresistor, and a piezo. The *UDP_Snsor.ino* file is an arduino project which, when loaded onto the ESP32, will send this information as a space-seperated line of text to a hard-coded ip address. The sensor readings it takes are spaced out by about 50ms, and the packets are sent through UDP, so there shouldn't be too much latency on the recieving end.
