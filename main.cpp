#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <U8g2_for_Adafruit_GFX.h>

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define PANEL_CHAIN 1

MatrixPanel_I2S_DMA *matrix = nullptr;
U8G2_FOR_ADAFRUIT_GFX u8g2;
WebServer server(80);

String message = "ESP32 64x32 MATRIX";
int16_t scrollX = PANEL_WIDTH;
uint32_t lastFrame = 0;
uint8_t mode = 0;
uint8_t brightness = 80;
bool serialWasConnected = false;

const char *apSsid = "ESP32S3-Matrix";
const char *apPassword = "kskblzdjd";

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return matrix->color565(r, g, b);
}

String htmlEscape(const String &value) {
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); i++) {
    char c = value[i];
    if (c == '&') {
      escaped += F("&amp;");
    } else if (c == '<') {
      escaped += F("&lt;");
    } else if (c == '>') {
      escaped += F("&gt;");
    } else if (c == '"') {
      escaped += F("&quot;");
    } else if (c == '\'') {
      escaped += F("&#39;");
    } else {
      escaped += c;
    }
  }
  return escaped;
}

void configureMatrix() {
  HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANEL_CHAIN);

  // ESP32-S3 safe pin mapping. GPIO19/20 are used by native USB and must be avoided.
  mxconfig.gpio.r1 = 4;
  mxconfig.gpio.g1 = 5;
  mxconfig.gpio.b1 = 6;
  mxconfig.gpio.r2 = 7;
  mxconfig.gpio.g2 = 15;
  mxconfig.gpio.b2 = 16;
  mxconfig.gpio.a = 8;
  mxconfig.gpio.b = 9;
  mxconfig.gpio.c = 10;
  mxconfig.gpio.d = 11;
  mxconfig.gpio.e = -1;  // 64x32 1/16-scan panels normally do not use E.
  mxconfig.gpio.clk = 12;
  mxconfig.gpio.lat = 13;
  mxconfig.gpio.oe = 14;

  mxconfig.clkphase = false;

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  matrix->begin();
  matrix->setBrightness8(brightness);
  matrix->clearScreen();

  u8g2.begin(*matrix);
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  u8g2.setFontMode(1);
  u8g2.setFontDirection(0);
}

uint16_t textWidth(const String &text) {
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  return u8g2.getUTF8Width(text.c_str());
}

void drawUtf8Text(int16_t x, int16_t baseline, const String &text, uint16_t color) {
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  u8g2.setForegroundColor(color);
  u8g2.setBackgroundColor(0);
  u8g2.drawUTF8(x, baseline, text.c_str());
}

String htmlPage() {
  String page;
  page.reserve(7200);
  page += F("<!doctype html><html><head><meta charset='utf-8'>");
  page += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<title>点阵屏控制台</title><style>");
  page += F(":root{color-scheme:light;--bg:#f5f7fb;--panel:#fff;--ink:#18202d;--muted:#667085;--line:#d8dee8;--brand:#1769e0;--soft:#eef5ff}");
  page += F("*{box-sizing:border-box}body{margin:0;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI','PingFang SC','Microsoft YaHei',sans-serif;background:var(--bg);color:var(--ink)}");
  page += F("main{width:min(720px,100%);margin:0 auto;padding:22px 16px 34px}.top{padding:18px 0 14px}h1{font-size:26px;line-height:1.2;margin:0 0 8px;font-weight:750}");
  page += F(".sub{margin:0;color:var(--muted);font-size:14px}.panel{background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:18px;box-shadow:0 10px 30px rgba(28,42,68,.08)}");
  page += F(".status{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin:0 0 16px}.stat{border:1px solid var(--line);border-radius:8px;background:#fbfcff;padding:12px}.k{font-size:12px;color:var(--muted);margin-bottom:6px}.v{font-weight:700;font-size:15px;word-break:break-all}");
  page += F("label{display:block;margin:16px 0 8px;color:#344054;font-size:14px;font-weight:650}input,select,button{width:100%;font-size:17px;border-radius:8px;border:1px solid var(--line);background:#fff;color:var(--ink);padding:12px 13px;outline:none}");
  page += F("input:focus,select:focus{border-color:var(--brand);box-shadow:0 0 0 3px rgba(23,105,224,.12)}input[type=range]{padding:0;accent-color:var(--brand)}");
  page += F(".row{display:grid;grid-template-columns:1fr 64px;gap:12px;align-items:center}.pill{text-align:center;padding:11px 10px;border-radius:8px;background:var(--soft);color:var(--brand);font-weight:800}");
  page += F("button{margin-top:20px;border:0;background:var(--brand);color:#fff;font-weight:750;letter-spacing:0;cursor:pointer;box-shadow:0 8px 18px rgba(23,105,224,.18)}button:active{transform:translateY(1px)}");
  page += F(".hint{margin:16px 0 0;color:var(--muted);font-size:13px;line-height:1.55}.mono{font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace}");
  page += F("@media(max-width:560px){.status{grid-template-columns:1fr}main{padding:18px 12px 28px}.panel{padding:15px}h1{font-size:23px}}</style></head><body><main>");
  page += F("<section class='top'><h1>智能点阵屏控制台</h1><p class='sub'>ESP32-S3 · 32x64 HUB75 LED Matrix</p></section>");
  page += F("<section class='panel'><div class='status'><div class='stat'><div class='k'>当前亮度</div><div class='v'>");
  page += brightness;
  page += F("</div></div></div><form action='/set' method='get'>");
  page += F("<label>显示内容</label><input name='text' maxlength='80' value='");
  page += htmlEscape(message);
  page += F("' placeholder='请输入要显示的文字'>");
  page += F("<label>动画模式</label><select name='mode'>");
  page += F("<option value='0'");
  if (mode == 0) page += F(" selected");
  page += F(">从右向左滚动</option><option value='1'");
  if (mode == 1) page += F(" selected");
  page += F(">自下向上移动</option><option value='2'");
  if (mode == 2) page += F(" selected");
  page += F(">标题闪烁</option></select>");
  page += F("<label>显示亮度</label><div class='row'><input type='range' min='5' max='180' value='");
  page += brightness;
  page += F("' name='bri' oninput='v.textContent=this.value'><span class='pill' id='v'>");
  page += brightness;
  page += F("</span></div><button type='submit'>应用到点阵屏</button></form>");
  page += F("<p class='hint'>连接 Wi-Fi <span class='mono'>");
  page += apSsid;
  page += F("</span>，密码 <span class='mono'>");
  page += apPassword;
  page += F("</span>，浏览器访问 <span class='mono'>http://192.168.4.1</span>。</p>");
  page += F("</section></main></body></html>");
  return page;
}

void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage());
}

void handleSet() {
  if (server.hasArg("text")) {
    message = server.arg("text");
    message.trim();
    if (message.length() == 0) {
      message = "ESP32 64x32 MATRIX";
    }
    scrollX = PANEL_WIDTH;
  }

  if (server.hasArg("mode")) {
    mode = server.arg("mode").toInt() % 3;
  }

  if (server.hasArg("bri")) {
    brightness = constrain(server.arg("bri").toInt(), 5, 180);
    matrix->setBrightness8(brightness);
  }

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void configureWebServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  Serial.print("Wi-Fi AP: ");
  Serial.println(apSsid);
  Serial.print("Password: ");
  Serial.println(apPassword);
  Serial.print("Web: http://");
  Serial.println(WiFi.softAPIP());
}

void readCommand(Stream &stream) {
  if (!stream.available()) {
    return;
  }

  String line = stream.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return;
  }

  if (line.startsWith("MODE=")) {
    mode = line.substring(5).toInt() % 3;
    stream.print("Mode set to ");
    stream.println(mode);
  } else if (line.startsWith("BRI=")) {
    brightness = constrain(line.substring(4).toInt(), 5, 180);
    matrix->setBrightness8(brightness);
    stream.print("Brightness set to ");
    stream.println(brightness);
  } else {
    message = line;
    scrollX = PANEL_WIDTH;
    stream.print("Message set to: ");
    stream.println(message);
  }
}

void drawScrollLeft() {
  matrix->fillScreen(0);
  drawUtf8Text(scrollX, 20, message, rgb(0, 220, 255));

  scrollX--;
  if (scrollX < -textWidth(message)) {
    scrollX = PANEL_WIDTH;
  }
}

void drawVerticalMove() {
  static int16_t y = PANEL_HEIGHT;
  matrix->fillScreen(0);
  drawUtf8Text(2, y, message, rgb(255, 180, 0));
  y--;
  if (y < -2) {
    y = PANEL_HEIGHT;
  }
}

void drawFlashTitle() {
  static bool on = true;
  matrix->fillScreen(on ? rgb(0, 0, 20) : 0);
  matrix->drawRect(0, 0, PANEL_WIDTH, PANEL_HEIGHT, rgb(0, 90, 255));
  drawUtf8Text(4, 20, message, on ? rgb(255, 255, 255) : rgb(60, 60, 60));
  on = !on;
}

void setup() {
  delay(1500);
  Serial.begin(115200);
  Serial.setTimeout(20);
  Serial.println();
  Serial.println("Booting ESP32-S3 matrix demo...");

  configureMatrix();
  configureWebServer();

  Serial.println("Ready. Send text, MODE=0/1/2, or BRI=5..180.");
}

void loop() {
  bool serialConnected = Serial;
  if (serialConnected && !serialWasConnected) {
    Serial.println("Ready. Send text, MODE=0/1/2, or BRI=5..180.");
  }
  serialWasConnected = serialConnected;

  readCommand(Serial);
  server.handleClient();

  uint32_t now = millis();
  uint16_t interval = (mode == 2) ? 450 : 35;
  if (now - lastFrame < interval) {
    return;
  }
  lastFrame = now;

  if (mode == 0) {
    drawScrollLeft();
  } else if (mode == 1) {
    drawVerticalMove();
  } else {
    drawFlashTitle();
  }
}
