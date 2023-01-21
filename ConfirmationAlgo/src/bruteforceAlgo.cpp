/* /Raw asm to pseudo language\
[5483]  r0 = 4;
[5486]  r0 = 1;
[5489]  goto 6027; => should be set to noop in order to avoid confirmation algorithm
[5491]  if (r0 == 6) r1 = 1;
[5495]  if (r1 == 1) goto 5579;

        uint16_t r7, tmp;
[6027]  if (r0 == 0) {
            r0 = r1 + 1;
[6035]  } else {
            if (r1 == 0) {
                r0 = (r0 + MAX_VAL) % MODULO;
                r1 = r7;
                goto [6027];
[6048]      } else {
                tmp = (r0);
                r1 = (r1 + MAX_VAL) % MODULO;
                goto [6027];
                r1 = r0;
                r0 = tmp;
                r0 = (r0 + MAX_VAL) % MODULO;
                goto [6027];
            }

        }

*/

/* /Confirmation Algorithm\

Start conditions: a = 4, b = 1
End condition: a = 6

Let c (register 7)
Confirmation(a, b):
    If (a = 0):
        return b + 1
    If (b = 0):
        return Confirmation(a - 1, c)
    return Confirmation(a - 1, Confirmation(a, b - 1))

// First terms two :
Confirmation(1, b) = b + c + 1
Confirmation(2, b) = (b + 1) * (c + 1) + c

*/

#define MAX_VAL 32767
#define MODULO (MAX_VAL + 1)

#include <iostream>

static uint16_t r7;
/* target => r0 = 6; */

uint16_t confirmationAlgo(uint16_t r0, uint16_t r1) {
    // f(0, x)
    if (r0 == 0)
        return (r1 + 1) % MODULO;

    // f(1, x)
    if (r0 == 1)
        return (r1 + r7 +1) % MODULO;

    // f(2, x)
    if (r0 == 2)
        return (r7 + (r1 + 1) * (r7 + 1)) % MODULO;

    // f(x, 0)
    if (r1 == 0)
        return confirmationAlgo((r0 + MAX_VAL) % MODULO, r7);

    // f(x, y)
    return confirmationAlgo(r0 - 1, confirmationAlgo(r0, (r1 - 1) % MODULO));
}

int main(int argc, char **argv) {
    uint16_t i, ret;

    for (i = 0; i <= MAX_VAL ; i++) {
        r7 = i;
        ret = confirmationAlgo(4, 1);
        if (ret == 6)
            break;
    }

    std::cout << "MAGIC NUMBER FOUND: " << i << std::endl;

    return 0;
}