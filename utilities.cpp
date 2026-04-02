#include "utilities.h"

void primeDebouncer(XCade &xcade)
{
  // Prime the debouncer
  for(uint8_t i=0; i<5; i++)
  {
    xcade.updateInputs();
    delay(20);
  }
}
