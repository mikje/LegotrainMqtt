# LegotrainMqtt
Lego trein besturing via MQTT

Shopping list:

- LilyGO TTGO T-Energy ESP32-WROVER - with 18650 Battery Holder ( https://www.tinytronics.nl/en/development-boards/microcontroller-boards/with-wi-fi/lilygo-ttgo-t-energy-esp32-wrover-with-18650-battery-holder ) 
- CQRobot JST PH 2,0 mm pitch 2-polige JST IC mannelijke stekker, vrouwelijke stopcontacten behuizing en T-vorm krimpklem. 50 sets/200 stuks JST PH connector adapter kabel montage. ( https://www.amazon.nl/dp/B09DP9FZTX?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1 )
- dfplayer module
- sd card (8gb suffice)
- 4 resistors (220 ohm)
- speaker:  AZDelivery 10 x Høyttaler 3 watt 8 ohm minihøyttaler ( https://www.amazon.nl/dp/B09PL7WXXD?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1 ) 
- battery 18650 (rechargable)
- some small cables to solder between esp and connectors

Software:

- Arduino IDE
- MQTT broker
- MQTT explorer or Home Assistant

Hardware:

- 3D printer for the housing
- pla filament



Install:

Open INO file in arduino IDE

fill in:
- mqtt user
- mqtt pass
- ssid
- ssid pass
- ip of mqtt broker
- mac of buwizz
- mqtt broker port 

Flash INO file to ESP32

Put mp3 files on sd card (order is important!)
insert sd card in dfplayer


