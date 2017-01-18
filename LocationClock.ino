// This #include statement was automatically added by the Particle IDE.
#include "lib1.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>

// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

// IMPORTANT: Set Pin, number of types and NeoPixel model
#define NEOPIXEL_PIN D2
#define NEOPIXEL_COUNT 50
#define NEOPIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEOPIXEL_TYPE);

struct PersonConfig{
    int pen, red, blue, green;
    String current_location;
};

std::map< String, PersonConfig> user_map = {
    {"default", {0, 0, 0, 0, "Home"}},
    {"Katie", {0, 75, 255, 0, "Home"}},
    {"Matt", {4, 50, 0, 255, "Home"}},
    {"Manan", {2, 255, 0, 0, "Home"}},
    {"Robert", {3, 0, 255, 0, "Home"}},
    {"Abby", {1, 0, 255, 255, "Home"}}
};

std::map<String, int> place_map = {
    {"Home", 0},
    {"SO", 7},
    {"Trivia", 5},
    {"Work",4},
    {"Climbing", 3},
    {"Peril", 2},
    {"Family", 1},
    {"Travelling", 6},
};

// These keep track of a person's last and current location
std::map<String, String> last_location_map;
std::map<String, String> curr_location_map;

//sample function input: "entered,Katie,Home"
int UpdateClock(String command){
    Serial.print("Updating Clock" + command);

    int comma_index = command.indexOf(',');
    int second_comma_index = command.indexOf(',', comma_index+1);

    String input_arrival = command.substring(0, comma_index); // Arrival is either "entered" or "exited"
    String input_person = command.substring(comma_index+1, second_comma_index);
    String input_place = command.substring(second_comma_index+1); // To the end of the string

    PersonConfig person_config = user_map[input_person];
    last_location_map[input_person] = curr_location_map[input_person];

    int pln=2;
    // If "exited", then change person's location to Travelling.
    if(input_arrival == "exited"){
        pln = 6;
        curr_location_map[input_person] = "Travelling";
    }
    else {
        pln = place_map[input_place];
        curr_location_map[input_person] = input_place;
    }

    for(int i=person_config.pen; i<50; i=i+6){
        strip.setPixelColor(i, strip.Color(0, 0, 0));
        strip.show();
    }

    strip.setPixelColor(person_config.pen+pln*6,
                        strip.Color(person_config.green, person_config.red, person_config.blue));
    strip.show();
    return 1;
}

void setup()
{
    Particle.function("UpdateClock", UpdateClock);
    Particle.function("ExitPeril", leavePeril);
    Serial.begin(9600);
    Serial.print("Setting Up Serial");

    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    initLeds();
    initLocations(); // Initialize all people to mortal peril
}

void initLeds(){
    for (int j=0; j<50; j++){
      strip.setPixelColor(j, strip.Color(255, 255, 255));
      strip.show();
      delay(200);
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      strip.show();
    }
}

void loop() {
    // Connect the Particle to the cloud (so it can publish events)
    if (Particle.connected() == false) {
        Particle.connect();
    }
}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}