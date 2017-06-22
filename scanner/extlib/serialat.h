#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2 // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3 // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)

// We'll use a software serial interface to connect to ESP8266
SoftwareSerial ESPserial (D2, D3);

void send_serial_msg(String string_msg) {
	if (string_msg.length() > 0) {
		Serial.print("L" + String(string_msg.length()) + " ");
		Serial.println(string_msg);
		while (ESPserial.available() == 0) ;
		ESPserial.begin(9600);
		if( ESPserial.available()>0 ){
			ESPserial.println(String(string_msg));
		}
	}
}