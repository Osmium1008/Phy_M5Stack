#include <M5Stack.h>

#include <BluetoothSerial.h>
#include <SPI.h>
#include <WiFi.h>

// WiFiのパスワードとか証明書とか入れておく場所
#include "env.h"

char pattern[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
};

const int OE=5,DYNA=1,DYNB=3,DATA=22,LATCH=2,CLK=21,SSp=16,MI=17;

SPIClass hspi(HSPI);

const int JST = 3600 * 9;

void write_num(String filename, int bx, int by){
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

void task0(void* arg) {
  while(true){

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
    

  delay(200);

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
  
  delay(20000);

  time_t t;
  time(&t);
  struct tm *tm_now;
  tm_now = localtime(&t);

  int month = tm_now->tm_mon+1,day=tm_now->tm_mday;

  write_num(String(month/10),0,0);
  write_num(String(month%10),4,0);
  write_num(String(day/10),0,6);
  write_num(String(day%10),4,6);

  write_num("sunny",16,0);

  delay(200);
  xTaskCreatePinnedToCore(task0, "Task0", 4096, NULL, 1, NULL, 1);
}


void loop() {
  delay(1000);
  write_num("rainy",16,0);
  delay(1000);
  write_num("snow",16,0);
  delay(1000);
  write_num("cloudy",16,0);
  delay(1000);
  write_num("sunny",16,0);
}
