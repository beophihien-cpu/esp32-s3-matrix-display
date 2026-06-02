# ESP32-S3 智能 32x64 点阵显示系统使用指南

本文档用于说明本工程的硬件连接、软件烧录、网页控制、中文显示和常见故障排查。

## 1. 项目功能

本系统基于 ESP32-S3 和 32x64 HUB75 RGB LED 点阵屏，实现以下功能：

- ESP32-S3 驱动 32x64 HUB75 点阵屏刷新显示
- 支持网页无线控制，无需手机 App
- 支持中文 UTF-8 文本输入与点阵显示
- 支持 3 种显示模式
- 支持网页调节亮度
- 支持 USB 串口调试和命令控制

当前网页控制信息：

```text
Wi-Fi 热点：ESP32S3-Matrix
热点密码：kskblzdjd
网页地址：http://192.168.4.1
```

## 2. 需要准备的硬件

| 器件 | 数量 | 说明 |
| --- | ---: | --- |
| ESP32-S3 开发板 | 1 | 当前工程按 ESP32-S3 配置 |
| 32x64 HUB75 RGB 点阵屏 | 1 | 常见 64x32 面板，也就是宽 64、高 32 |
| 5V 外接电源 | 1 | 建议 5V 4A 或更高 |
| 杜邦线 | 若干 | 用于连接 HUB75 信号线 |
| USB 数据线 | 1 | 用于烧录和串口调试 |

注意：32x64 RGB 点阵屏电流较大，不能直接用 ESP32-S3 的 USB 或 5V 引脚给屏幕供电。

## 3. 软件环境

VS Code 需要安装：

```text
PlatformIO IDE
C/C++
```


主要文件：

| 文件 | 作用 |
| --- | --- |
| `platformio.ini` | PlatformIO 工程配置 |
| `src/main.cpp` | 主程序 |
| `README.md` | 简短说明 |
| `使用指南.md` | 本详细指南 |

## 4. PlatformIO 配置说明

当前 `platformio.ini` 使用 ESP32-S3：

```ini
[env:esp32-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
monitor_filters = send_on_enter
upload_speed = 115200
upload_port = /dev/cu.usbmodem1201
monitor_port = /dev/cu.usbmodem1201
```

用到的库：

```ini
lib_deps =
  mrfaptastic/ESP32 HUB75 LED MATRIX PANEL DMA Display@^3.0.15
  adafruit/Adafruit GFX Library@^1.12.4
  olikraus/U8g2_for_Adafruit_GFX@^1.8.0
```

说明：

- `ESP32 HUB75 LED MATRIX PANEL DMA Display`：负责 HUB75 点阵屏 DMA 刷新
- `Adafruit GFX Library`：提供基础图形接口
- `U8g2_for_Adafruit_GFX`：负责中文 UTF-8 字体渲染
- `WebServer` 和 `WiFi`：ESP32 Arduino 框架自带，用于网页控制

## 5. 烧录程序

打开 VS Code 后，用 PlatformIO 打开工程目录。

烧录前请关闭串口监视器，否则会占用串口。

在 PlatformIO 左侧菜单中执行：

```text
PROJECT TASKS -> esp32-s3 -> General -> Upload
```

烧录成功时终端会出现：

```text
Writing at ...
Hash of data verified.
Hard resetting via RTS pin...
[SUCCESS]
```

如果提示串口被占用：

```text
Could not exclusively lock port
```

说明 monitor 还开着。关闭 monitor 后重新上传。

## 6. 串口监视器

打开方式：

```text
PROJECT TASKS -> esp32-s3 -> General -> Monitor
```

串口参数：

```text
端口：/dev/cu.usbmodem1201
波特率：115200
```

启动后正常输出：

```text
Booting ESP32-S3 matrix demo...
Wi-Fi AP: ESP32S3-Matrix
Password: kskblzdjd
Web: http://192.168.4.1
Ready. Send text, MODE=0/1/2, or BRI=5..180.
```

PlatformIO monitor 输入时可能不会显示正在输入的字符，按回车后程序会回显命令结果。

串口命令：

| 命令 | 功能 |
| --- | --- |
| `HELLO` | 更新显示内容为 HELLO |
| `硬件课设` | 更新显示内容为中文 |
| `MODE=0` | 从右向左滚动 |
| `MODE=1` | 自下向上移动 |
| `MODE=2` | 标题闪烁 |
| `BRI=40` | 设置亮度为 40 |

## 7. 网页控制

ESP32-S3 会创建 Wi-Fi 热点：

```text
ESP32S3-Matrix
```

连接密码：

```text
kskblzdjd
```

连接后，在浏览器打开：

```text
http://192.168.4.1
```

网页支持：

- 输入显示内容
- 输入中文文本
- 切换动画模式
- 调节显示亮度
- 应用到点阵屏

多个设备可以同时打开网页，但最后提交设置的设备会覆盖之前的设置。演示时建议由一个人操作。

## 8. 硬件连接表

### 8.1 供电连接

| 模块 | 引脚 | 连接到 | 说明 |
| --- | --- | --- | --- |
| LED 点阵屏 | VCC / 5V | 外接 5V 电源正极 | 不要接 ESP32-S3 的 5V 供电脚 |
| LED 点阵屏 | GND | 外接 5V 电源负极 | 必接 |
| ESP32-S3 | GND | 外接 5V 电源负极 | 必须共地 |
| ESP32-S3 | USB | 电脑 | 烧录和调试 |

必须保证：

```text
ESP32-S3 GND
LED 屏 GND
外接 5V 电源 GND
```

三者连接在一起。

### 8.2 HUB75 信号连接

| HUB75 信号 | ESP32-S3 GPIO | 作用 |
| --- | ---: | --- |
| R1 | GPIO4 | 上半屏红色数据 |
| G1 | GPIO5 | 上半屏绿色数据 |
| B1 | GPIO6 | 上半屏蓝色数据 |
| R2 | GPIO7 | 下半屏红色数据 |
| G2 | GPIO15 | 下半屏绿色数据 |
| B2 | GPIO16 | 下半屏蓝色数据 |
| A | GPIO8 | 行地址 A |
| B | GPIO9 | 行地址 B |
| C | GPIO10 | 行地址 C |
| D | GPIO11 | 行地址 D |
| E | 不接 | 32x64 常见 1/16 扫描一般不用 |
| CLK | GPIO12 | 时钟 |
| LAT / STB | GPIO13 | 锁存 |
| OE | GPIO14 | 输出使能 |
| GND | ESP32-S3 GND / 电源 GND | 共地 |
| VCC / 5V | 外接 5V 正极 | 屏幕供电 |

不要使用以下引脚接屏：

```text
GPIO19
GPIO20
```

ESP32-S3 的 `GPIO19/20` 通常用于原生 USB。接到 HUB75 后可能导致串口监视器反复断开或跳动。

### 8.3 HUB75 输入口确认

屏幕背面一般有两个接口：

```text
IN / INPUT / DIN：输入口，接 ESP32-S3
OUT / OUTPUT / DOUT：输出口，用于级联下一块屏
```

本项目只使用一块屏，应连接 `IN` 输入口。

## 9. 推荐上电顺序

1. 断开所有电源。
2. 按连接表接好 HUB75 信号线。
3. 接好 LED 屏 5V 外部电源线。
4. 确认 ESP32-S3、LED 屏、电源三者 GND 共地。
5. 给 LED 屏上电。
6. 插上 ESP32-S3 USB。
7. 手机连接热点并打开网页。
8. 调低亮度，例如 `40`。
9. 输入文字测试显示。

## 10. 中文显示说明

当前程序使用 U8g2 中文字体：

```cpp
u8g2_font_wqy12_t_gb2312
```

优点：

- 支持常用中文
- 支持 UTF-8 输入
- 适合 32 像素高度的点阵屏

可以在网页中输入：

```text
硬件课设
你好ESP32
智能点阵屏
```

注意：

- 32x64 屏幕宽度有限，长中文适合滚动显示
- 字体越大，占用 Flash 越多
- 当前 Flash 占用约 28.6%，ESP32-S3 空间足够

## 11. 修改字体

当前代码中有两处字体设置：

```cpp
u8g2.setFont(u8g2_font_wqy12_t_gb2312);
```

可尝试改成：

```cpp
u8g2_font_wqy14_t_gb2312
```

或：

```cpp
u8g2_font_wqy16_t_gb2312
```

建议：

| 字体 | 效果 | 适用场景 |
| --- | --- | --- |
| `wqy12` | 字小，显示内容多 | 滚动文字 |
| `wqy14` | 更清楚 | 常规展示 |
| `wqy16` | 更大 | 短标题 |

如果改大字体，可能需要调整文字基线，例如把：

```cpp
drawUtf8Text(scrollX, 20, message, ...);
```

改成：

```cpp
drawUtf8Text(scrollX, 22, message, ...);
```

## 12. 常见问题排查

### 12.1 上传失败：端口被占用

现象：

```text
Could not exclusively lock port
```

原因：

```text
串口监视器正在占用端口
```

解决：

- 关闭 monitor
- 或在终端里按 `Ctrl+C`
- 或点 VS Code 终端右上角垃圾桶关闭

### 12.2 上传失败：No serial data received

原因可能是：

- 没有插对 USB 口
- 没进入下载模式
- 开发板型号配置错误

当前已验证可用端口：

```text
/dev/cu.usbmodem1201
```

如果仍失败，可尝试：

1. 按住 `BOOT`
2. 点一下 `RST/EN`
3. 开始上传
4. 出现 `Writing at ...` 后松开 `BOOT`

### 12.3 monitor 打开后没有输出

可能原因：

- 程序启动信息已经错过
- monitor 打开太晚

解决：

- 按一下 `RST/EN`
- 或直接输入命令回车

### 12.4 monitor 反复跳动

常见原因：

```text
GPIO19/20 被拿去接 HUB75，干扰了 ESP32-S3 原生 USB
```

解决：

- 不要使用 `GPIO19/20`
- 按本指南接线表接屏

### 12.5 屏幕完全不亮

检查：

- LED 屏是否有外接 5V
- 电源电流是否足够
- ESP32-S3 和 LED 屏是否共地
- 是否接在 HUB75 的输入口 `IN`
- `OE`、`CLK`、`LAT` 是否接对

### 12.6 屏幕花屏

检查：

- A/B/C/D 行地址线
- CLK 时钟线
- LAT 锁存线
- GND 是否牢固共地

### 12.7 颜色不对

检查：

- R1/G1/B1
- R2/G2/B2

如果红绿蓝颜色互换，通常是 RGB 数据线顺序接错。

### 12.8 上下半屏反了

检查：

- R1 和 R2 是否接反
- G1 和 G2 是否接反
- B1 和 B2 是否接反

### 12.9 只有半屏亮

检查：

- R2/G2/B2 是否接好
- D 地址线是否接好
- 屏幕是否为 1/16 扫描

### 12.10 闪烁严重

可能原因：

- 电源电流不足
- 亮度过高
- GND 连接不牢

解决：

- 网页亮度调到 `40` 左右
- 使用 5V 4A 或更高电源
- 缩短供电线

## 13. 演示流程建议

课设演示时可以按以下顺序：

1. 展示 ESP32-S3 和 32x64 点阵屏硬件连接。
2. 说明 LED 屏使用独立 5V 电源并和 ESP32-S3 共地。
3. 手机连接 `ESP32S3-Matrix` 热点。
4. 打开 `http://192.168.4.1`。
5. 输入中文，例如 `硬件课设`。
6. 切换模式：
   - 从右向左滚动
   - 自下向上移动
   - 标题闪烁
7. 调节亮度，展示网页实时控制。
8. 说明 ESP32-S3 不支持经典蓝牙串口，因此使用 Wi-Fi 网页控制代替。

## 14. 当前项目和 PPT 原方案的区别

PPT 原方案是：

```text
ESP32 + 16x64 点阵
蓝牙 / Wi-Fi 无线控制
```

当前实际方案是：

```text
ESP32-S3 + 32x64 HUB75 RGB 点阵屏
Wi-Fi 热点网页控制
中文 UTF-8 字体显示
```

主要变化：

- 屏幕从 `16x64` 改为 `32x64`
- 主控从普通 ESP32 改为 ESP32-S3
- 蓝牙串口改为 Wi-Fi 网页控制
- 使用 HUB75 RGB 六路数据线
- 避开 ESP32-S3 的 `GPIO19/20`

## 15. 安全注意事项

- 不要用 ESP32-S3 给 LED 屏直接供电。
- 不要带电插拔 HUB75 排线。
- 首次点亮时亮度不要太高。
- 外接电源正负极不能接反。
- ESP32-S3 和 LED 屏必须共地。
- 如果闻到异味或屏幕异常发热，立即断电检查。
