#include "touch.h"
#include "CST816S.h"
#include "pinout.h"
#include "flash.h"

CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);

void init_touch()
{
  touch.begin();

  // IMPORTANT:
  // Clear any pending touch interrupt before sleeping
  // (reading touch data does this on CST816S)
  touch.available();
  touch.enable_double_click();
}

// Currently not using touch screen
void handle_touch()
{
  if (touch.available()){
    if (touch.gesture() == "DOUBLE CLICK"){
      toggle_flashlight();
    }
  }
}
