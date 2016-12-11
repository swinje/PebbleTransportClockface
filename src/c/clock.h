#pragma once

#include "pebble.h"

#define NUM_CLOCK_TICKS 13

static const struct GPathInfo ANALOG_BG_POINTS[] = {
  { 4,
    (GPoint []) {
      {68, 0},
      {71, 0},
      {71, 12},
      {68, 12}
    }
  },
  { 4, (GPoint []){
      {72, 0},
      {75, 0},
      {75, 12},
      {72, 12}
    }
  },
  { 4, (GPoint []){
      {112, 10},
      {114, 12},
      {108, 23},
      {106, 21}
    }
  },
  { 4, (GPoint []){
      {132, 47},
      {144, 40},
      {144, 44},
      {135, 49}
    }
  },
    { 4, (GPoint []){
      {132, 84},
      {144, 84},
      {144, 88},
      {132, 88}
    }
  },
  { 4, (GPoint []){
      {135, 118},
      {144, 123},
      {144, 126},
      {132, 120}
    }
  },
  { 4, (GPoint []){
      {108, 144},
      {114, 154},
      {112, 157},
      {106, 147}
    }
  },
  { 4, (GPoint []){
      {70, 155},
      {73, 155},
      {73, 167},
      {70, 167}
    }
  },
  { 4, (GPoint []){
      {32, 10},
      {30, 12},
      {36, 23},
      {38, 21}
    }
  },
  { 4, (GPoint []){
      {12, 47},
      {-1, 40},
      {-1, 44},
      {9, 49}
    }
  },
     { 4, (GPoint []){
      {0, 84},
      {12, 84},
      {12, 88},
      {0, 88}
    }
  },
  { 4, (GPoint []){
      {9, 118},
      {-1, 123},
      {-1, 126},
      {12, 120}
    }
  },
  { 4, (GPoint []){
      {36, 144},
      {30, 154},
      {32, 157},
      {38, 147}
    }
  },

};

static const GPathInfo MINUTE_HAND_POINTS = {
  4, (GPoint []) {
    { -5, 10 },
    { 5, 10 },
    { 1, -65 },
    { -1, -65 }
  }
};

static const GPathInfo DOT_POINTS = {
  4, (GPoint []) {
    { -5, 10 },
    { 5, 10 },
    { 1, -65 },
    { -1, -65 }
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  4, (GPoint []){
    {-5, 10},
    {5, 10},
    {1, -40},
    {-1, -40}
  }
};

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_MET1,
  RESOURCE_ID_IMAGE_MET2, 
  RESOURCE_ID_IMAGE_MET3, 
  RESOURCE_ID_IMAGE_MET4,
  RESOURCE_ID_IMAGE_MET5,
  RESOURCE_ID_IMAGE_MET6, 
  RESOURCE_ID_IMAGE_MET7,
  RESOURCE_ID_IMAGE_MET8,
  RESOURCE_ID_IMAGE_MET9, 
  RESOURCE_ID_IMAGE_MET10, 
  RESOURCE_ID_IMAGE_MET11,
  RESOURCE_ID_IMAGE_MET12,
  RESOURCE_ID_IMAGE_MET13, 
  RESOURCE_ID_IMAGE_MET14,
  RESOURCE_ID_IMAGE_MET15,
  RESOURCE_ID_IMAGE_MET20
};

enum MessageKey {
  DIRECTION_KEY = 0x0,      // TUPLE_INT
  DEPTIME_KEY = 0x1,        // TUPLE_CSTRING
  WEATHER_ICON_KEY = 0x2,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x3,  // TUPLE_CSTRING
};