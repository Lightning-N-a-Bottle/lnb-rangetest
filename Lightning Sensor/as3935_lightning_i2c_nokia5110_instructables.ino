
/*
 * AS3935 communicates with i2c in this sketch
 * and the Nokia 5110 uses SPI
 */
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

 Adafruit_PCD8544 display = Adafruit_PCD8544(8, 5, 4);
 
#include "I2C.h"
// include Playing With Fusion AXS3935 libraries
#include "PWFusion_AS3935_I2C.h"

// interrupt trigger global var        
volatile int8_t AS3935_ISR_Trig = 0;

// define some values used by the panel and buttons
int adc_key_in  = 0;
int z=0;
int mi=0;
#define btnLEFT   3
#define btnDOWN   2
#define btnUP     1
#define btnRIGHT  0
#define btnSELECT 4
#define btnNONE   5

// defines for hardware config
#define SI_PIN               9
#define IRQ_PIN              2        // digital pins 2 and 3 are available for interrupt capability
#define AS3935_ADD           0x03     // x03 - standard PWF SEN-39001-R01 config
#define AS3935_CAPACITANCE   104       // <-- SET THIS VALUE TO THE NUMBER LISTED ON YOUR BOARD 

// defines for general chip settings
#define AS3935_INDOORS    0
#define AS3935_OUTDOORS   1
#define AS3935_DIST_DIS   1
#define AS3935_DIST_EN    0

int l=0;
int n=0;
int lsDist=0;
int mapDist=0;
int m=0;
int dist=0;
int tt=0;
// prototypes
void AS3935_ISR();
//--- data that builds the onscreen icons
PWF_AS3935_I2C  lightning0((uint8_t)IRQ_PIN, (uint8_t)SI_PIN, (uint8_t)AS3935_ADD);
const unsigned char PROGMEM lightning_bmp[32] = {
0x01, 0xE0, 0x02, 0x20, 0x0C, 0x18, 0x12, 0x24, 0x21, 0x06, 0x10, 0x02, 0x1F, 0xFC, 0x01, 0xF0, 
0x01, 0xC0, 0x03, 0x80, 0x07, 0xF8, 0x00, 0xF0, 0x00, 0xC0, 0x01, 0x80, 0x01, 0x00, 0x01, 0x00  
};
const unsigned char PROGMEM cloud[32] = {
0x01, 0xE0, 0x02, 0x20, 0x0C, 0x18, 0x12, 0x24, 0x21, 0x06, 0x10, 0x02, 0x1F, 0xFC, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char PROGMEM lightning[32] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xF8, 0x01, 0xF0, 
0x01, 0xC0, 0x03, 0x80, 0x07, 0xF8, 0x00, 0xF0, 0x00, 0xC0, 0x01, 0x80, 0x01, 0x00, 0x01, 0x00
};

int read_buttons()
{
 adc_key_in = analogRead(0);      // read the value from Button Panel. Values returned if a button gets pressed and which one it was....
 
 if (adc_key_in > 1000) return btnNONE; 
 if (adc_key_in ==0 )   return btnLEFT;  
 if (adc_key_in > 310 and  adc_key_in< 330 )  return btnDOWN; 
 if (adc_key_in > 120  and  adc_key_in<145 )  return btnUP; 
 if (adc_key_in > 480  and  adc_key_in<550)  return btnRIGHT; 
 if (adc_key_in > 700 and  adc_key_in< 800)  return btnSELECT;   
 return btnNONE;  // when all others fail, return this...
}

//-----------the config menu
void menu () 
{
  delay(400);
  while (read_buttons() != 4){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Enter=Return"); 
  display.setCursor(0,8);
  display.println("Up = Mi");
  display.setCursor(0,18);
  display.println("down = KM");
  display.setCursor(0,28);
  display.println("Left=Indoors");
  display.setCursor(0,38);
  display.println("Right=Outdoors ");
  display.display();
  if (read_buttons() == 1){
 mi=0;  
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Set to KM");
  display.display();
  delay(1000);
  menu(); 
  }
  if (read_buttons() == 2){
 mi=1;
 display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Set to Miles");
  display.display();
  delay(1000);
  menu(); 
  }
  if (read_buttons() == 0){
 lightning0.AS3935_SetIndoors();   
 dist=0;
 display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Indoor");
  display.setCursor(0,20);
  display.println("Operation");
  display.display();
  delay(1000);
  menu(); 
  }
  if (read_buttons() == 3){
 lightning0.AS3935_SetOutdoors();  
 dist=1; 
 display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Outdoor");
  display.setCursor(0,20);
  display.println("Operation");
  display.display();
  delay(1000);
  menu(); 
  }
  }
  return;
 }


void setup()
{
  pinMode(6,OUTPUT);
  Serial.begin(115200);
  display.begin();
  // init done

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(25);

  display.display(); // show splashscreen
    display.clearDisplay();
     display.setCursor(0,0);
      display.setTextSize(1);
    display.println("Raht Lightning   Detector");
    display.setTextSize(1);
    display.drawBitmap(35, 22,  lightning_bmp, 16, 16, 1);
  display.setTextColor(BLACK);
  display.display();
  flash();
  display.display();
  delay(3000);
  
  // setup for the the I2C library: (enable pullups, set speed to 400kHz)
  I2c.begin();
  I2c.pullup(true);
  I2c.setSpeed(1); 
  delay(2);
  
  lightning0.AS3935_DefInit();   // set registers to default  
  // now update sensor cal for your application and power up chip
  lightning0.AS3935_ManualCal(AS3935_CAPACITANCE, AS3935_OUTDOORS, AS3935_DIST_EN);
                                 // AS3935_ManualCal Parameters:
                                 //   --> capacitance, in pF (marked on package)
                                 //   --> indoors/outdoors (AS3935_INDOORS:0 / AS3935_OUTDOORS:1)
                                 //   --> disturbers (AS3935_DIST_EN:1 / AS3935_DIST_DIS:2)
                                 // function also powers up the chip
                  
  // enable interrupt (hook IRQ pin to Arduino Uno/Mega interrupt input: 0 -> pin 2, 1 -> pin 3 )
  attachInterrupt(0, AS3935_ISR, RISING);
  lightning0.AS3935_PrintAllRegs();
  AS3935_ISR_Trig = 0;           // clear trigger

}

//------- this just gets played over and over..... and over...... and o......
void loop()
{
  display.clearDisplay();
  analogWrite(6,60); //---turn on the backlight
  //----- build the screen
   display.setCursor(0,0);
   display.println("LastStrike");
   display.setCursor(0,8);
   display.println("Dist: "+ String(lsDist));
   display.setCursor(50,8);
   //--- if we pick miles, do this
   if (mi==1){ display.println(" Mi");}
   //otherwise assume we want to do this....
   else { display.println(" Km");}
   display.setCursor(0,17);
   display.println("0");
   display.setCursor(72,17);
   //if miles--- 40 km = aaprox 25 miles
   if(mi==1){
   display.println("25");
   }
   else{
   //--- otherwise just scale it for 40 km
    display.println("40");
   }
   display.setCursor(0,39);
   display.println("Strikes: ");
   display.setCursor(48,39);
   display.println(String(l));
   display.setCursor(64,39);
   //-- place an indication that we're set to inside in the lower right corner of the display
   if (dist==1){display.println("Out");}
   else{
    //-- or Show were set for outdoors
    display.println("In");}
    //--draw the distance scale
   display.drawLine( 0,  37,  84, 37, BLACK);
  
   display.drawLine( 0,  25,  0, 37, BLACK);
   display.drawLine( 21,  35,  21, 37, BLACK);
   display.drawLine( 42,  35,  42, 37, BLACK);
   display.drawLine( 63,  35,  63, 37, BLACK);
   display.drawLine( 83,  25,  83, 37, BLACK);

   //display.fillCircle(mapDist,35,2,BLACK);
   if (l==0)
   //-- we begin with no strikes detected
   display.setCursor(9,24);
   display.println("No Strikes");
   if (l>0)
   //-if theres a strike. place a little zap at the distance it is...
   display.drawBitmap(mapDist, 22,  lightning, 16, 16, 1);
   if (mapDist>20){
    display.setCursor(9,24);
   display.println("Far Ligntning");
   }
   display.display();
  
  if(0 == AS3935_ISR_Trig){
  //   Serial.println(read_buttons());
  animateCloud();
  //Serial.println(read_buttons());
  if (read_buttons() !=5){
      menu();
  };
  // reset interrupt flag
  AS3935_ISR_Trig = 0;
  
  // now get interrupt source
  uint8_t int_src = lightning0.AS3935_GetInterruptSrc();
  if(0 == int_src)
  {
 //--we dont care about this trigger source. it's noise. dont tell me about noise!
  }
  else if(1 == int_src)
  {
    flash();
    uint8_t lightning_dist_km = lightning0.AS3935_GetLightningDistKm();
    display.clearDisplay();
    l++;
    display.println("  Lightning!!");
    if (mi==1){
    lsDist=(lightning_dist_km  * 0.62137);
    }
    else{
     lsDist=(lightning_dist_km); 
    }
    display.println((lsDist));
    mapDist = map(lsDist, 0, 25, 0, 83);
    display.println(" Miles");
    display.display();
    flash();
  }
  else if(2 == int_src)
  {
    //display.clearDisplay();
  display.setCursor(72,0);
  display.println("N");
  //  n++;
   display.display();
  delay(1000);
  display.setCursor(72,0);
  display.println(" ");
  }
  else if(3 == int_src)
  {
    display.clearDisplay();
 display.println("Noise level too high");
 display.display();
 delay(500);
  }
//  lightning0.AS3935_PrintAllRegs(); // for debug...
}
}

// this is irq handler for AS3935 interrupts, has to return void and take no arguments
// always make code in interrupt handlers fast and short
void AS3935_ISR()
{
  AS3935_ISR_Trig = 1;
}
//---flash the display routine
void flash(){
  for (int y=1;y<5;y++){
    digitalWrite(6,LOW);
    display.invertDisplay(true);
    delay(100);
    digitalWrite(6,HIGH);
    display.invertDisplay(false);
    delay(100);
  }
}
//--make that cloud go baby!!
void animateCloud(){
    display.drawBitmap(m,15,  cloud, 16, 16, 1);
  display.display();
  if(m>=80){m=0;}
    m++;
  delay(200);
}

