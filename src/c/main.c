#include <pebble.h>
#include "analog.h"
#include "digital.h"
#include "persist.h"
#include "message.h"


int main() {  
  init_message();
  if (!digital()) {
    set_active_watchface(0);
    analog_init();
    app_event_loop();
    analog_deinit();
  } else {
    set_active_watchface(1);
    digital_init();
    app_event_loop();
    digital_deinit();
  }
  deinit_message();  
}
