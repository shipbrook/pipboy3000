#include <Adafruit_GFX.h>
#include "Adafruit_HX8357.h"
#include <SPI.h>
#include <SD.h>

#define DEBUG            // Send debugging data to serial port

#define TFT_DC 9
#define TFT_CS 10
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC);

#define SD_CS 4

#define ROT_1A 6
#define ROT_1B 7
#define BTN1 A0
#define BTN2 A1
#define BTN3 A2
#define LED1 A4
#define LED2 A5
#define LED3 A3


//                                 Byte Values           R     G      B
#define GREENBRIGHT   0x07EF // 0000 0111 1110 1111    00000 111111 01111    
#define GREENDIM      0x0408 // 0000 0100 0000 1000    00000 100000 01000    
#define AMBERBRIGHT   0xFCE0 // 1111 1100 1110 0000    11111 100111 00000    
#define AMBERDIM      0x8240 // 1000 0010 0100 0000    10000 010010 00000    
#define BLUEBRIGHT    0x469C // 0100 0110 1001 1100    01000 110100 11100    
#define BLUEDIM       0x2B90 // 0010 1011 1001 0000    00101 011100 10000    
#define WHITEBRIGHT   0xFFFF // 1111 1111 1111 1111    11111 111111 11111    
#define WHITEDIM      0x8410 // 1000 0100 0001 0000    10000 100000 10000    
#define NUMCOLORS     4

#define LAST_SCREEN 4

static const int8_t EncoderStates[] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };
static const unsigned int Colors[][2] = { {GREENDIM,GREENBRIGHT}, {AMBERDIM,AMBERBRIGHT},
                                          {BLUEDIM,BLUEBRIGHT}, {WHITEDIM,WHITEBRIGHT} };
volatile int Encoder1State = 0, ButtonState = 0;
int currentMode = 0;
int currentScreen = 0;
int colorIndex = 0;
int currentTextColor = 1;
int flashlightMode = 0;

void setup(void) {
#ifdef DEBUG
  Serial.begin(9600);
#endif
  delay( 250 );

  pinMode( ROT_1A, INPUT_PULLUP );  // enable pull-ups on input pins for rotary encoder 1
  pinMode( ROT_1B, INPUT_PULLUP );

  pinMode( BTN1, INPUT_PULLUP ); // enable pull-ups on buttons
  pinMode( BTN2, INPUT_PULLUP );
  pinMode( BTN3, INPUT_PULLUP );

  pinMode( LED1, OUTPUT );
  pinMode( LED2, OUTPUT );
  pinMode( LED3, OUTPUT );
  
  tft.begin();                    // Initialize TFT display
  tft.setRotation(1);             // Display in landscape mode
  
  if (!SD.begin(SD_CS)) {
#ifdef DEBUG
    Serial.println(F("SD card init failed!"));
#endif
  }
  doLEDs();
  PaintScreen( 1 );
}

void loop() {
  // Check for presses or releases of any buttons
  //
  int newButtonState = GetButtons();
  if (newButtonState != ButtonState) {
    delay( 100 ); // Since we handle button combinations, wait in case they're not simultaneous
    newButtonState = GetButtons();
    switch (newButtonState) {
      case 4: // STATS    // If one of the buttons is pressed by itself, change modes
      case 2: // ITEMS
      case 1: // DATA
        if (currentMode != 4>>newButtonState) { // Do nothing if we're already in the selected mode
          currentMode = 4>>newButtonState;
          doLEDs();
          flashlightMode = 0;
          PaintScreen( 1 );
        }
        break;
      case 3 : // Center + Right = enter flashlight mode (any button or combo to exit)
        if (!flashlightMode) {
          flashlightOn();
        } else {
          flashlightMode = 0;
          PaintScreen( 1 );
        }
        break;
      case 5 : // Left + Right = return to home screen
        currentMode = currentScreen = 0;
        doLEDs();
        flashlightMode = 0;
        PaintScreen( 1 );
        break;
      case 6 : // Left + Center = change color
        colorIndex = ++colorIndex % NUMCOLORS;
        PaintScreen( flashlightMode );
        flashlightMode = 0;
        break;
    }
    ButtonState = newButtonState;
  }
  
  // Check main encoder knob  
  //
  int saveScreen = currentScreen;
  Encoder1State = ((Encoder1State<<2) | (digitalRead(ROT_1A)<<1) | digitalRead(ROT_1B)) & 0x0f;
  if ((Encoder1State & 3) != (Encoder1State>>2)&3) { // If state has changed since last call
    int Direction = EncoderStates[ Encoder1State ];  // Determine which direction the knob moved
    currentScreen += Direction;                      // Add or subtract from current screen
    if ((currentScreen < 0) || (currentScreen > LAST_SCREEN)) currentScreen -= Direction; // don't move past ends
    if (currentScreen != saveScreen) {// If the screen has changed, display it
      flashlightMode = 0;
      PaintScreen( 1 ); 
    }
    Encoder1State = ((Encoder1State<<2) | (digitalRead(ROT_1A)<<1) | digitalRead(ROT_1B)) & 0x0f; // debounce
  }
}

int GetButtons() {
  return ((digitalRead(BTN1)<<2)  | (digitalRead(BTN2)<<1) | digitalRead(BTN3)) xor 7; // values reversed due to pull-up
}

void doLEDs() {
  analogWrite( LED1, (currentMode == 0) ? 255 : 0 );
  analogWrite( LED2, (currentMode == 1) ? 255 : 0 );
  analogWrite( LED3, (currentMode == 2) ? 255 : 0 );
}

void PaintScreen( int clearFirst ) {
  char filename[13] = "screen__.dat";          // Determine which file to display
  filename[6] = (char) (48 + currentMode);
  filename[7] = (char) (48 + currentScreen);
#ifdef DEBUG
  uint32_t startTime = millis();
#endif
  DisplayHeaders( currentMode, currentScreen, clearFirst ); // Show the headers for this mode and screen
  ScreenFromFile( filename );                               // Read the desired file and display it
#ifdef DEBUG
  Serial.print(filename);
  Serial.print(F(" drawn in "));
  Serial.print(millis() - startTime, DEC);
  Serial.println(" ms");
#endif
}

void ClearScreen() {
  tft.startWrite();
  tft.setAddrWindow( 0, 0, 480, 320 );
  tft.writeColor( 0, 153600 );
  tft.endWrite();
}

void flashlightOn() {
  flashlightMode = 1;
  tft.startWrite();
  tft.setAddrWindow( 0, 0, 480, 320 );
  tft.writeColor( Colors[colorIndex][1], 153600 );
  tft.endWrite();
}

void DisplayHeaders( int mode, int screen, int clearFirst  ) {
  int screenBox[][5][2] = {{{38,59}, {107,103}, {220,58}, {304,56}, {378,70}},
                           {{40,62}, {130, 60}, {230,38}, {310,43}, {390,45}},
                           {{40,73}, {139, 73}, {237,55}, {317,50}, {390,48}}};
  char filename[11] = "frame_.dat";
#ifdef DEBUG
  uint32_t startTime = millis();
#endif
  if (clearFirst) tft.fillScreen(HX8357_BLACK);
#ifdef DEBUG
  Serial.print(F("Screen clear completed in "));
  Serial.print(millis()-startTime, DEC);
  Serial.println(" ms");
#endif
  tft.setTextColor(Colors[colorIndex][currentTextColor = 1]);
  tft.setTextSize(1);
  tft.fillRect( screenBox[mode][screen][0], 292, screenBox[mode][screen][1], 28, Colors[colorIndex][0] );
  tft.drawRect( screenBox[mode][screen][0], 292, screenBox[mode][screen][1], 28, Colors[colorIndex][1] );
  filename[5] = (char) 48 + (char) mode;
  ScreenFromFile( filename );
}

void printAt( int x, int y, char *text, int col ) {
  tft.setCursor( x, y );
  if (currentTextColor != col)
    tft.setTextColor( Colors[colorIndex][currentTextColor = col] );
  tft.print( text );
}

// This is the meat of the sketch. It opens a file on the SD card, reads it in, and draws text and graphics
// based on the data it reads. The first word indicates the function; subsequent words contain the various
// parameters (X and Y positions, width/height, etc.). For strings, it indicates the number of bytes to read
// in.
//
// TODO: optimize so that bytes are used instead of words where appropriate. Might speed things up infinitesimally.
//
void ScreenFromFile( char *filename ) {
  File infile;
  uint32_t count, i;
  int func, x1, y1, x2, y2, x3, y3, width, height, col;
  char s[80];
#ifdef DEBUG
  uint32_t startTime = millis();
#endif

  if (!(infile = SD.open(filename))) {
#ifdef DEBUG
    Serial.print(F("File not found."));
#endif
    return;
  }
  do {
    switch (func = (int)infile.read()) {
      case 1 : // horizontal line
        x1 = readint( infile );
        y1 = readint( infile );
        width = readint( infile );
        col = (int)infile.read();
        tft.drawFastHLine( x1, y1, width, Colors[colorIndex][col] );
        break;
      case 2 : // vertical line
        x1 = readint( infile );
        y1 = readint( infile );
        height = readint( infile );
        col = (int)infile.read();
        tft.drawFastVLine( x1, y1, height, Colors[colorIndex][col] );
        break;
      case 3 : // filled box
        x1 = readint( infile );
        y1 = readint( infile );
        width = readint( infile );
        height = readint( infile );
        col = (int)infile.read();
        tft.fillRect( x1, y1, width, height, Colors[colorIndex][col] );
        break;
      case 4 : // arbitrary line
        x1 = readint( infile );
        y1 = readint( infile );
        x2 = readint( infile );
        y2 = readint( infile );
        col = (int)infile.read();
        tft.drawLine( x1, y1, x2, y2, Colors[colorIndex][col] );
        break;
      case 5 : // triangle
        x1 = readint( infile );
        y1 = readint( infile );
        x2 = readint( infile );
        y2 = readint( infile );
        x3 = readint( infile );
        y3 = readint( infile );
        col = (int)infile.read();
        tft.drawTriangle( x1, y1, x2, y2, x3, y3, Colors[colorIndex][col] );
        break;
      case 6 : // text
        x1 = readint(infile);
        y1 = readint(infile);
        col = (int)infile.read();
        width = (int)infile.read();
        if (width > 0) {
          for (i=0; i < width; i++)
            s[i] = infile.read();
          s[i] = 0;
          printAt( x1, y1, s, col ); 
        }
        break;
      case 7 : // text size
        y1 = (int)infile.read();;
        tft.setTextSize( y1 );
        break;
      case 8 : // unfilled box
        x1 = readint( infile );
        y1 = readint( infile );
        width = readint( infile );
        height = readint( infile );
        col = (int)infile.read();
        tft.drawRect( x1, y1, width, height, Colors[colorIndex][col] );
        break;
      case 9 : // filled triangle
        x1 = readint( infile );
        y1 = readint( infile );
        x2 = readint( infile );
        y2 = readint( infile );
        x3 = readint( infile );
        y3 = readint( infile );
        col = (int)infile.read();
        tft.fillTriangle( x1, y1, x2, y2, x3, y3, Colors[colorIndex][col] );
        break;
    }
  } while (func != 99);
  infile.close();
}

int readint( File file ) {
  return (int)file.read() | ((int)file.read() << 8);
}
