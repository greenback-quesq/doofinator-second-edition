/////////////////// LIBRARIES ///////////////////
/* Music player */
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

/* Time and date */
#include <RTClib.h>
// IR to change song and volume
#include <IRremote.h>
/////////////////////////////////////////////////

const int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;
uint16_t lastCode = 0; // This keeps track of the last code RX'd

/* For SparkFun IR Remote */ 
const uint16_t BUTTON_POWER = 0x629D;
// const uint16_t BUTTON_A = 0x22DD;
// const uint16_t BUTTON_B = 0x02FD;
// const uint16_t BUTTON_C = 0xC23D;
// const uint16_t BUTTON_UP = 0x9867;
// const uint16_t BUTTON_DOWN = 0x38C7;
const uint16_t BUTTON_LEFT = 0x30CF;
const uint16_t BUTTON_RIGHT = 0x7A85;
const uint16_t BUTTON_CIRCLE = 0x18E7;

/////////////////// RTC MODULE ///////////////////
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

RTC_Millis rtc;
 /////////////////////////////////////////////////


/////////////////// VS1053B AUDIO ///////////////////
// define the pins used
// #define CLK 13       // SPI Clock, shared with SD card
// #define MISO 12      // Input data, from VS1053/SD card
// #define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins. 
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
/////////////////////////////////////////////////////

// Variables definition
int count = 0;  // track listing selection
bool play = true;  // start or stop the music
int vol_setting = 0;  // tracking current volume, initialized to three
int randomNumber; 
boolean flag = true;  // necessary for skipping or returning to tracks

void setup() {
  Serial.begin(9600);

  // establish randomSeed
  randomSeed(analogRead(0));
  irrecv.enableIRIn(); // enable IR coms

  /////////////////// TROUBLESHOOT CODE (RTC, VS1053B, uSD) ///////////////////

  // following line sets the RTC to the date & time this sketch was compiled
  rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
    
  // 2. Ensure VS1053B is connected properly
  // Serial.println("Adafruit VS1053 Startup Test");
  
  if (! musicPlayer.begin()) { // initialise the music player
     // Serial.println("Couldn't find VS1053, do you have the right pins defined?");
     while (1);
  }
  // Serial.println("VS1053B found");

  // 3. Ensure uSD card is connected properly
  if (!SD.begin(CARDCS)) {
    // Serial.println("SD failed, or not present");
    while (1);  // don't do anything more
  }

  // printDirectory(SD.open("/"), 0); // List files on SD for debugging

  /////////////////////////////////////////////////////////
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(vol_setting,vol_setting);
  // Serial.println("Playing DOOF Radio!");
  
  // Opening jingle based on time and day
  evilJingle();  
}

void count_fix_pos(){
  if (count == 21) {
    count = -1;
  }
  count++; // adjust track to track thereafter for next play
}

void count_fix_neg(){
  count--; // adjust track to track thereafter for next play
  count--; // account for automatic count_fix_pos forward
  if (count < 0){
    count = 21 + count;
  }
}

void loop() {
  // Regularly update the time using the RTC
  // DateTime now = rtc.now();
  
  // Begin playing on Doof Radio
  // Serial.print("current track #: ");
  // Serial.println(count);
  doofRadio(count);
  count_fix_pos();

  if (flag == false){
    delay(100);
    flag = true;
  }
  
  // OLD: while ((musicPlayer.playingMusic or musicPlayer.paused()) and flag) {
  while (flag) {
    if (irrecv.decode(&results)){
      /* read the RX'd IR into a 16-bit variable: */
      uint16_t resultCode = (results.value & 0xFFFF);
      
      /* The remote will continue to spit out 0xFFFFFFFF if a
      button is held down. If we get 0xFFFFFFF, let's just
      assume the previously pressed button is being held down */
      if (resultCode == 0xFFFF)
        resultCode = lastCode;
      else
        lastCode = resultCode;
  
      switch (resultCode){
        // play previous song
        case BUTTON_LEFT:
          // Serial.println("Previous Song");
          count_fix_neg();
          musicPlayer.stopPlaying();
          flag = false;
          break;
        // play next song
        case BUTTON_RIGHT:
          // Serial.println("Next Song");
          musicPlayer.stopPlaying();
          flag = false;
          break;
        case BUTTON_CIRCLE:
          if (! musicPlayer.paused()) {
            // Serial.println("Paused");
            musicPlayer.pausePlaying(true);
          }
          else {
            // Serial.println("Resumed");
            musicPlayer.pausePlaying(false);
          }
          break;
          /*
        case BUTTON_UP:
          // Serial.println("Increasing Volume");
          vol_setting++;
          musicPlayer.setVolume(vol_setting,vol_setting);
          break;
        case BUTTON_DOWN:
          // Serial.println("Increasing Volume");
          vol_setting--;
          musicPlayer.setVolume(vol_setting,vol_setting);
          break;
          */
      }

      delay(100);      
      irrecv.resume(); 
    }
  }
}

void doofRadio(int track){
  switch (track) {
  case 0:
    musicPlayer.startPlayingFile("inator.mp3");
    break;
  case 1:
    musicPlayer.startPlayingFile("morning.mp3");
    break;
  case 2:
    musicPlayer.startPlayingFile("evil.mp3");
    break;
  case 3:
    musicPlayer.startPlayingFile("christ.mp3");
    break;
  case 4:
    musicPlayer.startPlayingFile("doofania.mp3");
    break;
  case 5:
    musicPlayer.startPlayingFile("crisis.mp3");
    // ?
    break;
  case 6:
    musicPlayer.startPlayingFile("control.mp3");
    break;
  case 7:
    musicPlayer.startPlayingFile("busted.mp3");
        // song doesn't work
    break;
  case 8:
    musicPlayer.startPlayingFile("brother.mp3");
        // song doesn't work
    break;
  case 9:
    musicPlayer.startPlayingFile("evillove.mp3");
    break;
  case 10:
    musicPlayer.startPlayingFile("friend.mp3");
    break;
  case 11:
    musicPlayer.startPlayingFile("getalong.mp3");
    // ?
    break;
  case 12:
    musicPlayer.startPlayingFile("herheart.mp3");
    break;
  case 13:
    musicPlayer.startPlayingFile("impress.mp3");
    break;
  case 14:
    musicPlayer.startPlayingFile("dad.mp3");
    break;
  case 15:
    musicPlayer.startPlayingFile("Lies.mp3");
    break;
  case 16:
    musicPlayer.startPlayingFile("charmed.mp3");
    // ?
    break;
  case 17:
    musicPlayer.startPlayingFile("morning.mp3");
    break;
  case 18:
    musicPlayer.startPlayingFile("nemesis.mp3");
    break;
  case 19:
    musicPlayer.startPlayingFile("talk.mp3");
    break;
  case 20:
    musicPlayer.startPlayingFile("yodel.mp3");
    break;
  }
}

void evilJingle(){
  DateTime now = rtc.now();
  /*
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
   */

  
  if (now.hour() > 10 || now.hour() < 5 ) { // military time
    // after-hours
    musicPlayer.playFullFile("3.mp3");    
  } else if ((now.month() == 12 && now.day() == 31) || (now.month() == 1 && now.day() == 1)) {
    // new-years
    musicPlayer.playFullFile("8.mp3");    
  } else if (now.month() == 10 && now.day() == 31) {
    // halloween
    musicPlayer.playFullFile("9.mp3");   
  } else { // no RTC conditions met, randomize as appropriate
    randomNumber = random(0,11);
    // Serial.print("no RTC condition met, RANDOM: ");
    // Serial.println(randomNumber);
    switch (randomNumber) {
    case 0:
      musicPlayer.playFullFile("0.mp3");
      break;
    case 1:
      musicPlayer.playFullFile("1.mp3");
      break;
    case 2:
      musicPlayer.playFullFile("2.mp3");
      break;
    case 3:
      musicPlayer.playFullFile("4.mp3");
      break;
    case 4:
      musicPlayer.playFullFile("6.mp3");
      break;
    case 5:
      musicPlayer.playFullFile("7.mp3");
      break;
    case 6:
      musicPlayer.playFullFile("10.mp3");
      break;
    case 7:
      musicPlayer.playFullFile("11.mp3");
      break;
    case 8:
      musicPlayer.playFullFile("12.mp3");
      break;
    case 9:
      musicPlayer.playFullFile("13.mp3");
      break;
    case 10:
      musicPlayer.playFullFile("3.mp3");
      break;
    } 
  }

  delay(1000);
}
