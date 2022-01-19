/*
MorseKeyer by AD7U
*/
#define MAGIC_NUMBER 8
#define USE_DIGIKEYBOARD_FOR_DEBUG_PRINTING 0
#define USE_SERIAL 1
#define pin_dit 14
#define pin_dah 13 // D1 mini pin D8
#define pin_speed_pot A0 //A1=>pin P2, A3=>pin P3 on Digispark ATTiny85
#define REVERSE_SPEED_MAP 0
#define pin_speaker 12 //D1 Mini 12=>D6
#define DEBUG 0

#define DISABLE_SPEAKER 0
#define DISABLE_SPEED_POT 0

#if USE_DIGIKEYBOARD_FOR_DEBUG_PRINTING
    #include "DigiKeyboard.h"
#endif

// speed of Morse code in milliseconds - higher number means slower typing
int current_tone_length = 75; //will be populated from speed_pot

unsigned long writeTime_now;

int frequency = 750;
int speed = 2000;
int sound_mult = 0; //0=> no sound
int last_dit_dah = -1; //stores 0 for dit or 1 for dah, used to handle if both dit and dah are depressed

//Use ints to activate special commands after x number of presses
int dits_pressed = 0;
int dahs_pressed = 0;

bool speed_pot_controls_speed = true;
String history = "";

void setup()
{    
    #if USE_SERIAL == 1
        Serial.begin(9600);
    #endif
    
    pinMode(pin_dit, INPUT_PULLUP);
    //digitalWrite(pin_dit, HIGH);         // internal pull-up enabled
    
    pinMode(pin_dah, INPUT_PULLUP);
    //pinMode(pin_dah, INPUT);
    //digitalWrite(pin_dah, HIGH);         // internal pull-up enabled

    #if DISABLE_SPEAKER == 0
    pinMode(pin_speaker, OUTPUT);
    #endif

    morseCode(75, "-.-. ...");
}


//TODO: implement Iambic A/B/C/etc



void morseCode(int speed, String dit_dah_array)
{
    int mult;
    
    for (int i =0; i < dit_dah_array.length(); i++)
    {
        mult = 1;
        
        if (dit_dah_array[i] == '-' || dit_dah_array[i] == '_')
        {
            mult = 3;
        }
        else if (dit_dah_array[i] == ' '){
            if (i > 0 && i < (dit_dah_array.length()-1))
                mult = 7;
            else if (i > 0 && dit_dah_array[i-1] == ' ')
                continue; //skip another blank
            else
                mult = 3;
        }
    
        tone(pin_speaker, frequency);

        Serial.println(mult);
        
        delay(speed * mult);
        noTone(pin_speaker);                
        delay(speed);
    }
}
  
void loop()
{
    //sound_mult = 0; //reset // TODO: Identify why this causes dah to not stick.... it doesn't
    
    Serial.println(sound_mult);
    
    #if USE_DIGIKEYBOARD_FOR_DEBUG_PRINTING && DEBUG
    DigiKeyboard.print("millis() = ");
    DigiKeyboard.println(millis());
    #endif 
    
    #if USE_SERIAL && DEBUG
    Serial.print("millis() = ");
    Serial.println(millis());
    #endif

    
    //read speed pot
    #if DISABLE_SPEED_POT == 0
    if (REVERSE_SPEED_MAP == 1)
    {
        if (speed_pot_controls_speed)
            speed = map(analogRead(pin_speed_pot), 1023, 0, 10, 500);
        else
            frequency = map(analogRead(pin_speed_pot), 0, 1023, 1200, 300);
    }
    else
    {
        if (speed_pot_controls_speed)
            speed = map(analogRead(pin_speed_pot), 0, 1023, 10, 500);
        else
            frequency = map(analogRead(pin_speed_pot), 0, 1023, 1200, 300);
    }

    #if USE_DIGIKEYBOARD_FOR_DEBUG_PRINTING
    DigiKeyboard.println(speed);
    #endif 
    
    #if USE_SERIAL && DEBUG
    Serial.print("speed = ");
    Serial.println(speed);
    #endif
    #endif

    //detect dit or dah depressed
    if(digitalRead(pin_dit) == LOW)
    {
        sound_mult = 1;
        dits_pressed += 1;
        
        #if USE_DIGIKEYBOARD_FOR_DEBUG_PRINTING
            DigiKeyboard.println("dit depressed");
        #endif
        
        #if USE_SERIAL
            Serial.println("dit depressed");
        #endif
    }
    else
        dits_pressed = 0; //reset

        
    if(digitalRead(pin_dah) == LOW)
    {
        sound_mult = 3;
        dahs_pressed += 1;
        
        #if USE_DIGIKEYBOARD_FOR_DEBUG_PRINTING
            DigiKeyboard.println("dah depressed");
        #endif
        
        #if USE_SERIAL
            Serial.println("dah depressed");
        #endif
    }
    else
        dahs_pressed = 0; //reset

    /*
        dits must be 1/3 of a dah with a total of 4
        
    */

    //Serial.println(sound_mult);
    
    //tone for specific time
    #if DISABLE_SPEAKER == 0
    if (sound_mult > 0)
    {
        tone(pin_speaker, frequency); //sounds tone
        
        //sleep for specific time while sound is generated

        if (dahs_pressed > 0 && dits_pressed > 0)
        {
            //both pressed and last was dit
            if (last_dit_dah == 0)
            {
                sound_mult = 3; //dah
                last_dit_dah = 1;
            }
            else //if (last_dit_dah == 1)
            {
                sound_mult = 1; //dit
                last_dit_dah = 0;            
            }
        }
        else
            last_dit_dah = -1; //reset

        history += sound_mult == 3 ? "-" : ".";
        
        delay(speed * sound_mult); //sound tone
        
        noTone(pin_speaker);
        
        delay(speed);

        //toggle speed/freq after 8 digits
        if (dits_pressed >= MAGIC_NUMBER and dahs_pressed == 0)
        {
            dits_pressed = 0;
            speed_pot_controls_speed = !speed_pot_controls_speed;

            #if DEBUG == 0
            Serial.println("Switching speed/freq");
            #endif
            
            delay(1000);
            int speed2 = 75;
            
            if (speed_pot_controls_speed){
                //dit
                tone(pin_speaker, frequency);
                delay(speed2);
                noTone(pin_speaker);                
                delay(speed2);

                //dit
                tone(pin_speaker, frequency);
                delay(speed2);
                noTone(pin_speaker);                
                delay(speed2);

                //dit
                tone(pin_speaker, frequency);
                delay(speed2);
                noTone(pin_speaker);                
                delay(speed2);
            }else {   
                morseCode(speed2, "..-.");
            }                
            
            delay(2000);
        }
    }
    else {
        sound_mult--;
            
        //Serial.println(sound_mult);
            
        if (sound_mult >= 0)
        {
            history += " "; // add a space
            sound_mult = 0;

            Serial.print("history: ");
            Serial.println(history);
        }
    }
    #endif
    

}
