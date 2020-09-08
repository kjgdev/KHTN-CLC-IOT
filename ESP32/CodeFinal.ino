#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
//-------------- Wifi ------------------------//
#include "html.h"
#include <EEPROM.h>
#include <WebServer.h>
const char *ssid = "ESP32-SETUP";
const char *password = "12345678";
WebServer server(80); // khởi tạo sever ở port 80
//-------------- Define port esp ------------------------//
#define SPEAKER 13
#define BUTTON 14
#define BUTTONLED 15
//-------------- Define camera esp ------------------------//
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
//-------------- Define websocket ------------------------//
String websocket_ip = "";
String websocket_port = "";
using namespace websockets;
WebsocketsClient client;
//-------------- Init value device ------------------------//
int buttonState = 0;
int BassTab[]={1911,1702,1516,1431,1275,1136,1012};//bass 1~7
//-------------- Define mail ------------------------//
#include "ESP32_MailClient.h"
SMTPData smtpData;
void sendCallback(SendStatus info);
#define emailSenderAccount    "minhkhangit.dev@gmail.com"    
#define emailSenderPassword   "0123456789khang"
#define emailRecipient        "minhkhang.it123@gmail.com"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "Thong bao ring bell"
//-------------- Define time ------------------------//
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;
//-------------- Define json ------------------------//
#include <ArduinoJson.h>
char jsonOutput[128];

//---------------------------------------------------------//
//--------------------- Function --------------------------//

//--------------------- Wifi động -------------------------//
// -> trả về trang login wifi
void ketnoi(){
  String s = MAIN_page;
  server.send(200,"text/html",s); // gửi dưới dạng html 
}
// -> Khi đã kết nối được wifi
void ketnoi1(){
  server.send(200,"text/plain","Hello World"); // gửi dưới dạng html 
}

// -> ghi tài khoản, mat khau wifi vao ROM
void cai_dat(){
  String tenwf = server.arg("tenwifi");
  String mk = server.arg("matkhau");
  String ws_ip = server.arg("ip");
  String ws_port = server.arg("port");
  
  Serial.print("SSID: ");
  Serial.println(tenwf);
  Serial.print("Pass: ");
  Serial.println(mk);
  Serial.print("IP: ");
  Serial.println(ws_ip);
  Serial.print("PORT: ");
  Serial.println(ws_port);
  
  if (tenwf.length() > 0 && mk.length() > 0 && ws_ip.length() > 0 && ws_port.length() > 0) {
    // -> Xóa rỗng bộ nhớ
    for (int i = 0; i < 96; ++i) {
      EEPROM.write(i, 0);
    }
    // -> Ghi ssid
    for (int i = 0; i < tenwf.length(); ++i){
      EEPROM.write(i, tenwf[i]);
    }
    // -> Ghi mật khẩu
    for (int i = 0; i < mk.length(); ++i){
      EEPROM.write(32 + i, mk[i]);
    }
    // -> Ghi ip
    for (int i = 0; i < ws_ip.length(); ++i){
      EEPROM.write(96 + i, ws_ip[i]);
    }
    // -> Ghi port
    for (int i = 0; i < ws_port.length(); ++i){
      Serial.println("write port");
      EEPROM.write(140 + i, ws_port[i]);
    }
  EEPROM.commit();
  }
  server.send(200,"text/plain","Restarting in 10 seconds"); // gửi dưới dạng html 
  Serial.println("Restarting in 10 seconds");
  delay(10000);
  Serial.println("Restart");
  ESP.restart();
}

// -> Test connect wifi: true: kết nối thành công, false: tạo server phát wifi
bool testWifi(void)
{
  int c = 0;
  Serial.println("Chờ kết nối");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print(".");
    c++;
  }
  Serial.println("");
  Serial.println("Không thể kết nối vì quá thời gian");
  return false;
}
//--------------------- Init các cổng ------------------------------//
void pinInit()
{
  pinMode(SPEAKER,OUTPUT); // initialize the SPEAKER pin as an output
  digitalWrite(SPEAKER,LOW);   
  pinMode(BUTTONLED, OUTPUT);// initialize the LED pin as an output
  pinMode(BUTTON, INPUT);// initialize the BUTTON pin as an input
}
//--------------------- Gui mail thong bao ------------------------------//
void sendMailNotification(){
  String websocket_ip = "";
  for (int i = 96; i < 140; ++i)
  {
    websocket_ip += char(EEPROM.read(i));
  }
  Serial.print("Websocket IP: ");
  Serial.println(websocket_ip);
  String str1 = "<div style=\"color:#2f4468;\"><h3>";
  String str2 = ":8000/home</h3><p>- Sent from ESP32 board</p></div>";
  String result = "";
  result += str1;
  result += websocket_ip;
  result += str2;
  Serial.println(result);
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("ESP32", emailSenderAccount);
  smtpData.setPriority("High");
  smtpData.setSubject(emailSubject);
  smtpData.setMessage(result, true);
  smtpData.addRecipient(emailRecipient);
  smtpData.setSendCallback(sendCallback);
  if (!MailClient.sendMail(smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
  //Clear all data from Email object to free memory
  smtpData.empty();
}
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}
//--------------------- Phat am thanh ------------------------------//
void sound(uint8_t note_index)
{
    for(int i=0;i<100;i++)
    {
       digitalWrite(SPEAKER,HIGH);
       delayMicroseconds(BassTab[note_index]);
       digitalWrite(SPEAKER,LOW);
       delayMicroseconds(BassTab[note_index]);
    }
}
//--------------------- revice command open door ------------------------------//
void waitOpenDoor(){
  HTTPClient http;
  http.begin("http://192.168.1.88:8000/openDoor");
  int httpResponseCode = http.GET();
  if(httpResponseCode == 200){
    String payload = http.getString();
    if(payload == "1"){
      Serial.println("revice");
    }
  }
  else{
     Serial.println("error");
  }
  http.end();
}
//--------------------- send log open door ------------------------------//
void sendLog(){
  HTTPClient http;
  http.begin("http://192.168.1.88:8000/log");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST();
  if(httpResponseCode == 200){
    Serial.println("send success");
  }
  else{
    Serial.println("error");
  }
  http.end();
}

//--------------------- send log open door ------------------------------//
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}



//----------------------------------------------------------------------------//
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  Serial.println();
  // -> Init các cổng
  pinInit();
  //------------------        Camera init        ----------------------//
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 40;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  //--------------------------------------------------------------------------------------------------------------//

  //------------------        wifi init        ----------------------//
  EEPROM.begin(512); //Initialasing EEPROM
  // ----------- Xóa sao khi test -----------------//
//  for (int i = 0; i < 180; ++i) {
//      EEPROM.write(i, 0);
//  }
  // ----------- Xóa sao khi test -----------------//
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);// tạo điểm truy cập wifi (Set up a soft access point to establish a WiFi network)
  String tenwifie = "";
  String mke = "";
  Serial.println("Đọc ROM");
  // -> Doc tai khoan tren rom 
  for (int i = 0; i < 32; ++i)
  {
    tenwifie += char(EEPROM.read(i));
  }
   // -> Doc mat khau tren rom 
  for (int i = 32; i < 96; ++i)
  {
    mke += char(EEPROM.read(i));
  }
  for (int i = 96; i < 140; ++i)
  {
    websocket_ip += char(EEPROM.read(i));
  }
  for (int i = 140; i < 180; ++i)
  {
    websocket_port += char(EEPROM.read(i));
  }
  
  Serial.print("SSID: ");
  Serial.println(tenwifie);
  Serial.print("Pass: ");
  Serial.println(mke);
  Serial.print("Websocket IP: ");
  Serial.println(websocket_ip);
  Serial.print("Websocket PORT: ");
  Serial.println(websocket_port);
  
  // -> Ket noi Wifi
  WiFi.begin((char*)tenwifie.c_str(), (char*)mke.c_str());
  
  // -> Ket noi wifi ngoai thanh cong
  if (testWifi())
  {
    Serial.println("Kết nối thành công");
    Serial.print("Địa chỉ IP:");
    Serial.println(WiFi.localIP());
    // -> Khởi tạo Websocket
    Serial.println("Connect Websocket");
    //while(!client.connect(websocket_server_host, websocket_server_port, "/")){
    Serial.println((char*)websocket_ip.c_str());
    Serial.println(websocket_port.toInt());
    while(!client.connect((char*)websocket_ip.c_str(), websocket_port.toInt(), "/")){
      delay(500);
      Serial.print(".");
    }
    Serial.println("Websocket Connected!");
    // -> Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
    return;
  }
  // -> Phat wifi, tao server de get tai khoan, mat khau wifi
  else {  
      Serial.println("Cấu hình điểm kết nối");
      WiFi.softAP(ssid, password);
      Serial.print("Địa chỉ Ip của ESP:");
      Serial.println(WiFi.softAPIP());
      Serial.print("SSID: ");
      Serial.println(ssid);
      Serial.print("Pass: ");
      Serial.println(password);
      server.on("/",ketnoi);// khi bạn 192.168.x.x thì nó sẽ thực hiện hàm ketnoi đầu tiên
      server.on("/caidat",cai_dat);
      server.begin(); // bắt đầu khởi động sever
  }
}

void loop() {
  // -> xử lý các yêu cầu từ bên client
  if (WiFi.status() != WL_CONNECTED){
    server.handleClient();
    delay(200);
    digitalWrite(BUTTONLED,HIGH);
    delay(200);
    digitalWrite(BUTTONLED,LOW);
  }
  else{
    // -> Read status button
    buttonState = digitalRead(BUTTON);

    if (buttonState == LOW) {
      Serial.println("press");
      digitalWrite(BUTTONLED, HIGH);
//      for(int note_index=0;note_index<7;note_index++)
//      {
//         sound(note_index);
//         delay(50);
//      }
      //sendMailNotification();
      sendLog();
    }
    else if (buttonState == HIGH){
    // turn LED off:
     digitalWrite(BUTTONLED, LOW);
    }

    // -------------- waiting command from server ---------------//
    waitOpenDoor();
    
    // ----------------- Send data camera -----------------------//
    camera_fb_t *fb = esp_camera_fb_get();
    if(!fb){
      Serial.println("Camera capture failed");
      esp_camera_fb_return(fb);
      return;
    }

    if(fb->format != PIXFORMAT_JPEG){
      Serial.println("Non-JPEG data not implemented");
      return;
    }

    client.sendBinary((const char*) fb->buf, fb->len);
    esp_camera_fb_return(fb);
    // ----------------------------------------------------------//
  }
}
