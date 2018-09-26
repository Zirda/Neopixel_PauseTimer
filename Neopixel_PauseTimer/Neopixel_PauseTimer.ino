#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);


volatile unsigned int time_counter = 0;
int min_counter = 0;
int led_counter = 0;
int pause_led = 0;
int pause_led_prev = 15;
int time_modifier = 4;
uint32_t led_color = strip.Color(0, 255, 0);
bool pause = false;
bool light_led = false;
bool pause_flag = false;


void setupTimer1(){
   //Aktivera CTC, 64 Prescaler, Interrupt)
   noInterrupts();
   TCCR1A = 0;
   TCCR1B = 0;
   TCNT1 = 0;
   
   TCCR1A |= (1<<WGM12);
   TCCR1B |= (1<<CS11)|(1<<CS10);
   TIMSK1 |= (1<<OCIE1A);
   //OCR1A = 31249;
   //OCR1AH = B01111010;
   //OCR1AL = B00010001;
   //OCR1A = (B01111010 * 256) + B00010001;
    OCR1A = 6249;
   interrupts();
}


//Ändra färgen på slinga 2 och 3. För lika
//Tömmer inte tidigare leds när rasten är klar

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(5);    //Låg brightness. 15 ok , 10 inte ok? Lägg till resistor vid input?
  setupTimer1();
  sei();
}

ISR(TIMER1_COMPA_vect){
  time_counter += 5;
  if (!pause){  //Not on break 0-54 min
    if ((time_counter%(205*time_modifier)) == 0){   //every 3.75 minute = 225, Testar 206 för att använda alla leds för första 55 min och inte hoppa sista två. 
          light_led = true;
      }
  }
  if (pause){ //Break Start! 55-59 min
    if ((time_counter%(15)) == 0){  //each 3.75 second
          light_led = true;
    }
    //Börja countdown per min. 1 färg per minut kvar. 5 loopar med update 1 led var 3.75 sek. Börja lysa upp en led vid 3s -> 3.75s?
  }
  if((time_counter%(60*time_modifier)) == 0 && time_counter != 0){ 
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    min_counter++;
  }
  if(pause_flag){
    pause_alarm();
  }
}

void loop() {
  if (min_counter >= 60){
      reset_hour();
  }
  if (led_counter >= 16){
    led_counter = 0;
  }
  if (light_led){
        next_led(led_counter, led_color); //Lights upp the next hour based LED
        led_counter++;
        light_led = false;
  }

  if (min_counter >= 55){
    if (min_counter == 55){
      led_color =  strip.Color(0, 255, 230);
    }
    else if (min_counter == 56){
      led_color =  strip.Color(0, 25, 255);
    }
    else if (min_counter == 57){
      led_color =  strip.Color(100, 0, 255);
    }
    else if (min_counter == 58){
      led_color =  strip.Color(230, 0, 150);
    }
    else{
      led_color =  strip.Color(255, 90, 0);
    }
  }
  
  if ((time_counter > (3355*time_modifier)) && (time_counter < (3360*time_modifier))){   //54 min och 55 sekunder in. 5 sekunders larm!
    pause_flag = true;
  }
  else {
    pause_flag = false;
  }
  if(!pause && time_counter > 3360*time_modifier){
      pause = true;
      led_counter = 0;
    }
}


void reset_hour(){
    time_counter = 0;
    min_counter = 0;
    led_counter = 0;
    pause = false;
    led_color =  strip.Color(0, 255, 0);
    for (int i=0; i <= 16; i++){
      strip.setPixelColor(i, strip.Color(0,0,0));
    }
    strip.show();
}

void next_led(int led_counter, uint32_t led_color){
  strip.setPixelColor(led_counter,led_color);
  strip.show();
}

void pause_alarm(){
  if (pause_led >= 16){
    pause_led = 0;
  }
  strip.setPixelColor(pause_led, strip.Color(255, 0, 0));
  strip.setPixelColor(pause_led_prev, strip.Color(0, 0, 0));
  strip.show();
  pause_led_prev = pause_led;
  pause_led++;
}
