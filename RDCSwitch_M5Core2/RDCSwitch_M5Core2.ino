/*
 * 梅田８階RDCの、最前列・スクリーン前の照明スイッチをON/OFFする（リモコン側）
 * WiFi、WEBサーバー機能については次のサンプルスケッチを元にしている。
 * [ファイル] > [スケッチ例] > [M5Core2] > [Advanced] > [WIFI] > [BasicHttpClient]
 * ボタン機能については次のサンプルスケッチを参考にしている。
 * [ファイル] > [スケッチ例] > [M5Core2] > [Touch] > [events_buttons_gestures_rotation]
 *【制約事項】
 * ・壁スイッチ側から Timeout のステータスが帰ってくる理由は不明
*/


//#define KBD  // IPアドレスの入力・変更用のKBDを表示する


// WIFI_MULTIが定義されていたら WiFiMulti.h を使う。
// そうでなければ WiFiClientSecure.h を使う。
//#define WIFI_MULTI

// WiFiClientSecure.h を使うときにWEP認証を使う。
#define WIFI_WEP

#include <M5Core2.h>
#include <Arduino.h>
#include <WiFi.h>
#ifdef WIFI_MULTI
#include <WiFiMulti.h>
#else
#include <WiFiClientSecure.h>
#endif
#include <HTTPClient.h>

// WiFiアクセスポイントの設定
#if defined(WIFI_MULTI) || defined(WIFI_WEP)
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

#define DEFAULT_IP_ADDRESS  "192.168.11.2"  // スイッチ側のIPアドレス初期値

/******************************************************
* 画面表示とユーザーI/F
******************************************************/

#define BUTTON_LEFT  8     // ボタン領域の左端位置
#define BUTTON_TOP  30     // ボタン領域の上端位置
#define WI  51             // 数字ボタンの幅
#define HI  38             // 数字ボタンの高さ
#define MESSAGE_AREA  (116+HI+1)  // メッセージ表示領域のY座標

// 文字入力バッファ
#define STR_MAX 80
char ip_address[STR_MAX+2];
char str[STR_MAX+2];
int strNum = 0;

// ボタンの色
ButtonColors on_clrs = {RED, WHITE, WHITE};     // タップした時の色 (背景, 文字列, ボーダー)
ButtonColors off_clrs = {BLACK, WHITE, WHITE};  // 指を離した時の色 (背景, 文字列, ボーダー)
ButtonColors on_clrs2 = {RED, WHITE, WHITE};    // タップした時の色 (背景, 文字列, ボーダー)
ButtonColors off_clrs2 = {BLUE, WHITE, WHITE};  // 指を離した時の色 (背景, 文字列, ボーダー)
// ボタン定義名( X, Y, 幅, 高さ, 回転, ラベル, 指を離した時の色指定, タッチした時の色指定, ラベル位置）
#ifdef KBD
Button b0(0, 0, 0, 0, false ,"0", off_clrs, on_clrs, MC_DATUM);
Button b1(0, 0, 0, 0, false, "1", off_clrs, on_clrs, MC_DATUM);
Button b2(0, 0, 0, 0, false, "2", off_clrs, on_clrs, MC_DATUM);
Button b3(0, 0, 0, 0, false, "3", off_clrs, on_clrs, MC_DATUM);
Button b4(0, 0, 0, 0, false, "4", off_clrs, on_clrs, MC_DATUM);
Button b5(0, 0, 0, 0, false, "5", off_clrs, on_clrs, MC_DATUM);
Button b6(0, 0, 0, 0, false, "6", off_clrs, on_clrs, MC_DATUM);
Button b7(0, 0, 0, 0, false, "7", off_clrs, on_clrs, MC_DATUM);
Button b8(0, 0, 0, 0, false, "8", off_clrs, on_clrs, MC_DATUM);
Button b9(0, 0, 0, 0, false, "9", off_clrs, on_clrs, MC_DATUM);
Button bPeriod(0, 0, 0, 0, false, ".", off_clrs, on_clrs, MC_DATUM);
Button bClear(0, 0, 0, 0, false, "CLEAR", off_clrs, on_clrs, MC_DATUM);
Button bEnter(0, 0, 0, 0, false, "ENTER", off_clrs, on_clrs, MC_DATUM);
#endif
Button bSwitch(0, 0, 0, 0, false, "ON/OFF LIGHTS", off_clrs2, on_clrs2, MC_DATUM);

// ボタンのハンドラ
void updateData(char* data) {
  if (STR_MAX <= strNum) return;
  strcat(str, data);
  strNum++;
  dispEditingString();
}
#ifdef KBD
void b0Tapped(Event& e) { updateData((char*)"0"); }
void b1Tapped(Event& e) { updateData((char*)"1"); }
void b2Tapped(Event& e) { updateData((char*)"2"); }
void b3Tapped(Event& e) { updateData((char*)"3"); }
void b4Tapped(Event& e) { updateData((char*)"4"); }
void b5Tapped(Event& e) { updateData((char*)"5"); }
void b6Tapped(Event& e) { updateData((char*)"6"); }
void b7Tapped(Event& e) { updateData((char*)"7"); }
void b8Tapped(Event& e) { updateData((char*)"8"); }
void b9Tapped(Event& e) { updateData((char*)"9"); }
void bPeriodTapped(Event& e) { updateData((char*)"."); }
void bClearTapped(Event& e) {
  strcpy(str, "");
  strNum = 0;
  dispEditingString();
}
void bEnterTapped(Event& e) {
  strcpy(ip_address, str);
  strcpy(str, "");
  strNum = 0;
  dispIPAddress();
}
#endif
void bSwitchTapped(Event& e) {
  moveSwitch();
  // スイッチ側のステータスを一定時間表示して、元の表示戻す
  delay(3000);
  clearInstruction(BLACK);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Buttons.draw();
}

void doButtons() {
  // ボタンへのハンドラー設定
#ifdef KBD
  b0.addHandler(b0Tapped, E_RELEASE);
  b1.addHandler(b1Tapped, E_RELEASE);
  b2.addHandler(b2Tapped, E_RELEASE);
  b3.addHandler(b3Tapped, E_RELEASE);
  b4.addHandler(b4Tapped, E_RELEASE);
  b5.addHandler(b5Tapped, E_RELEASE);
  b6.addHandler(b6Tapped, E_RELEASE);
  b7.addHandler(b7Tapped, E_RELEASE);
  b8.addHandler(b8Tapped, E_RELEASE);
  b9.addHandler(b9Tapped, E_RELEASE);
  bPeriod.addHandler(bPeriodTapped, E_RELEASE);
  bClear.addHandler(bClearTapped, E_RELEASE);
  bEnter.addHandler(bEnterTapped, E_RELEASE);
#endif
  bSwitch.addHandler(bSwitchTapped, E_RELEASE);
#ifdef KBD
  b0.repeatDelay = 200;
  b1.repeatDelay = 200;
  b2.repeatDelay = 200;
  b3.repeatDelay = 200;
  b4.repeatDelay = 200;
  b5.repeatDelay = 200;
  b6.repeatDelay = 200;
  b7.repeatDelay = 200;
  b8.repeatDelay = 200;
  b9.repeatDelay = 200;
  bPeriod.repeatDelay = 200;
  bClear.repeatDelay = 200;
  bEnter.repeatDelay = 200;
#endif
  bSwitch.repeatDelay = 200;

  // ボタンの位置設定
#ifdef KBD
  b0.set(BUTTON_LEFT+WI*0, BUTTON_TOP,    WI+1, HI+1);
  b1.set(BUTTON_LEFT+WI*1, BUTTON_TOP,    WI+1, HI+1);
  b2.set(BUTTON_LEFT+WI*2, BUTTON_TOP,    WI+1, HI+1);
  b3.set(BUTTON_LEFT+WI*3, BUTTON_TOP,    WI+1, HI+1);
  b4.set(BUTTON_LEFT+WI*4, BUTTON_TOP,    WI+1, HI+1);
  b5.set(BUTTON_LEFT+WI*0, BUTTON_TOP+HI, WI+1, HI+1);
  b6.set(BUTTON_LEFT+WI*1, BUTTON_TOP+HI, WI+1, HI+1);
  b7.set(BUTTON_LEFT+WI*2, BUTTON_TOP+HI, WI+1, HI+1);
  b8.set(BUTTON_LEFT+WI*3, BUTTON_TOP+HI, WI+1, HI+1);
  b9.set(BUTTON_LEFT+WI*4, BUTTON_TOP+HI, WI+1, HI+1);
  bPeriod.set(BUTTON_LEFT+WI*5, BUTTON_TOP   , WI+1, HI+HI+1); 
  bClear.set(BUTTON_LEFT,  116  , 150, HI);
  bEnter.set(164,          116  , 150, HI);
#endif
#ifdef KBD
  bSwitch.set(BUTTON_LEFT, 116+HI+10 , 306, HI+HI);
#else
  bSwitch.set(BUTTON_LEFT, 50, 306, HI*4);
#endif

  // ボタン表示
  M5.Buttons.draw();
}

// メッセージエリアを消去する
void clearInstruction(int color) {
  M5.Lcd.fillRect( 0, MESSAGE_AREA,
      M5.Lcd.width()-1, M5.Lcd.height()-MESSAGE_AREA, color);
}

// IPアドレス表示エリアを消去する
void clearIPAddress(int color) {
  M5.Lcd.fillRect( 0, 0, M5.Lcd.width()-1, BUTTON_TOP-1, color);
}

// 編集中のIPアドレスを表示
void dispEditingString() {
  clearIPAddress(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.printf(str);
}

// IPアドレスを表示
void dispIPAddress() {
  char s[STR_MAX+2];
  clearIPAddress(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(10, 20);
  strcpy(s, "Switch address: ");
  strcat(s, ip_address);
  M5.Lcd.printf(s);
}

/******************************************************
* WiFi接続とスイッチ制御
******************************************************/
#ifdef WIFI_MULTI
WiFiMulti wifiMulti;
#endif
HTTPClient http;

#ifndef WIFI_MULTI
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
#endif

// スイッチを動作させる関数
// 戻り値: 0=成功、1=失敗
int moveSwitch() {
  char url[STR_MAX+2+7];
  strcpy(url, "http://");
  strcat(url, ip_address);

  clearInstruction(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setCursor(0, MESSAGE_AREA+15);
  M5.Lcd.printf("Sending command to:\n %s", url);
#ifdef WIFI_MULTI
  while (wifiMulti.run() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(300);
  }
#endif
  M5.Lcd.print("\n");

  http.begin(url);
  http.setTimeout(5000);
  int httpCode;
  for (int limit=0; limit<5; limit++) {  // 接続できなかったら5回トライ
    httpCode = http.GET();
    if (httpCode != -1) break;  // -1は"connection refused"のエラーコード
    delay(200);
    continue;
  }
  if (httpCode > 0) {
    M5.Lcd.printf("Response code: (%d) ", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      M5.Lcd.println(payload);
    }
    http.end();
    return 0;
  }

  M5.Lcd.printf("Request failed: (%d) %s\n",
                httpCode, http.errorToString(httpCode).c_str());
  http.end();
  return 1;
}

void setup() {
  M5.begin();

#ifdef WIFI_MULTI
  // WiFi設定
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

#ifndef WIFI_MULTI
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
#endif

  strcpy(ip_address, DEFAULT_IP_ADDRESS);

  // ボタンとガイドメッセージを表示
  doButtons();
  dispIPAddress();
}

void loop() {
  M5.update();
  delay(2);
}
