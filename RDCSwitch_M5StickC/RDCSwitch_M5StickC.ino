/*
 * 梅田８階RDCの、最前列・スクリーン前の照明スイッチをON/OFFする
 */

#include <M5StickCPlus.h>

// サーボモータ
#include <Servo.h>
#define MOTOR_PIN   26    // サーボモータの信号ピン
#define MOTOR_DELAY 150   // サーボモータ動作後の待ち時間
#define SWITCH_OFF  130   // サーボモータの回転角（通常時）
#define SWITCH_ON   115   // サーボモータの回転角（スイッチ動作時）
Servo motor;

/***********************************************************
* WiFi接続
***********************************************************/
#include <WiFiClientSecure.h>

// WiFiアクセスポイントの種別を指定
// WEP認証（パスワードのみで認証）の場合は次の行を有効にする。
// 次の行をコメントアウトするとWPA2認証を試みる。
#define WIFI_WEP

// WiFiアクセスポイントの設定
#ifdef WIFI_WEP
// WEP認証のアクセスポイント
// [スケッチ例] の [WiFiClientInsecure] を参考にした
const char* WIFI_SSID     = "RDC-test";  // SSID of your WiFi access point
const char* WIFI_PASSWORD = "RDC-test";  // WEP password
#else
// WPA2認証のアクセスポイント
// [スケッチ例] の [WiFiClientSecureEnterprise] を参考にした
#include "esp_wpa2.h"
const char* WIFI_SSID     = "OIT-AirLAN.1x";  // SSID
const char* WIFI_USERID   = "********";       // User ID
const char* WIFI_PASSWORD = "********";       // Password
#endif

/*
// WiFi接続を確認し、再トライ、接続できなければ再起動を行う
void confirmWiFiConnection()
{
  int counter = 0;

  while (WiFi.status() != WL_CONNECTED) {  // 接続できるまで待つ
    delay(500);         // 0.5秒待ってリトライ
    Serial.print(".");  // 進捗表示
    counter++;
    if (60 <= counter) {   // 30秒で接続できなければリセット
      Serial.println("\nERROR: Restarting ESP32.\n");
      ESP.restart();
    }
  }
  Serial.print("WiFi connected: ");
  Serial.println(WIFI_SSID);
}
*/

/***********************************************************
* WEBサーバー
***********************************************************/
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server(80);

int status = 0;

void handleRoot() {
  server.send(200, "text/plain", "Request accepted.");
/*  
  Timeoutエラーが発生するのでここでは実行しない。
  // サーボモータを回転してもとに戻す
  motor.write(SWITCH_ON);
  delay(MOTOR_DELAY);
  motor.write(SWITCH_OFF);
  delay(MOTOR_DELAY);
*/
  status = 1;
}

void handleNotFound() {
  String message = "Invalid option in URI.\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/***********************************************************
* Arduinoメイン制御部
***********************************************************/

void displayInformation() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(3);
  M5.Lcd.print(WIFI_SSID);
  M5.Lcd.print("\n\nhttp://");
  M5.Lcd.print(WiFi.localIP());
}

void setup() {

  // システム初期化
  M5.begin();
  M5.Lcd.begin();

  // 画面表示の初期化
  M5.Lcd.setRotation(1);         // スクリーンの向き
  M5.Lcd.setTextSize(2);         // フォントサイズ
  M5.Lcd.printf("Connecting to WiFi: %s ", WIFI_SSID);

  // WiFi接続
#ifdef WIFI_WEP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#else
  WiFi.disconnect(true);  // 念の為切断してから処理開始
  WiFi.mode(WIFI_STA);    // init wifi mode
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERID, strlen(WIFI_USERID)); 
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERID, strlen(WIFI_USERID)); 
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD)); 
  esp_wifi_sta_wpa2_ent_enable(); //set config settings to enable function
  WiFi.begin(WIFI_SSID);   // connect to wifi
#endif

  // WiFi接続の確認
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {  // 接続できるまで待つ
    delay(500);         // 0.5秒待ってリトライ
    M5.Lcd.print(".");
    counter++;
    if (60 <= counter) {   // 30秒で接続できなければリセット
      M5.Lcd.print("\nERROR: Restarting WiFi\n");
      counter = 0;
      ESP.restart();
    }
  }

  // サーボモーターの初期化
  pinMode(MOTOR_PIN, OUTPUT);
  motor.attach(MOTOR_PIN);
  delay(MOTOR_DELAY);
  motor.write(SWITCH_OFF);
  delay(MOTOR_DELAY);

  // WEBサーバーの開始
  if (MDNS.begin("esp32")) {
    M5.Lcd.print("\n");
  }
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  // 画面表示
  displayInformation();
}

void loop() {
  WiFiClientSecure client;
  server.handleClient();
  if (status) {

    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextColor(WHITE, BLUE);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(3);
    M5.Lcd.print("\nRequest accepted.");

    // サーボモータを回転してもとに戻す
    motor.write(SWITCH_ON);
    delay(MOTOR_DELAY);
    motor.write(SWITCH_OFF);
    delay(MOTOR_DELAY);

    // 画面表示を戻す
    delay(3000);
    M5.Lcd.setTextColor(WHITE, BLACK);
    displayInformation();

    status = 0;
  }
  delay(2); //allow the cpu to switch to other tasks
}
