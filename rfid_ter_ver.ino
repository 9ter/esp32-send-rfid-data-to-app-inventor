#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"
SoftwareSerial RFID(5, 4);  // RX and TX
LiquidCrystal_I2C lcd(0x27, 20, 4);
BluetoothSerial SerialBT;

String rfidData = "";
const int buzzerPin = 25;  // Define the pin connected to the buzzer
const char *pin = "1234";  // Change this to more secure PIN.

String device_name = "ESP32-BT-Slave";
bool isRFIDActive = false;
String mac_string;

String command[2];


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif



void setup() {
  Serial.begin(9600);
  RFID.begin(9600);

  lcd.begin();
  lcd.backlight();
  pinMode(buzzerPin, OUTPUT);

  SerialBT.begin(device_name);  //Bluetooth device name
  uint8_t mac_arr[6];           // Byte array to hold the MAC address from getBtAddress()
  BTAddress mac_obj;            // Object holding instance of BTAddress with the MAC (for more details see libraries/BluetoothSerial/src/BTAddress.h)
  String mac_str;               // String holding the text version of MAC in format AA:BB:CC:DD:EE:FF

  SerialBT.getBtAddress(mac_arr);           // Fill in the array
  mac_obj = SerialBT.getBtAddressObject();  // Instantiate the object
  mac_str = SerialBT.getBtAddressString();  // Copy the string
  Serial.print("This device is instantiated with name ");
  Serial.println(device_name);

  Serial.print("The mac address using byte array: ");
  for (int i = 0; i < ESP_BD_ADDR_LEN - 1; i++) {
    Serial.print(mac_arr[i], HEX);
    Serial.print(":");
  }
  Serial.println(mac_arr[ESP_BD_ADDR_LEN - 1], HEX);

  Serial.print("The mac address using BTAddress object using default method `toString()`: ");
  Serial.println(mac_obj.toString().c_str());
  Serial.print("The mac address using BTAddress object using method `toString(true)`\n\twhich prints the MAC with capital letters: ");
  Serial.println(mac_obj.toString(true).c_str());  // This actually what is used inside the getBtAddressString()

  Serial.print("The mac address using string: ");
  Serial.println(mac_str.c_str());
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());

#ifdef USE_PIN
  SerialBT.setPin(pin);
  Serial.println("Using PIN");
#endif

  pinMode(buzzerPin, OUTPUT);

  mac_string = String(mac_str.c_str());



}

void loop() {
  //Serial.println("RFID Data: " + rfidData);

  if (!SerialBT.connected()) {
    //Serial.println("BT not connected");
    /*lcd.setCursor(0, 0);
      lcd.print("BT not connected");*/
    lcd.setCursor(0, 0);
    lcd.print("Device Name -----");
    lcd.setCursor(0, 1);
    lcd.print(mac_string);
    lcd.setCursor(0, 2);
    lcd.print(device_name);
    delay(50);

  }

  if (RFID.available()) {
    char raw_data = RFID.read();

    // ตรวจสอบว่าตัวอักษรไม่ใช่ ASCII control characters
    if (raw_data != 0x02 && raw_data != 0x03) {
      rfidData += raw_data;
    }

    // ถ้าเจอ ASCII control character ETX (0x03) แสดงว่าข้อมูลเสร็จสิ้น
    if (raw_data == 0x03) {
      Serial.println("RFID Data: " + rfidData);

      if (isRFIDActive) {
        beep(2);
        SerialBT.print(rfidData); // ส่ง
        Serial.println("SendSuccessful");
        rfidData = "";
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Please wait....");
        delay(1000);
        isRFIDActive = false; //ออก
      }
      rfidData = "";
      isRFIDActive = false; //ออก
    }
  }

  if (SerialBT.available()) {
    String datacommand = SerialBT.readString();
    Serial.print("datacommand ----> ");
    Serial.println(datacommand);
    rfidData = "";
    BT_command(datacommand);
  }



}


void BT_command(String datacommand) {

  // หาตำแหน่งของเครื่องหมายจุลภาค
  int found = datacommand.indexOf(':');

  // ถ้าพบเครื่องหมายจุลภาค
  if (found != -1) {
    // แยกสตริงเป็น 2 ชุด
    command[0] = datacommand.substring(0, found);          // active
    command[1] = datacommand.substring(found + 1);         // 70

    // แสดงผลลัพธ์ผ่าน Serial Monitor
    Serial.println("ชุดที่ 1: " + command[0]);
    Serial.println("ชุดที่ 2: " + command[1]);
  } else {
    //Serial.println("ไม่พบเครื่องหมายจุลภาคในสตริง");
    command[0] = datacommand;
    Serial.println("command ----> " + command[0]);

  }

  if (command[0] == "connect") {
    //Serial.println(command[0]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
    isRFIDActive = false;
    Serial.println("NO RFID Active");

  } else if (command[0] == "active") {
    Serial.println("RFID Active");
    isRFIDActive = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PRICE :  ");
    lcd.setCursor(9, 0);
    lcd.print(command[1]);
    lcd.setCursor(15, 0);
    lcd.print("BAHT");
    lcd.setCursor(0, 1);
    lcd.print("Insert your card");

  } else if (command[0] == "norfid") {
    isRFIDActive = false;
    beep(5);
    Serial.println("NO RFID Active");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("NoRfid In The System");
  } else if  (command[0] == "cancel") {
    isRFIDActive = false;
    Serial.println("NO RFID Active");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Cancel");
    lcd.setCursor(0, 2);
    lcd.print("Please wait....");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
  } else if (command[0] == "balance") {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Remaining Balance");
    lcd.setCursor(0, 2);
    lcd.print(command[1]);
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
  } else if (command[0] == "nomoney") {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Not enough money");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
  }
  else if (command[0] == "posterror") {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR !!!");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
  }
  else if (command[0] == "databaseerror") {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("ERROR !!!");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
  }
  else if (command[0] == "reconnect") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PLEASE ORDER");
    lcd.setCursor(0, 1);
    lcd.print("YOUR FOOD ! !");
  }

}


void beep(int num) {
  // Play two short beeps in succession

  for (int i = 0 ; i < num ; i++) {
    tone(buzzerPin, 2500, 200);  // Play the first beep at 1000 Hz for 200 milliseconds
    delay(300);
  }

}
