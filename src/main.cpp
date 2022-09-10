#include <M5Stack.h>

#include <ArduinoJson.h>
#include <M5GFX.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

// WiFiのパスワードとか証明書とか入れておく場所
#include "env.h"

static M5GFX lcd;
// 仕様書によるとデフォルトでセンサーのスレーブアドレスが
// (上7bit 0x40) + (write 0/read 1)らしいので
constexpr uint16_t SENSOR_ADDR = 0x40;
// 仕様書によると距離のデータは 0x5E,0x5F の2バイトに乗っているらしいので
constexpr uint16_t DISTANCE_ADDR = 0x5E;

constexpr uint8_t RLED_PIN = 1, GLED_PIN = 3, BLED_PIN = 17;

const String TOKYO_AREA_CODE = "130000";
const String TOKYO_SUBAREA_CODE = "130010";

// JMA(気象庁): DIGICERT
const char *JMA_HOST = "www.jma.go.jp";
const String JMA_PATH_TO_TOKYO_AREA =
    "/bosai/forecast/data/forecast/" + TOKYO_AREA_CODE + ".json";

// 天気予報 API（livedoor 天気互換）: ISRG(Let's Encrypt)
const char *WTSUKUMIJIMA_HOST = "weather.tsukumijima.net";
const String WTSUKUMIJIMA_PATH_TO_TOKYO_SUBAREA =
    "/api/forecast/city/" + TOKYO_SUBAREA_CODE;

const int HTTPS_PORT = 443;

String httpsGet(int port, const char *cert, const char *host,
                const String &path) {
  String res;

  WiFiClientSecure client;
  client.setCACert(cert);

  Serial.println("connect to" + String(host));

  int tl = millis();
  while (!client.connect(host, port)) {
    if (millis() > tl + 30000) {
      Serial.println("connection failed");
      goto joi;
    }
  }
  {
    Serial.println("Connected to server");

    client.println("GET " + path + " HTTP/1.1");
    client.println("HOST: " + String(host));
    client.println("Connection: close");
    client.println();

    if (!client) {
      res = "error";
    } else {
      res = client.readString();
    }
    client.readString();
    client.stop();
  }
joi:;
  return res;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello World!!");

  // ディスプレイを使う
  lcd.init();
  lcd.setFont(&fonts::lgfxJapanGothic_28);
  lcd.setBrightness(128);
  lcd.setCursor(0, 30);
  lcd.println("Hello World!!");

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    lcd.print('.');
    delay(1000);
  }
  lcd.printf("\nConnected\n");

  String res = httpsGet(HTTPS_PORT, ISRG_ROOT_X1, WTSUKUMIJIMA_HOST,
                        WTSUKUMIJIMA_PATH_TO_TOKYO_SUBAREA);
  String body = res.substring(res.indexOf("\r\n\r\n") + 4);

  lcd.fillRect(0, 60, 200, 200, 0x000000U);
  lcd.setCursor(0, 60);

  DynamicJsonDocument doc(4096);
  deserializeJson(doc, body);
  const char *public_time = doc["publicTime"];
  Serial.println(String(public_time));

  const char *forecast = doc["forecasts"][0]["telop"];
  const char *tmax = doc["forecasts"][0]["temperature"]["max"]["celsius"];

  lcd.println("今日の天気: " + String(forecast));
  lcd.printf("最高気温: %s℃\n", tmax);

  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  pinMode(BLED_PIN, OUTPUT);

  // I2C初期化 マスターになるらしい
  Wire.begin();
}

void loop() {
  // ディスプレイを一度クリア
  lcd.fillRect(0, 120, 200, 200, 0x000000U);
  lcd.setCursor(0, 120);

  // LEDを消灯 HIGHが消灯
  digitalWrite(RLED_PIN, HIGH);
  digitalWrite(GLED_PIN, HIGH);
  digitalWrite(BLED_PIN, HIGH);

  // 読み出したいアドレスをセンサーに送信
  Wire.beginTransmission(SENSOR_ADDR);
  Wire.write(DISTANCE_ADDR);
  int ans = Wire.endTransmission();

  if (ans) {
    Serial.printf("ERR No.%d\n", ans);

    //エラーあったら白で点灯 LOWが点灯
    digitalWrite(RLED_PIN, LOW);
    digitalWrite(GLED_PIN, LOW);
    digitalWrite(BLED_PIN, LOW);
  } else {
    uint8_t c[2];

    // センサーから指定したアドレスに乗ってるデータを貰う 2は2バイトの2
    Wire.requestFrom(SENSOR_ADDR, 2);
    c[0] = Wire.read();
    c[1] = Wire.read();
    Wire.endTransmission();

    // 距離を出してディスプレイに表示
    int dist = ((c[0] << 4) + c[1]) / 16 / 4;
    lcd.printf("distance: %dcm\n", dist);

    // 対応する色を点灯 LOWで点灯
    if (dist == 63) {
      digitalWrite(RLED_PIN, LOW);
    } else if (dist > 20) {
      digitalWrite(GLED_PIN, LOW);
    } else {
      digitalWrite(BLED_PIN, LOW);
    }
  }
  delay(500);
}
