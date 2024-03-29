#include <M5Stack.h>

#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>

// WiFiのパスワードとか証明書とか入れておく場所
#include "env.h"


char day_pattern[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


// weather : (16,0)~
// max_temp: (1,9), (5,9)
char weather_pattern[] = {
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,1,1,1,0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
};

const int OE=5,DYNA=1,DYNB=3,DATA=22,LATCH=2,CLK=21,SSp=16,MI=17;

SPIClass hspi(HSPI);

const int JST = 3600 * 9;

bool is_spi_initialized=false;

unsigned long last_received = 0;

void write_num(String filename, int bx, int by, char* pattern){
  SD.begin();
  File f = SD.open(String("/")+filename+".txt");
    if(f){
      int sz=f.size();
      int x=bx,y=by;
      for(int j=0;j<sz;j++){
        f.seek(j);
        char buf=f.read();

        if(buf=='\n'){
          y++;
          x=bx;
        }
        else if(buf!='\r'){
          pattern[x+y*32]=buf-'0';
          x++;
        }
      }
      f.close();
    }
}

void display_on_matrix(void* arg) {
  if(!is_spi_initialized){
    pinMode(LATCH, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(DYNA, OUTPUT);
    pinMode(DYNB, OUTPUT);
    pinMode(OE, OUTPUT);

    hspi.begin(CLK,MI,DATA,SSp);
    hspi.setBitOrder(LSBFIRST);
    hspi.setClockDivider(SPI_CLOCK_DIV2);
    hspi.setDataMode(SPI_MODE0);
    digitalWrite(LATCH, LOW);
    is_spi_initialized = true;
  }
  delay(200);

  while(true){
    if(millis()<=last_received + 60000){
      char* pattern=nullptr;
      if((unsigned long)((millis()-last_received)/4000)%2)pattern=day_pattern;
      else pattern=weather_pattern;

      for ( int i = 0; i < 4; i++) {

        for ( int j = 0; j < 16; j++ ){
          char s = 255;
          for (int k = 0; k < 8; k++) {
            s = s - (pattern[ i * 32 + ( (15 - j) % 4) * 128 + (j >> 2 ) * 8 + k] << k);
          }
          hspi.transfer(s);
        }

        digitalWrite(OE, LOW);           // パネル消灯
        digitalWrite(DYNA, i & 1 );      // ダイナミック点灯桁指定(LOW)
        digitalWrite(DYNB, i >> 1 & 1 ); // ダイナミック点灯桁指定(HIGH)
        digitalWrite(LATCH, HIGH);       // ラッチ解除
        digitalWrite(OE, HIGH);          // パネル点灯
        digitalWrite(LATCH, LOW);        // ラッチ

        delay(3) ;            // 3msウェイト
      }
    }
    else digitalWrite(OE, LOW);
  }
}

void onReceive(const uint8_t* mac_addr, const uint8_t* data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
      mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  M5.Lcd.println();
  M5.Lcd.printf("Last Packet Recv Data(%d): ", data_len);
  for (int i = 0; i < data_len; i++) {
    M5.Lcd.print(data[i]);
    M5.Lcd.print(" ");
  }
  if(data[2]!=255){
    int tmp = data[2] - 50;
    if(tmp<0){
      write_num(String(tmp+10),5,9,weather_pattern);
    }
    else{
      write_num(String(tmp/10),1,9,weather_pattern);
      write_num(String(tmp%10),5,9,weather_pattern);
    }
  }
  if(data[0]==230 && data[1]==155) write_num("cloudy",16,0, weather_pattern);
  else if(data[0]==230 && data[1]==153) write_num("sunny",16,0, weather_pattern);
  else if(data[0]==233 && data[1]==155 && data[3]==170) write_num("snow",16,0, weather_pattern);
  else write_num("rainy",16,0, weather_pattern);

  last_received = millis();
}

void setup() {
  M5.begin();
  Serial.begin(115200);

  while(!(M5.BtnA.read() || M5.BtnB.read()));
  const char *SSID;
  const char *PASSWORD;
  if(M5.BtnA.wasPressed()){
    SSID=SSID_HOME;
    PASSWORD=PASSWORD_HOME;
  }
  else{
    SSID=SSID_PHYS;
    PASSWORD=PASSWORD_PHYS;
  }

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
		delay(500);
  }
  
  configTime(JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  
  delay(20000);

  time_t t;
  time(&t);
  struct tm *tm_now;
  tm_now = localtime(&t);

  int month = tm_now->tm_mon+1,day=tm_now->tm_mday;

  write_num(String(month/10),0,1,day_pattern);
  write_num(String(month%10),4,1,day_pattern);
  write_num(String(day/10),12,1,day_pattern);
  write_num(String(day%10),16,1,day_pattern);
  String day_name;
  switch(tm_now->tm_wday){
    case 0:
      day_name = "Sun";
      break;
    case 1:
      day_name = "Mon";
      break;
    case 2:
      day_name = "Tue";
      break;
    case 3:
      day_name = "Wed";
      break;
    case 4:
      day_name = "Thu";
      break;
    case 5:
      day_name = "Fri";
      break;
    case 6:
      day_name = "Sat";
      break;
  }
  write_num(day_name,22,0,day_pattern);

  write_num("kadai", 0, 8, day_pattern);
  
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-Now Init Success");
  }
  esp_now_register_recv_cb(onReceive);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println(WiFi.macAddress());

  delay(200);
  M5.Lcd.println("Matrix Launched.");
  xTaskCreatePinnedToCore(display_on_matrix, "Task0", 4096, NULL, 1, NULL, 1);
}


void loop() {
  delay(1000);
  // write_num("rainy",16,0);
  // delay(1000);
  // write_num("snow",16,0);
  // delay(1000);
  // write_num("cloudy",16,0);
  // delay(1000);
  // write_num("sunny",16,0);
}
