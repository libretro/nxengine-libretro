#ifndef _TRIG_H
#define _TRIG_H

extern signed int sin_table[256];

char trig_init(void);
void vector_from_angle(uint8_t angle, int speed, int *xs, int *ys);
void ThrowObjectAtPlayer(Object *o, int rand_variance, int speed);

#endif
