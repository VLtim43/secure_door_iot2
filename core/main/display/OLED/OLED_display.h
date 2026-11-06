#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

void oled_display_start(void);
// Replace screen contents with text, wrapped across up to eight 16-character lines.
void oled_display_show_text(const char* text);

#endif
