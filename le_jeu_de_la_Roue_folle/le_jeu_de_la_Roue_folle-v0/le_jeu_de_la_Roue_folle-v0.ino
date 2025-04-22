
#include <LiquidCrystal_I2C.h>  
LiquidCrystal_I2C lcd(0x27,16,2);

#define pin_roue1 2
#define pin_roue2 3

#define pin_bp1 10
#define pin_bp2 8
#define pin_bp3 12

#define pin_v1 11
#define pin_v2 9
#define pin_v3 13

bool roue1,roue2;

volatile unsigned long t1,t2,mt1,mt2 ;

void setup() {

  attachInterrupt(digitalPinToInterrupt(pin_roue1), top_roue1, RISING);
  attachInterrupt(digitalPinToInterrupt(pin_roue2), top_roue2, RISING);

  Serial.begin(9600);

  pinMode( pin_roue1 , INPUT_PULLUP);
  pinMode( pin_roue2 , INPUT_PULLUP);
  
  pinMode( pin_bp1 , INPUT_PULLUP);
  pinMode( pin_bp2 , INPUT_PULLUP);
  pinMode( pin_bp3 , INPUT_PULLUP);

  pinMode( pin_v1 , OUTPUT);
  pinMode( pin_v2 , OUTPUT);
  pinMode( pin_v3 , OUTPUT);

  digitalWrite(pin_v3,HIGH);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("Bonjour");
}


void top_roue1(){
  t1 =  millis()-mt1;
  mt1 = millis();
}

void top_roue2(){
  t2 = millis() - mt2;
  mt2 = millis();
}


void loop() {



  if (!digitalRead(pin_bp1)) digitalWrite(pin_v1, HIGH); else digitalWrite(pin_v1, LOW);
  if (!digitalRead(pin_bp2)) digitalWrite(pin_v2, HIGH); else digitalWrite(pin_v2, LOW);
  if (!digitalRead(pin_bp3)) digitalWrite(pin_v3, HIGH); else digitalWrite(pin_v3, LOW);


  lcd.setCursor(0,0);
  
  lcd.print("r1 = ");
  lcd.print( int( 1000.0/t1*400 ) ); // un tour de roue = 400 mm
  lcd.print(" mm/s");
  
  lcd.setCursor(0,1);
  lcd.print("r2 = ");
  lcd.print( int( 1000.0/t2*400 ) );
  lcd.print(" mm/s");

}
