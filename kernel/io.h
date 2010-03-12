
/* assembly IN, OUT operation wrappers */
#include <stdint.h>

void outb(int16_t port, int8_t value);
int8_t inb(int16_t port);
int16_t inw(int16_t port);

