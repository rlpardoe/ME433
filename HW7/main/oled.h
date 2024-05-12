#ifndef oled_H__
#define oled_H__

void draw_message(char *p, int x, int y);
void write_letter(unsigned char c, int x, int y);
void start_i2c();

#endif