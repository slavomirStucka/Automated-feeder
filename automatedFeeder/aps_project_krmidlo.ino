// pridanie potrebnych kniznic
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
//definovanie makier pre piny a tlacidla
#define SIGNAL_PIN 8
#define IR_RECEIVE_PIN 2
#define IR_BUTTON_100 25
#define IR_BUTTON_PLUS 21
#define IR_BUTTON_MINUS 7
#define IR_BUTTON_PLAY 67
#define IR_BUTTON_EQ 9
#define IR_BUTTON_CH 70
//vytvorenie objektov pre pracu s kniznicami
Servo Myservo;
LiquidCrystal_I2C lcd(0x27,16,2);
//inicializacia pomocnych premennych
int minutes; //minuty odpocitavania
int pom_minutes=0; //nastaveny casovac v minutach
int charge; //davka, 1 = mala, 2 = stredna, 3 = velka 
int setcharge=0; //nastavena davka
int seconds; //sekundy odpocitavania
char timer[16]; //string buffer pre vypis
int command; //ulozeny prikaz tlacidla

//prvotne nastavenie lcd displeja
void lcd_init(){
    lcd.clear();
    lcd.init();
    lcd.backlight();
}

//prvotne nastavenie krmidla
void setup() {
  lcd_init();
  pinMode(2,INPUT); //pin 2 ako vstupny
  Myservo.attach(5); //pripojit servo na pin 5
  Myservo.write(0); //otocit servo na uhol 0 stupnov
  IrReceiver.begin(IR_RECEIVE_PIN); //zacat pracu s IR prijmacom
  pinMode(SIGNAL_PIN, INPUT); //pin SIGNAL_PIN tj 8 ako vstupny
}

//hlavny cyklus krmidla
void loop()
{
  minutes=pom_minutes; //zresetuje casovac
  charge = setcharge; //nastavi nastavenu davku
  lcd.clear();
  //kym nieje nastaveny cas 
  while(pom_minutes==0){
    lcd.setCursor(2,0);
    lcd.print("Nastav cas");
    //ked je dostupny signal
    if(IrReceiver.decode()){
      IrReceiver.resume(); //umozni prijat dalsi signal
      command=IrReceiver.decodedIRData.command; //uloz dekodovany signal
      lcd.setCursor(2,0);
      lcd.print("Nastav cas");
      //ked je stlacene tlacidlo +
      if(command==IR_BUTTON_PLUS){
        minutes = minutes + 15; //pridaj 15 minut
        if(minutes > 180){
          minutes = 15; //ked je cas viac nez 3 hod, zresetuj ho na 15min
        }
        lcd.setCursor(1,1);
        sprintf(timer,"%02d hod %03d min",minutes / 60, minutes % 60); //vypis kolko je momentalne nastavene
        lcd.print(timer);
        delay(400);
        continue;
      }
      //ked je stlacene tlacidlo minus
      else if(command==IR_BUTTON_MINUS){
        minutes = minutes - 15; //uber 15 minut
        if(minutes < 15){
          minutes = 180; //ked je cas menej ako 15 minut, zresetuj ho na 3 hod
        }
        lcd.setCursor(1,1);
        sprintf(timer,"%02d hod %03d min",minutes / 60, minutes % 60);
        lcd.print(timer); //vypis kolko je nastavene
        delay(400);
        continue;
      }
      //ked je stlacene tlacidlo 100+ a nejaky cas bol nastaveny
      else if(command == IR_BUTTON_100 && minutes > 0){
        pom_minutes = minutes; //uloz cas a vyjdi z cyklu
        break;
      }
    }
    //manualne nasypanie tlacidlom PLAY/PAUSE pri nastavovani casu 
    if(command==IR_BUTTON_PLAY){
      nasypanie_manual();
    }
  }

  lcd.clear();
  //kym nieje nastavena davka
  while(setcharge == 0){
    lcd.setCursor(2,0);
    lcd.print("Nastav davku");
    //ked je dostupny signal
    if(IrReceiver.decode()){
      IrReceiver.resume(); //umozni prijat dalsi signal
      command=IrReceiver.decodedIRData.command; //uloz prikaz
      lcd.setCursor(2,0);
      lcd.print("Nastav davku");
      //ak je stlacene +
      if(command==IR_BUTTON_PLUS){
        charge = charge + 1; //pridaj dalsiu davku
        if(charge > 3){
          charge = 1; //ak je davka viac nez 3(velka) zresetuj na 1(mala)
        }
        lcd.setCursor(2,1);
        sprintf(timer,"%01d nasobna",charge);
        lcd.print(timer); //vypis nastavenu davku
        delay(400);
        continue;
      }
      //ak je stlacene -
      else if(command==IR_BUTTON_MINUS){
        charge = charge - 1; //uber davku
        if(charge < 1){
          charge = 3; //ak je davka mensia ako 1(mala) zresetuj na 3(velka)
        }
        lcd.setCursor(2,1);
        sprintf(timer,"%01d nasobna",charge);
        lcd.print(timer); //vypis nastavenu davku
        delay(400);
        continue;
      }
      //ak je stlacene 100+ a bola nastavena davka
      else if(command == IR_BUTTON_100 && charge > 0){
        setcharge = charge; //uloz davku
        break; //vyjdi z cyklu
      }
    }
    //manualne nasypanie pri nastaveni davky 
    if(command==IR_BUTTON_PLAY){
      nasypanie_manual();
    }
  }
  
  seconds=1; //nastav sekundy
  lcd.clear();
  //kym cas nevyprsal
  while(minutes>=0){     
    
    lcd.setCursor(2,0);
    lcd.print("Dalsia davka");
    lcd.setCursor(1,1);
    sprintf(timer,"%01d h %02d m %02d s",minutes / 60, minutes % 60 ,seconds);
    lcd.print(timer); //vypis kolko casu zostava
    delay(1000); //pockaj 1 sekundu
    
    seconds--;                //odpocitanie sekund
    if(seconds==0&&minutes>0){ //odpocitanie minuty a nastavnie na 60 sek
      seconds=60;
      minutes--;
      }
    if(seconds==0&&minutes==0){ //ked cas vyprsal, nasyp
      nasypanie();
      break;
      }
      
    if(IrReceiver.decode()) { //ked je dostupny signal, uloz ho a zresetuj stav prijmaca
        IrReceiver.resume();
        command=IrReceiver.decodedIRData.command;
        //ak je stlacene tlacidlo EQ, zresetuj vsetky casove hodnoty a vyjdi z cyklu
        if(command==IR_BUTTON_EQ){
          pom_minutes=0;
          minutes = 0;
          lcd.clear();
          lcd.setCursor(2,0);
          lcd.print("Nastav cas");
          break;
        }
        //ak je stlacene tlacidlo CH, zresetuj vsetky davkove hodnoty a vyjdi z cyklu
        else if(command == IR_BUTTON_CH){
          setcharge = 0;
          charge = 0;
          lcd.clear();
          lcd.setCursor(2,0);
          lcd.print("Nastav davku");
          break;
        }
        //ak je stlacene tlacidlo PLAY/PAUSE, nasyp
        else if(command==IR_BUTTON_PLAY){
          nasypanie_manual();
          break;
          }
      }
      
    
    Myservo.write(0);
  }

}

//nasype davku, ak dostane signal zo senzora
void nasypanie(){
  bool nasypane=false;
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Cakam na teba");
  lcd.setCursor(2,1);
  lcd.print("moj milacik!");
  //pokial este nebolo nasypane, skusa ci nieje signal  
  while(nasypane==false){   
   if(digitalRead(SIGNAL_PIN)==HIGH) {  
        nasypanie_manual();
        nasypane=true;
    }
  }
}
//praca so servom pri nasypavani, volana priamo pri manualnom nasypani alebo automaticky pri zachyteni signalu zo senzora  
void nasypanie_manual(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Nech sa paci!");
  lcd.setCursor(2,1);
  lcd.print("Dobru chut!");
  Myservo.write(90); //otoc servo na uhol 90 stupnov, co je otvorena poloha
  delay(1000 * charge); //pockaj davka sekund
  Myservo.write(0); //otoc servno naspat do zavretej polohy
  delay(5000); //pockaj 5 sekund aby sa dalo precitat ze bolo nasypane
}