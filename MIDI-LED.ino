#include <FastLED.h>
#include <MIDI.h>

#define LED_PIN     5
#define NUM_LEDS    30
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
// LED Mapping modes
#define OFFSET 0
#define SUM 1
#define FLOOR_SUM 2


int mode = FLOOR_SUM;


CRGB leds[NUM_LEDS];
int midiNotes[128];

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
}


void loop()
{
  checkMIDI();
  updateLeds();
}

void updateLeds() {
  int offset = 0;
  int notesPerLed = 1;

  switch(mode) {
    case OFFSET:
      offset = 60 - floor(NUM_LEDS / 2) - 21;
      break;
    case SUM:
      notesPerLed = ceil(88.0 / NUM_LEDS);
      break;
    case FLOOR_SUM:
      notesPerLed = floor(88.0 / NUM_LEDS);
      offset = 60 - floor((NUM_LEDS * notesPerLed) / 2) - 21;
      break;
  }

  for(int i = 0; i < NUM_LEDS; i++) {
    int j = 0;
    int ledSum = 0;
    while(j < notesPerLed && (j + i * notesPerLed + 21) < 109) {
      ledSum += midiNotes[j + i * notesPerLed + 21 + offset];
      j++;
    }
    if(ledSum == 0) {
      leds[i] = NULL;  
    } else {
      leds[i] = getColor(ledSum);  
    }
  }
  
  FastLED.show();
}

void checkMIDI(){
  while(!MIDI.read()) {} // Wait for next MIDI event
  do {
    switch(MIDI.getType())      // Get the type of the message we caught
    {
        case midi::NoteOn:
            midiNotes[MIDI.getData1()] = MIDI.getData2();
            break;
        case midi::NoteOff:
            midiNotes[MIDI.getData1()] = 0;
            break;
        default:
            break;
    }
  } while(MIDI.read());
}

CHSV getColor(int velocity) {
  if(velocity > 127) velocity = 127;
  float weight = velocity / 127.0;
  int hue;
  hue = int((1-weight) * 160 + weight * 64);
  return CHSV(hue, 255, 255);
}
