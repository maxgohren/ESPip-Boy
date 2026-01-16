#include "CST816S.h"

CST816S touch(IIC_SDA, IIC_SCL, TP_RST, TP_INT);

void init_touch()
{
  touch.begin();

  // IMPORTANT:
  // Clear any pending touch interrupt before sleeping
  // (reading touch data does this on CST816S)
  touch.available();
}

// Currently not using touch screen
void handle_touch()
{
}
