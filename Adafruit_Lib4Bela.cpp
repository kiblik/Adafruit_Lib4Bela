#include "Adafruit_Lib4Bela.h"

int print(const char* x) {
  return rt_printf("%s", x);
}
int println(const char* x) {
  return rt_printf("%s", x) || rt_printf("\n");
}
int print(int x) {
  return rt_printf("%d", x);
}
int println(int x) {
  return rt_printf("%d", x) || rt_printf("\n");
}
int println() {
  return rt_printf("\n");
}
