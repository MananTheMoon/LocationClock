// This #include statement was automatically added by the Particle IDE.
#include "lib1.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>
#include <random>

// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

// IMPORTANT: Set Pin, number of types and NeoPixel model
#define NEOPIXEL_PIN D2
#define NEOPIXEL_COUNT 55
#define NEOPIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEOPIXEL_TYPE);

struct PersonConfig{
    int pen, red, blue, green;
    String current_location;
};

std::map< String, PersonConfig> user_map = {
    {"Katie", {0, 40, 130, 0, "Home"}},
    {"Matt", {3, 0, 0, 100, "Home"}},
    {"Manan", {5, 100, 0, 0, "Home"}},
    {"Robert", {1, 0, 100, 0, "Home"}},
    {"Abby", {2, 0, 80, 100, "Home"}},
    {"Josh", {4, 100, 0, 50, "Home"}}
};

std::map<String, int> place_map = {
    {"Home", 0},
    {"SO", 7},
    {"Trivia", 5},
    {"Work",4},
    {"Grocery", 3},
    {"Peril", 2},
    {"Family", 1},
    {"Travelling", 6},
};

// These keep track of a person's last and current location
std::map<String, String> last_location_map;
std::map<String, String> curr_location_map;

PersonConfig getRandomPerson(int numberOfPeople) {
    String player_order[6] = {"Robert", "Manan", "Abby", "Matt", "Josh", "Katie"};

    if(numberOfPeople > sizeof(player_order) || numberOfPeople < 1) {
        numberOfPeople = sizeof(player_order);
    }

    int rnd_index = rand() % numberOfPeople;
    String rnd_person = player_order[rnd_index];

    return user_map[rnd_person];
}

//sample function input: "entered,Katie,Home"
int UpdateClock(String command){
    Serial.print("Updating Clock" + command);

    int comma_index = command.indexOf(',');
    int second_comma_index = command.indexOf(',', comma_index+1);

    String input_arrival = command.substring(0, comma_index); // Arrival is either "entered" or "exited"
    String input_person = command.substring(comma_index+1, second_comma_index);
    String input_place = command.substring(second_comma_index+1); // To the end of the string

    // If input person not found
    if(user_map.find(input_person) == user_map.end()) {
        return 0;
    }

    PersonConfig person_config = user_map[input_person];
    flashLeds(person_config.green, person_config.red, person_config.blue, 0, 500, 0);
    updateClockHelper(input_arrival, input_person, input_place);
    returnPeopleToCurrentLocations();

    Particle.publish("ClockUpdated");
    Particle.publish("ClockUpdateData", command);
    return 1;
}

void updateClockHelper(String input_arrival, String input_person, String input_place) {
    // If input person not found
    if(user_map.find(input_person) == user_map.end()) {
        return;
    }

    PersonConfig person_config = user_map[input_person];
    last_location_map[input_person] = curr_location_map[input_person];

    int pln = 2;
    // If "exited", then change person's location to Travelling.
    if(input_arrival == "exited"){
        pln = 6;
        curr_location_map[input_person] = "Travelling";
    }
    else {
        pln = place_map[input_place];
        curr_location_map[input_person] = input_place;
    }

    for(int i = person_config.pen; i < NEOPIXEL_COUNT; i = i + user_map.size() + 1){
        strip.setPixelColor(i, strip.Color(0, 0, 0));
        strip.show();
    }

    strip.setPixelColor(person_config.pen + pln * (user_map.size() + 1),
                        strip.Color(person_config.green, person_config.red, person_config.blue));
    strip.show();
}

//sample input: "Katie"
int leavePeril(String name){
    if(curr_location_map[name] != "Peril") {
        return 1;
    }
    String last_location = "Home"; // Default value if there is no last location for a person
    if(last_location_map.count(name) && curr_location_map[name] == "Peril"){
        String last_location = last_location_map[name];
    }
    String command = "entered," + name + "," + last_location;
    UpdateClock(command);
    return 1;
}

// Can call with or without a command
int pickAColor(String command) {
    int numberOfPeople;
    if (command.length() == 0 || command.length() > 1 || command == "0") {
        numberOfPeople = user_map.size();
    } else {
        numberOfPeople = atoi(command);
    }
    PersonConfig rnd_person = getRandomPerson(numberOfPeople);
    strobeLeds(rnd_person.green, rnd_person.red, rnd_person.blue, 2, 1500, 1500);
    returnPeopleToCurrentLocations();
    return 1;
}

void returnPeopleToCurrentLocations() {
    for (auto const& it : user_map) {
        String name = it.first;
        updateClockHelper("entered", name, curr_location_map[name]);
    }
}

void setup()
{
    Particle.function("UpdateClock", UpdateClock);
    Particle.function("ExitPeril", leavePeril);
    Particle.function("PickColor", pickAColor);
    Serial.begin(9600);
    Serial.print("Setting Up Serial");

    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    initLeds();
    initLocations(); // Initialize all people to mortal peril
}

// Set everyone to mortal peril after initializing
void initLocations() {
    updateClockHelper("entered", "Katie", "Peril");
    updateClockHelper("entered", "Matt", "Peril");
    updateClockHelper("entered", "Manan", "Peril");
    updateClockHelper("entered", "Robert", "Peril");
    updateClockHelper("entered", "Abby", "Peril");
    updateClockHelper("entered", "Josh", "Peril");
}

void flashLeds(int green, int red, int blue, int times, int flash_on_delay, int flash_off_delay) {
    for (int j = 0; j < NEOPIXEL_COUNT; j++) {
        strip.setPixelColor(j, strip.Color(green, red, blue));
    }
    strip.show();
    delay(flash_on_delay);

    for (int j = 0; j < NEOPIXEL_COUNT; j++) {
        strip.setPixelColor(j, strip.Color(0,0,0));
    }
    strip.show();
    delay(flash_off_delay);

    if (times > 0) {
        flashLeds(green, red, blue, times - 1, flash_on_delay, flash_off_delay);
    }
}

void strobeLeds(int green, int red, int blue, int times, int strobe_up_delay, int strobe_down_delay) {
    int strobe_res = 100;

    double strobe_up_increment = strobe_up_delay / strobe_res;
    double strobe_down_increment = strobe_down_delay / strobe_res;
    double green_inc = green / strobe_res;
    double red_inc = red / strobe_res;
    double blue_inc = blue / strobe_res;

    for (int j = 0; j < NEOPIXEL_COUNT; j++) {
        strip.setPixelColor(j, strip.Color(green, red, blue));
    }
    strip.show();

    for(double i = strobe_up_increment*5; i < strobe_up_delay; i+=strobe_up_increment) {
        for (int j = 0; j < NEOPIXEL_COUNT; j++) {
            strip.setPixelColor(j, strip.Color(i*green_inc/strobe_up_increment, i*red_inc/strobe_up_increment, i*blue_inc/strobe_up_increment));
        }
        strip.show();
        delay(strobe_up_increment);
    }

    for(double i = strobe_down_delay; i > strobe_down_increment*5; i-=strobe_down_increment) {
        for (int j = 0; j < NEOPIXEL_COUNT; j++) {
            strip.setPixelColor(j, strip.Color(i*green_inc/strobe_down_increment, i*red_inc/strobe_down_increment, i*blue_inc/strobe_down_increment));
        }
        strip.show();
        delay(strobe_down_increment);
    }

    for (int j = 0; j < NEOPIXEL_COUNT; j++) {
        strip.setPixelColor(j, strip.Color(0, 0, 0));
    }
    strip.show();


    if (times > 0) {
        strobeLeds(green, red, blue, times - 1, strobe_up_delay, strobe_down_delay);
    }
}

void initLeds() {
    for (int j = 0; j < NEOPIXEL_COUNT; j++) {
      strip.setPixelColor(j, strip.Color(rand() % 255,rand() % 255,rand() % 255));
      strip.show();
      delay(100);
      strip.setPixelColor(j, strip.Color(0, 0, 0));
      strip.show();
    }
}

void loop() {}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}