const char MAIN_page[] PROGMEM = R"=====(
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP8266 kết nối WiFi bằng điện thoại</title>
</head>
<body>
<h4>Input value Conenct</h4>
<form action="caidat">
  SSID<br> <input style="margin-top:10px;width: 100px;" name="tenwifi" type="text"/> </br>
  Mật khẩu<br> <input style="margin-top:10px;width: 100px;" name="matkhau" type="text"/> </br>
  IP<br> <input style="margin-top:10px;width: 100px;" name="ip" type="text"/> </br>
  PORT<br> <input style="margin-top:10px;width: 100px;" name="port" type="text"/> </br>
  <input type="submit" value="Connect"/>
</form>
  
</body>

</html>
)=====";
