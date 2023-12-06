# Gas Sensor
Sensor de nivel de gas para tanques estacionarios. Se coloca sobre 
el indicador de nivel y transmite las lectura de manera inalámbrica. 
Hay 2 versiones principales: uno que usa LoRa y otro que usa Sigfox.  

La versión LoRa no es viable porque necesita muchas estaciones disponibles.  
La versión Sigfox funciona.  

Debe funcionar durante 3 años continuos usando 2 baterías AA. 
Se necesita un consumo promedio de 4 mA o menos.

El diseño "sensor_sigfox_1.1" es una actualización de la versión 1 del sensor Sigfox. 
Se han realizado cmabios en hardware y software . Los cambios principales son:  
- los módulos están configurados en modo de bajo consumo
- la lectura del voltaje de alimentación se realiza con el hardware interno del microcontrolador (de esta forma se elimina un divisor de voltaje externo)
- cambio de sensor magnético (se descarta el AS5600 y se agrega el MLX90393)
- usar frecuencia de reloj de ultra-bajo consumo (32.768 kHz)
- se puede usar el generador de frecuencia interno, pero también se agrega un cristal y capacitores de carga para commparar el consumo
- eliminar booster
- agregar conector U.FL para antena y pad para soldar antena de alambre opcional (eliminar conector SMA)

El conector U.FL se usa en muchos dispositivos  
https://en.wikipedia.org/wiki/Hirose_U.FL  
https://learn.sparkfun.com/tutorials/three-quick-tips-about-using-ufl/all  
https://www.data-alliance.net/ufl-cables/  

Los cristales más baratos tienen encapsulado cilindrico  
https://www.lcsc.com/product-detail/Crystals_CREC-01-X-TB-11CHJRI032768000_C7462599.html  
Para facilitar el montaje se usarán cristales SMD aunque son un poco más grandes  
https://www.lcsc.com/product-detail/Crystals_huaxindianzi-DP2032K76812001_C17702403.html  

El microcontrolador es ATtiny826 (8kB, family 2, 20 pins). Se usan los siguiente pines:  
- alimentación VCC pin 1, GND pin 20
- I2C (TWI) SDA pin 10 PB1, SCL pin 11 PB0
- cristal TOSC1 pin 8 PB3, TOSC2 pin 9 PB2
- UART hay 2 puertos con pines configurables (Serial y Serial1) con TX, RX, XCK, XDIR en PC0, PC1, PC2, PC3 y PA1, PA2, PA3, PA4
https://datasheet.lcsc.com/lcsc/2309071123_Microchip-Tech-ATTINY826-XU_C3235639.pdf
https://github.com/SpenceKonde/megaTinyCore

Los componentes más baratos están en LCSC, luego DigiKey y en tercer lugar Mouser  

El sensor Hall se puede usar en modo I2C o SPI. Se usará el modo I2C
