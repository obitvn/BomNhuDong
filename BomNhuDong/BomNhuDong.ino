#include <EEPROM.h>
#include <Keypad.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "BasicStepperDriver.h"

#define MOTOR_STEPS 200
#define RPM 1

#define MICROSTEPS 1

#define DIR 2
#define STEP 5
#define ENABLE 6 // optional (just delete ENABLE from everywhere if not used)
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
LiquidCrystal lcd(A2,A3,A4,A5,3,4);

const byte rows = 4; //số hàng
const byte columns = 4; //số cột

uint16_t holdDelay = 1000; //Thời gian trễ để xem là nhấn 1 nút nhằm tránh nhiễu
uint16_t n = 3; // 
uint16_t state = 0; //nếu state =0 ko nhấn,state =1 nhấn thời gian nhỏ , state = 2 nhấn giữ lâu
char key = 0, temp;
uint16_t count=0;
char keys[rows][columns] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};
byte rowPins[rows] = {11,12,13,A0}; //Cách nối chân với Arduino
byte columnPins[columns] = {7,8,9,10};
Keypad matrix = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);
// 200 STEP ỨNG VỚI 0.1 ML



char phim;
uint32_t mode=1, mls, luu_luong;

String getString(int Length, char EscapeChar) {    // hàm này dùng để đọc số nhập vào khi cài đặt ml, lưu lượng                  
   String currentString="";                                                                       
  int count = 0;                                                                        
  char currentKey;
  while (matrix.getKey() != EscapeChar){                                              
    delay(100);                                                                  
  //if( matrix.keyStateChanged() )        
    currentKey = matrix.getKey();
    if( currentKey != NO_KEY){                                              
        if (currentKey != EscapeChar)
        {    
          lcd.print(currentKey);                               
          currentString += currentKey;                                    
         count ++;
         delay(200);                                                          
        if (count >= Length)
        {                                              
        return currentString;                                   
        }                                                                     
    }                                                                           
    else {       
      //added this line 4-20-2012 RW
      return currentString;                                          
    }                                                                               
    }                                                                              
  }                                                                                      
}                                                                                             



long convert_ml_to_step(long ml) // đổi số ml ra số bước
{
  long value;
  value = ml*1110;
  return value;//quay 5.55 vòng bơm được 1 ML 
}

void Bom_theo_luu_luong(uint8_t luu_luong) // tính theo ml / phút
{
  uint32_t buoc = convert_ml_to_step(luu_luong);
  uint32_t rpm = buoc/200;
  if((rpm<200)&&(rpm>0))
  {
  stepper.setRPM(rpm);
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.print("error          ");
  }
}

void setup() {
  // put your setup code here, to run once:
stepper.begin(RPM, MICROSTEPS);
Serial.begin(9600);//bật serial, baudrate 9600
 Serial.println("Test");
lcd.begin(16, 2);
lcd.setCursor(0, 0);
lcd.print("Pump");
mls = EEPROM.read(1); // đọc giá trị đã lưu
luu_luong = EEPROM.read(2);
}

void Check_mode()
{
  Serial.println(phim);
  if(phim=='*') // bấm * để chuyển giữa các chế độ
  {
    mode++;
    if(mode>3) mode = 0;
  }

  if(phim=='B') // bắt đầu bơm
  {
   mode=0;
   Serial.println("run");
   lcd.setCursor(0,0);
   lcd.print("Running         ");
   long buoc = convert_ml_to_step(mls);
   Serial.println(buoc);
   Bom_theo_luu_luong(luu_luong);
   stepper.move((uint32_t)buoc);
   lcd.setCursor(0,0);
   lcd.print("Done            "); // bơm xong
   lcd.setCursor(0,1);
   lcd.print("* back // B  run");
   while(phim==NO_KEY) // nếu chưa có phím được bấm, vẫn đợi
   {
   phim = matrix.getKey();
   }
   
  }
  else if(mode==1) // run
  {
   //Serial.println("home");
   lcd.setCursor(0,0);
   lcd.print("press B to run");
   lcd.setCursor(0,1);
   lcd.print("press * to setup");
   
  }
  else if(mode==2) // cài đặt số ml cần bơm
  {
   lcd.setCursor(0,0);
   lcd.print("Nhap ML:        ");
   lcd.setCursor(0,1);
   lcd.print("press # finish  ");
   lcd.setCursor(9,0);
   String test = getString(3,'#');
   mls = test.toInt();
   Serial.println("mls:");
   Serial.println(mls);
   char eep_mls=mls;
   EEPROM.write(1, eep_mls);
   lcd.setCursor(0,0);
   lcd.print("                 ");
   lcd.setCursor(0,1);
   lcd.print("Dang Luu.        ");
   delay(100);
   for(int i=0; i<5; i++)
   {
   lcd.setCursor(8+i,1);
   lcd.print(".");
   delay(100);
   }
   delay(100);
   mode = 3;
  }
  else if(mode==3) // cài đặt lưu lượng cần bơm 0-20
  {
   lcd.setCursor(0,0);
   lcd.print("Luu Luong:      ");
   lcd.setCursor(0,1);
   lcd.print("press # finish");
   lcd.setCursor(11,0);
   String test = getString(3,'#');
   luu_luong = test.toInt();
   Serial.println("luu luong:");
   Serial.println(luu_luong);
   char eep_luu_luong=luu_luong;
   EEPROM.write(2, eep_luu_luong);
   lcd.setCursor(0,0);
   lcd.print("               ");
   lcd.setCursor(0,1);
   lcd.print("Dang Luu.        ");
   delay(100);
   for(int i=0; i<5; i++)
   {
   lcd.setCursor(8+i,1);
   lcd.print(".");
   delay(100);
   }
   delay(100);
   mode = 1;
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
//stepper.rotate(360);
 phim = matrix.getKey(); // check bàn phím
 Check_mode(); // bắt đầu thực hiện các chế độ


}
