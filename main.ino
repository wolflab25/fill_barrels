#include "timer-api.h"
#include "U8glib.h"
#include "images.h"
#include <RotaryEncoder.h>

#define MAINSCREEN 0
#define FILLBARRELSSCREEN 1
#define SETTINGSCREEN 2
#define EMERGENCYSTOPSCREEN 3
#define STATUSSCREEN 4
#define JOBBEGIN 1
#define JOBIDLE 0

#define ENC1 A2
#define ENC2 A3
#define ENCBTN A1
#define STOPBTN A0
#define MENU_ITEMS 5
#define COLDLEVELSENSOR 9
#define HOTLEVELSENSOR 7
#define COLDVALVE 5
#define HOTVALVE 6
#define LEAKSENSOR A7
#define LEAKLEVEL 1000
#define EMERGENCYTIME 120
#define JOBFILLALL 3
#define JOBFILLCOLD 1
#define JOBFILLHOT 2

int STARTPOS = 0;
int FINALPOS = 4;
int newPos = 0;
int stopbtnstate = 0;
int encbtnstate = 0;
int navi = MAINSCREEN;
volatile int jobstate = JOBIDLE;
int jobtype = 0;
int current_menu = 0;
volatile int emergencytimer = EMERGENCYTIME;






U8GLIB_ST7920_128X64_1X u8g(13, 11, 10);  // SPI Com: SCK = en = 18, MOSI = rw = 16, CS = di = 17
RotaryEncoder encoder(ENC1, ENC2);

   char* concat(char *s1, char *s2) {

        size_t len1 = strlen(s1);
        size_t len2 = strlen(s2);                      

        char *result = malloc(len1 + len2 + 1);

        if (!result) {
            fprintf(stderr, "malloc() failed: insufficient memory!\n");
            return NULL;
        }

        memcpy(result, s1, len1);
        memcpy(result + len1, s2, len2 + 1);    

        return result;
    }

void draw(void) {
   u8g.drawBitmapP( 0, 0, 16, 64, screens[newPos]);
}

void drawstatus(void) {
  uint8_t i, h;
  u8g_uint_t w, d;
  int cs, hs, ls;
 char *menu_strings[5] = { "ДАТЧИК УРОВНЯ ХОЛ:", "ДАТЧИК УРОВНЯ ГОР:", "КРАН ХОЛ:", "КРАН ГОР:", "ДАТЧИК ПРОТЕЧКИ:" };
  cs = digitalRead(COLDLEVELSENSOR);
  hs = digitalRead(HOTLEVELSENSOR);
  ls = analogRead(LEAKSENSOR);
  
  u8g.setFont(rus6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  
  h = u8g.getFontAscent()-u8g.getFontDescent();
  w = u8g.getWidth();
  u8g.setDefaultForegroundColor();
  
   if (cs==0){
    char *s = concat(menu_strings[0], "ВКЛ");
    u8g.drawStr(0, 0*h, s);
    free(s);
    }
   if (cs==1){
    char *s = concat(menu_strings[0], "ВЫКЛ");
    u8g.drawStr(0, 0*h, s);
    free(s);
    }
if (hs==0){
    char *s = concat(menu_strings[1], "ВКЛ");
    u8g.drawStr(0, 1*h, s);
    free(s);
    }
   if (hs==1){
    char *s = concat(menu_strings[1], "ВЫКЛ");
    u8g.drawStr(0, 1*h, s);
    free(s);
    }
    char buf [4];
    sprintf (buf, "%03i", ls);
    char *s = concat(menu_strings[4], buf);
    u8g.drawStr(0, 2*h, s);
    free(s);
   
  
}

void drawprogram(void) {
  uint8_t i, h;
  u8g_uint_t w, d;
  char *menu_strings[5] = { "НАБИРАЕТСЯ БАК С", "ХОЛОДНОЙ ВОДОЙ", "ГОРЯЧЕЙ ВОДОЙ", "ДО ЗАКРЫТИЯ КРАНА", "ОСТАЛОСЬ " };
  u8g.setFont(rus6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  h = u8g.getFontAscent()-u8g.getFontDescent();
  u8g.setDefaultForegroundColor();
  u8g.drawStr(0, 0*h, menu_strings[0]);
  if (bitRead(jobtype,0)==1){
   u8g.drawStr(0, 1*h, menu_strings[1]);   
  }else{
   u8g.drawStr(0, 1*h, menu_strings[2]);  
    }
  u8g.drawStr(0, 2*h, menu_strings[3]);
  char buf [4];
    sprintf (buf, "%03i", emergencytimer);
    char *s = concat(menu_strings[4], buf);
    char *ss = concat(s, " СЕК");
    u8g.drawStr(0, 3*h, ss);
    free(s);free(ss);
}

void setup()
{
  Serial.begin(57600);
  //Serial.println("SimplePollRotator example for the RotaryEncoder library.");
  timer_init_ISR_1Hz(TIMER_DEFAULT);
  // You may have to modify the next 2 lines if using other pins than A2 and A3
  PCICR |= (1 << PCIE1);    // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
  PCMSK1 |= (1 << PCINT8) | (1 << PCINT9)|(1 << PCINT10) | (1 << PCINT11)| (1 << PCINT12);  // This enables the interrupt for pin 2 and 3 of Port C.
   pinMode(ENCBTN, INPUT_PULLUP);
   pinMode(STOPBTN, INPUT_PULLUP); 
   pinMode(COLDLEVELSENSOR, INPUT_PULLUP);   
   pinMode(HOTLEVELSENSOR, INPUT_PULLUP);   
   pinMode(LEAKSENSOR, INPUT_PULLUP);   
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
   
  // rebuild the picture after some delay
  delay(1000);

} // setup()



ISR(PCINT1_vect) {
  encoder.tick(); // just call tick() to check the state.
  stopbtnstate=digitalRead(STOPBTN);
  encbtnstate=digitalRead(ENCBTN);
}

void timer_handle_interrupts(int timer) {
    if (jobstate=JOBBEGIN){
    Serial.println("goodbye from timer");
    emergencytimer--;
    }
}


void loop(void) {
  // picture loop
 static int pos = 0;
if (navi==STATUSSCREEN){
  
  if (stopbtnstate==0){
      tone(8, 300, 5);
      navi=MAINSCREEN;
   }   
 }
if (navi==FILLBARRELSSCREEN){
  
  if (stopbtnstate==0){
      tone(8, 300, 5);
      jobstate=JOBIDLE;
      emergencytimer=EMERGENCYTIME;
      navi=MAINSCREEN;
   }   
 }
 
 if (navi==MAINSCREEN){
  //encoder.tick(); // just call tick() to check the state.
  newPos = encoder.getPosition();

  if (encbtnstate==0){
    if (newPos==4){
      tone(8, 300, 5);
      navi=STATUSSCREEN;
    }
     
      if (newPos==0){
         tone(8, 300, 5);
        emergencytimer = EMERGENCYTIME; 
        jobtype=JOBFILLALL;
        jobstate=JOBBEGIN;
        navi=FILLBARRELSSCREEN;
        }
      if (newPos==1){
        emergencytimer = EMERGENCYTIME;
         tone(8, 300, 5);
         jobstate=JOBBEGIN;
        navi=FILLBARRELSSCREEN;       
        jobtype=JOBFILLCOLD;}
      if (newPos==2){
        emergencytimer = EMERGENCYTIME;
         tone(8, 300, 5);
        jobtype=JOBFILLHOT;
         jobstate=JOBBEGIN;
        navi=FILLBARRELSSCREEN;         
        }
  }
  if (pos != newPos) {
    tone(8, 300, 5);
    //delay(100);
    //noTone(8);
    if (newPos<STARTPOS){
     encoder.setPosition(FINALPOS); 
     newPos = FINALPOS;
     }
    if (newPos>FINALPOS){
     encoder.setPosition(STARTPOS); 
     newPos= STARTPOS;
     }
    pos = newPos;
    current_menu= newPos;
    Serial.print(newPos);
    Serial.println();    
    // Just to show, that long lasting procedures don't break the rotary encoder:
    // When newPos is 66 the ouput will freeze, but the turned positions will be recognized even when not polled.
    // The interrupt still works.
    // The output is correct 6.6 seconds later.
  } // if

 }
  u8g.firstPage();  
      do {
        if (navi==MAINSCREEN){draw();}
        if (navi==STATUSSCREEN){drawstatus();}
        if (navi==FILLBARRELSSCREEN){drawprogram();}        
      } while( u8g.nextPage() );

}
