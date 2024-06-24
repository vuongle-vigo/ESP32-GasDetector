#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c  //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // QT-PY / XIAO

int DELAY_TIME = 500;
const int RED_PIN = 27;		// led đỏ
const int GREEN_PIN = 26;	// led xanh lá
const int BLUE_PIN = 25;
const int ALARM_PIN = 33;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*
   Alarm
*/
float sinVal;
int toneVal;

byte MQ6_Pin = 34;  // Định nghĩa chân D34 cho cảm biến MQ6

float Referance_V = 3300.0;  // Điện áp tham chiếu của ESP32 tính bằng mV
float RL = 1.0;  // Giá trị điện trở tải (RL) trong module là 1k Ohm
float Ro = 10.0;  // Giá trị Ro là 10k Ohm
float mVolt = 0.0;
const float Ro_clean_air_factor = 10.0;

void setup() {
  Serial.begin(9600);  // Thiết lập tốc độ baud rate 9600
  pinMode(MQ6_Pin, INPUT);  // Định nghĩa chân D34 là chân đầu vào
  delay(500);

  //Khoi tao coi
  pinMode(ALARM_PIN, OUTPUT);

  //khoi tao led
  pinMode(RED_PIN, OUTPUT);
	pinMode(GREEN_PIN, OUTPUT);
	pinMode(BLUE_PIN, OUTPUT);

  // Initialize OLED display
  display.begin(i2c_Address, OLED_RESET);
  display.display();  // Hiển thị màn hình chào mừng
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Wait for 30 sec warmup");
  display.display();

  Serial.println("Wait for 30 sec warmup");
  delay(3000);  // Chờ 30 giây để cảm biến làm nóng
  Serial.println("Warmup Complete");

  for (int i = 0; i < 30; i++) {
    mVolt += Get_mVolt(MQ6_Pin);
  }
  mVolt = mVolt / 30.0;  // Tính giá trị điện áp trung bình cho 30 lần đo
  Serial.print("Voltage at D34 Pin = ");
  Serial.print(mVolt);
  Serial.println("mVolt");

  float Rs = Calculate_Rs(mVolt);
  Serial.print("Rs = ");
  Serial.println(Rs);

  Ro = Rs / Ro_clean_air_factor;
  Serial.print("Ro = ");
  Serial.println(Ro);
  Serial.println(" ");
  mVolt = 0.0;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Warmup Complete");
  display.display();
}

void loop() {
  // Gọi hàm displayAllBasicColors để thể hiện toàn bộ các màu cơ bản !
  displayAllBasicColors();
  // stopRGB();
  for (int i = 0; i < 500; i++) {
    mVolt += Get_mVolt(MQ6_Pin);
  }
  mVolt = mVolt / 500.0;  // Tính giá trị điện áp trung bình cho 500 lần đo
  Serial.print("Voltage at D34 Pin = ");
  Serial.print(mVolt);
  Serial.println(" mV");

  float Rs = Calculate_Rs(mVolt);
  Serial.print("Rs = ");
  Serial.println(Rs);

  float Ratio_RsRo = Rs / Ro;
  Serial.print("Rs/Ro = ");
  Serial.println(Ratio_RsRo);

  unsigned int LPG_ppm = LPG_PPM(Ratio_RsRo);
  Serial.print("LPG ppm = ");
  Serial.println(LPG_ppm);

  // Hiển thị các thông số trên màn hình OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Voltage: ");
  display.print(mVolt);
  display.println(" mV");
  display.print("Rs: ");
  display.println(Rs);
  display.print("Rs/Ro: ");
  display.println(Ratio_RsRo);
  display.print("LPG ppm: ");
  display.println(LPG_ppm);
  display.display();
  if (LPG_ppm > 1000) {
    showRGB(1);
    startAlarm();
  }
  else {
    stopRGB();
    noTone(ALARM_PIN);
  }
  delay(100);  // Thêm một chút độ trễ để giảm tải cho vi điều khiển
	
}

float Calculate_Rs(float Vo) {
  // Tính giá trị Rs
  float Rs = (Referance_V - Vo) * (RL / Vo);
  return Rs;
}

unsigned int LPG_PPM(float RsRo_ratio) {
  // Tính nồng độ PPM của LPG
  float ppm = pow((RsRo_ratio / 18.446), (1 / -0.421));
  return (unsigned int)ppm;
}

float Get_mVolt(byte AnalogPin) {
  // Tính điện áp từ giá trị ADC
  int ADC_Value = analogRead(AnalogPin);
  delay(1);
  float mVolt = ADC_Value * (Referance_V / 4096.0);
  return mVolt;
}

// Cài đặt hàm displayAllBasicColors
 
void displayAllBasicColors()
{
	// Tắt toàn bộ các led - cái này dễ mà ha
 
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, LOW);
	
	delay(DELAY_TIME);
 
	// Chỉ bật led đỏ
 
	digitalWrite(RED_PIN, HIGH);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, LOW);
 
	delay(DELAY_TIME);
 
	// Chỉ bật led xanh lá
 
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, HIGH);
	digitalWrite(BLUE_PIN, LOW);
 
	delay(DELAY_TIME);
 
	// Chỉ bật led xanh dương
 
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, HIGH);
 
	delay(DELAY_TIME);
 
	// Bật màu vàng bắng cách bật led đỏ và xanh
 
	digitalWrite(RED_PIN, HIGH);
	digitalWrite(GREEN_PIN, HIGH);
	digitalWrite(BLUE_PIN, LOW);
 
	delay(DELAY_TIME);
 
	// Xanh lam (Cyan) bằng cách bật led xanh lá và xanh dương
 
	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, HIGH);
	digitalWrite(BLUE_PIN, HIGH);
 
	delay(DELAY_TIME);
 
	// Tím (đỏ xanh dương)
 
	digitalWrite(RED_PIN, HIGH);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(BLUE_PIN, HIGH);
 
	delay(DELAY_TIME);
 
	// Màu trắng (tất cả các led)
	// Mình không hiểu nổi vụ con công tô màu cho con quạ :3, đáng lẻ phải ra màu trắng chứ, mà thế quái nào lại ra màu đen :3, chắc do con công pha màu kém quá :D
 
	digitalWrite(RED_PIN, HIGH);
	digitalWrite(GREEN_PIN, HIGH);
	digitalWrite(BLUE_PIN, HIGH);
 
	delay(DELAY_TIME);
}
 
// Cài đặt hàm showSpectrum
 
 
void showSpectrum()
{
	
	for (int i = 0; i < 0; i++)
	{
		showRGB(i);  // Call RGBspectrum() with our new x
		// delay(10);   // Delay 10ms 
	}
}
 
 
// Cài đặt hàm showRGB(int color)
 
// Chúng ta sẽ cài đặt hàm showRGB để mỗi khi nhận một giá trị từ 0 - 767
// nó sẽ chuyển dần dầm các màu của con đèn led rgb thành các màu đỏ - cam - vàng - lục - lam - chàm - tím
 
// mình chia nó thành 3 khu
// đỏ - xanh lục
// xanh lục - xanh lam
// xanh lam - đỏ
 
// gồm có 4 mốc
// 0   = đỏ chét (đỏ 100%)
// 255 = xanh lục 100%
// 511 = xanh dương (100%)
// 767 = lại là đỏ chét
 
// Những con số nằm giữa các màu sẽ được tính toán theo công thức bên dưới (đọc dễ hiểu mà) để ra được các màu cần thiết
 
 
 
void showRGB(int color)
{
	int redPWM;
	int greenPWM;
	int bluePWM;
 
 
 
	if (color <= 255)          // phân vùng 1
	{
		redPWM = 255 - color;    // red đi từ sáng về tắt
		greenPWM = color;        // green đi từ tắt thành sáng
		bluePWM = 0;             // blue luôn tắt
	}
	else if (color <= 511)     // phân vùng 2
	{
		redPWM = 0;                     // đỏ luôn tắt
		greenPWM = 255 - (color - 256); // green đi từ sáng về tắt
		bluePWM = (color - 256);        // blue đi từ tắt thành sáng
	}
	else // color >= 512       // phân vùng 3
	{
		redPWM = (color - 512);         // red tắt rồi lại sáng
		greenPWM = 0;                   // green luôn tắt nhé
		bluePWM = 255 - (color - 512);  // blue sáng rồi lại tắt
	}
 
	// rồi xuất xung ra và chơi thôi :3
 
	analogWrite(RED_PIN, redPWM);
	analogWrite(BLUE_PIN, bluePWM);
	analogWrite(GREEN_PIN, greenPWM);
}

void stopRGB() {
  analogWrite(RED_PIN, 0);
	analogWrite(BLUE_PIN, 0);
	analogWrite(GREEN_PIN, 0);
}

void startAlarm() {
  for(int x=180; x<181; x++){
            // convert degrees to radians then obtain value
            sinVal = (sin(x*(3.1412/180)));
            // generate a frequency from the sin value
            toneVal = 2000+(int(sinVal*1000));
            tone(ALARM_PIN, toneVal);
            // delay(2000); 
     } 
}
