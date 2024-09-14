// compileren voor LillyGo T-Display

/***************************************************
DFPlayer - A Mini MP3 Player For Arduino
 <https://www.dfrobot.com/product-1121.html>
 
 ***************************************************
 This example shows the basic function of library for DFPlayer.
 
 Created 2016-12-07
 By [Angelo qiao](Angelo.qiao@dfrobot.com)
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
 1.Connection and Diagram can be found here
 <https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299#Connection_Diagram>
 2.This code is tested on Arduino Uno, Leonardo, Mega boards.
 ****************************************************/

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <BLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ctype.h>

const char* ssid = "<<SSID>>";
const char* password = "<<SSID PASSWD>>";

const char* mqtt_server = "<<IP ADDRESS>>";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
boolean verbonden = false;
boolean motorspeed = false;
boolean speedmode = false;
boolean geluid = false;
boolean led = false;
int outputwaarde=0;
String outputwaardetekst;

int ch[4];

static BLEUUID serviceUUID("4e050000-74fb-4481-88b3-9919b1676e93"); //Service UUID of BuWizz 
static BLEUUID charUUID(BLEUUID((uint16_t)0x92d1)); //Characteristic  UUID of BuWizz control
String My_BLE_Address = "XX:XX:XX:XX:XX:XX"; //MAC of the BuWizz
static BLERemoteCharacteristic* pRemoteCharacteristic;

BLEScan* pBLEScan ; //Name the scanning device as pBLEScan
// BLEScanResults foundDevices;

static BLEAddress *Server_BLE_Address;
String Scaned_BLE_Address;

boolean paired = false; //boolean variable to toggel pairing
boolean notfound = true;

int8_t ch1; //Temp variable for CH1
int8_t ch2; //Temp variable for CH2

uint8_t valueselect[] = {0x10,0x00,0x00,0x00,0x00,0x00}; //BuWizz "payload" to control the outputs.
uint8_t modeselect[] = {0x11,0x04}; //BuWizz mode select. 0x01: slow, 0x02: normal, 0x03: fast, 0x04: ludicous

boolean mode_fast = false;

boolean isValidNumber(String str){
boolean isNum=false;
for(byte i=0;i<str.length();i++)
{
isNum = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
if(!isNum) return false;
}
return isNum;
}


bool connectToserver (BLEAddress pAddress){
    BLEClient*  pClient  = BLEDevice::createClient();
    boolean connection_ok = false;
    while (!connection_ok){
      Serial.println("Connecting to BuWizz...");
      pClient->connect(BLEAddress(pAddress));
      if (pClient->isConnected()){
        connection_ok = true;
        paired = true;
        Serial.println("Connected to BuWizz");
      }
    }
    
    
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);     // Obtain a reference to the service we are after in the remote BLE server.
    Serial.println("debug A");
    if (pRemoteService != nullptr)
    {
      Serial.println(" - Found the service");
    }
    else
    return false;

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);     // Obtain a reference to the characteristic in the service of the remote BLE server.
    Serial.println("debug B");
    if (pRemoteCharacteristic != nullptr)
    {
      Serial.println(" - Found the characteristic");
      return true;
    }
    else
    return false;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
//      Serial.printf("Scan Result: %s \n", advertisedDevice.toString().c_str());
        if (advertisedDevice.getName() == "BuWizz"){
          Serial.println("BuWizz found! MAC address:");
          Serial.println("debug H");
          advertisedDevice.getScan()->stop();
          Serial.println("debug I");
          notfound=false;
          Serial.println("debug J");
          Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
          Serial.println("debug K");
          Scaned_BLE_Address = Server_BLE_Address->toString().c_str();
          Serial.println("debug L");
          Serial.println(Scaned_BLE_Address);
          Serial.println("debug M");
          }
    }
};



#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      // digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      // digitalWrite(ledPin, LOW);
    }
    Serial.println(isValidNumber(messageTemp))  ;
    if (isValidNumber(messageTemp)==1) {outputwaarde = messageTemp.toInt();motorspeed=true; geluid=false; speedmode=false;led=false;}
    if (isValidNumber(messageTemp)==0) {outputwaardetekst = messageTemp;
                                        if (outputwaardetekst=="Stoomfluit") {motorspeed=false;geluid=true; speedmode=false;led=false;}
                                        if (outputwaardetekst=="Closedoors") {motorspeed=false; geluid=true;  speedmode=false;led=false;}
                                        if (outputwaardetekst=="NSfluit") {motorspeed=false;  speedmode=false;led=false; geluid=true;}
                                        if (outputwaardetekst=="Trainstop") {motorspeed=false;  speedmode=false;led=false; geluid=true;}
                                        if (outputwaardetekst=="led1on") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led2on") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led3on") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led4on") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led1off") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led2off") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led3off") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="led4off") {motorspeed=false; geluid=false; speedmode=false;led=true;}
                                        if (outputwaardetekst=="slow") {motorspeed=false; geluid=false; speedmode=true;led=false;}
                                        if (outputwaardetekst=="normal") {motorspeed=false; geluid=false; speedmode=true;led=false;}
                                        if (outputwaardetekst=="fast") {motorspeed=false; geluid=false; speedmode=true;led=false;}
                                        if (outputwaardetekst=="ludicous") {motorspeed=false; geluid=false; speedmode=true;led=false;}
                                       }
    Serial.println(outputwaarde);
    Serial.println(outputwaardetekst);
  }
}

void reconnect() {
  // Loop until we're reconnected
  Serial.println("reconnect ");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client","<<MQTT USER>>","<<MQTT PASS>>")) {
      Serial.println("connected");
      verbonden = true;
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network


  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // Serial.println("");
   Serial.println(WiFi.localIP());
}

void setup()
{
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(32, OUTPUT);
#if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/27, /*tx =*/26);
#else
  FPSerial.begin(9600);
#endif

  Serial.begin(115200);
  setup_wifi();
    client.setServer(mqtt_server, <<MQTT BROKER PORT>>);
    client.setCallback(callback);
    Serial.println("ESP32 BLE BuWizz Interface on PPM"); //Intro message
Serial.println("debug C");
    BLEDevice::init("");
    Serial.println("debug D");
    pBLEScan = BLEDevice::getScan(); //create new scan
    Serial.println("debug E");
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above 
    Serial.println("debug F");
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    Serial.println("debug G");

    while (notfound == true){ //Searching for BLE device names "BuWizz"
      pBLEScan->start(3); 
      }
    while (connectToserver(*Server_BLE_Address) != true){  
      Serial.println("Tikk-Takk.. waiting..");
    }
    
    pRemoteCharacteristic->writeValue((uint8_t*)modeselect, 2, true);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
 // myDFPlayer.play(1);  //Play the first mp3
}

void loop()
{
  static unsigned long timer = millis();
  
  if (millis() - timer > 3000) {
    timer = millis();
//    myDFPlayer.next();  //Play next mp3 every 3 second.
  }
  
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
  if (paired == true) {
      if (!client.connected()) {
        reconnect();
      }
  client.loop();
  
  if (geluid) {
    myDFPlayer.volume(30);

    // 2 train stop at station

    if (outputwaardetekst=="Stoomfluit") {myDFPlayer.play(1);}
    if (outputwaardetekst=="Trainstop") {myDFPlayer.play(2);}
    if (outputwaardetekst=="Trainbraking") {myDFPlayer.play(3);}
    if (outputwaardetekst=="Crossing") {myDFPlayer.play(5);}
    if (outputwaardetekst=="Trainhorn") {myDFPlayer.play(6);}
    if (outputwaardetekst=="Loudsound") {myDFPlayer.play(7);}
    if (outputwaardetekst=="Synthsound") {myDFPlayer.play(8);}
    
    if (outputwaardetekst=="Closedoors") {myDFPlayer.play(9);}
    if (outputwaardetekst=="Trainhorn2") {myDFPlayer.play(10);}
    if (outputwaardetekst=="NSfluit") {myDFPlayer.play(11);}
    

    
    geluid = false;
  }

  /* if (closedoors) {
    myDFPlayer.volume(30);
    myDFPlayer.play(9);
    stoomfluit = false;
  }
  */

  if (speedmode) {


                                        if (outputwaardetekst=="slow") {modeselect[1] = 0x01;}
                                        if (outputwaardetekst=="normal") {modeselect[1] = 0x02;}
                                        if (outputwaardetekst=="fast") {modeselect[1] = 0x03;}
                                        if (outputwaardetekst=="ludicous") {modeselect[1] = 0x04;}


    // modeselect[1] = 0x04;
    pRemoteCharacteristic->writeValue((uint8_t*)modeselect, 2, true);
    speedmode = false;

  }

  if (led) {
    if (outputwaardetekst=="led1on") {digitalWrite(12, HIGH);}
    if (outputwaardetekst=="led1off") {digitalWrite(12, LOW);}
    if (outputwaardetekst=="led2on") {digitalWrite(14, HIGH);}
    if (outputwaardetekst=="led2off") {digitalWrite(14, LOW);}
    if (outputwaardetekst=="led3on") {digitalWrite(33, HIGH);}
    if (outputwaardetekst=="led3off") {digitalWrite(33, LOW);}
    if (outputwaardetekst=="led4on") {digitalWrite(32, HIGH);}
    if (outputwaardetekst=="led4off") {digitalWrite(32, LOW);}
    led = false;
  }

  if (motorspeed)  {
    motorspeed = false;
      valueselect[1] = outputwaarde;
      valueselect[2] = outputwaarde;
//      if (ch[1]<1470) ch1 = map(ch[1],940,1480,-127,0);
//      else{
//        if (ch[1]>1500) ch1 = map(ch[1],1501,2045,0,127);
//        else ch1=0;
//      }
  //    valueselect[2]=50;    //Read ch1 output from the interupts, range is from  about 900 to 2000, so remap and reformat into 7 bit signed number.
      
//      if (ch[2]<1460) ch2 = map(ch[2],970,1460,-127,0);
//      else{
//        if (ch[2]>1500) ch2 = map(ch[2],1501,2005,0,127);
//       else ch2=0;
//      }
      valueselect[3]=50;   //Same as above, but for ch2

      valueselect[4]=1900;
      
     if (ch[3] > 1500)
     {  
          if (mode_fast == false)
          {
            mode_fast = true;
            modeselect[1] = 0x04;
            pRemoteCharacteristic->writeValue((uint8_t*)modeselect, 2, true);
          }    
     }
     else
     {
      if (mode_fast == true)
        {
          mode_fast = false;
          modeselect[1] = 0x02;
          pRemoteCharacteristic->writeValue((uint8_t*)modeselect, 2, true);
        }
     }        //Mode select. On my GT3, I have a 2 position switch. 1st pos: normal mode, 2nd pos: ludicorus mode.
     
/*      if (DEBUGGING){
           Serial.print("Steering: ");
           Serial.print(ch[1]);
           Serial.print("\t");
           // Serial.print(valueselect[2], HEX);
           Serial.print("\n");
           Serial.print("Throttle: ");
           Serial.print(ch[2]);
           Serial.print("\t");
           // Serial.print(valueselect[3], HEX);
           Serial.print("\n");
           Serial.print("CH3: ");
           Serial.print(ch[3]);
           Serial.print("\t");
           // Serial.print(modeselect[1], HEX);
           Serial.print("\n");
      }
      */
       pRemoteCharacteristic->writeValue((uint8_t*)valueselect, 6, true);     // send motor values to BuWizz
  }
    }  
  
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}
