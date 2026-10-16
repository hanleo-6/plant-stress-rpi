#include "display.h"
#include "i2c.h"
#include "font5x7.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Display (128x64 SSD1306 OLED) using the I2C protocol.
// Shows validity status and CWSI value to the user.
//
// Each write first sends a control byte saying what follows:
//   0x00 -> command bytes, 0x40 -> display data.
// The panel is addressed in pages, 64-pixel height written with
// 8 pixel tall strips at a time

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_PAGES (SSD1306_HEIGHT / 8)

#define SSD1306_CTRL_CMD 0x00
#define SSD1306_CTRL_DATA 0x40

#define GLYPH_W FONT5X7_WIDTH // glyph width, owned by font5x7.h
#define CHAR_W (GLYPH_W + 1) // includes a blank column for spacing

// ssd1306_cmd: send a single command byte
static int ssd1306_cmd(int fd, uint8_t c) {
    uint8_t buf[2] = { SSD1306_CTRL_CMD, c };
    return i2c_write(fd, buf, sizeof buf);
}

// ssd1306_cursor: set the next data write at (page, col)
static int ssd1306_cursor(int fd, uint8_t page, uint8_t col) {
    if (ssd1306_cmd(fd, 0xB0 | page) != 0 || // page start address
        ssd1306_cmd(fd, 0x00 | (col & 0x0F)) != 0 || // column low nibble
        ssd1306_cmd(fd, 0x10 | (col >> 4)) != 0) { // column high nibble
        return -1;
    }
    return 0;
}

// ssd1306_putc: draw single glyph at the cursor,
//               then advance one character cell forwards
static int ssd1306_putc(int fd, char c) {
    if (c < 0x20 || c > 0x7E) { // unknown
        c = '?';
    }
    uint8_t buf[CHAR_W + 1];
    buf[0] = SSD1306_CTRL_DATA;
    memcpy(buf + 1, font5x7[c - 0x20], GLYPH_W);
    buf[GLYPH_W + 1] = 0x00; // insert blank space after character
    return i2c_write(fd, buf, CHAR_W + 1); // ctrl + glyph + blank only
}

// ssd1306_puts: draw a string starting at (page, col)
static int ssd1306_puts(int fd, uint8_t page, uint8_t col, const char *s) {
    if (ssd1306_cursor(fd, page, col) != 0) {
        return -1;
    }
    for (; *s != '\0'; s++) {
        if (ssd1306_putc(fd, *s) != 0) {
            return -1;
        }
    }
    return 0;
}

// display_show_state: write validity state line (always shown)
static int display_show_vstate(int fd, validity_state_t state) {
    char line[32];
    snprintf(line, sizeof line, "State: %s", validity_to_string(state));
    return ssd1306_puts(fd, 2, 0, line);
}

// display_show_cwsi: write the CWSI line, or a placeholder when not valid
static int display_show_cwsi(int fd, double cwsi, bool valid) {
    char line[32];
    if (valid) {
        snprintf(line, sizeof line, "CWSI:  %.2f", cwsi);
    } else {
        snprintf(line, sizeof line, "CWSI:  --");
    }
    return ssd1306_puts(fd, 4, 0, line);
}

// display_init: initialise display panel
// values tuned after being taken from:
//    https://github.com/Matiasus/SSD1306/blob/master/lib/ssd1306.c
int display_init(int fd) {
    static const uint8_t init[] = {
        0xAE,       // display on/off:       off (configuring)
        0xD5, 0x80, // clock divide/osc:     0x80 (default)
        0xA8, 0x3F, // multiplex ratio:      0x3F -> 63+1 active rows
        0xD3, 0x00, // display offset:       0 rows
        0x40,       // start line:           0
        0x8D, 0x14, // charge pump:          0x14 = enabled
        0x20, 0x02, // addressing mode:      0x02 = page
        0xA1,       // segment remap:        mirror horizontal
        0xC8,       // COM scan direction:   flip vertical
        0xDA, 0x12, // COM pins config:      0x12 = alternative (128x64)
        0x81, 0xCF, // contrast:             0xCF
        0xD9, 0xF1, // pre-charge period:    0xF1 (default)
        0xDB, 0x40, // VCOMH deselect level: 0x40 (default)
        0xA4,       // entire display:       follow RAM (default)
        0xA6,       // pixel mode:           normal
        0xAF,       // display on/off:       on
    };
    for (size_t i = 0; i < sizeof init; i++) {
        if (ssd1306_cmd(fd, init[i]) != 0) {
            return -1;
        }
    }
    return display_clear(fd);
}

// display_render: render a given reading and validity state on display
int display_render(int fd, double cwsi, bool valid, validity_state_t state) {
    if (display_clear(fd) != 0) {
        return -1;
    }
    if (ssd1306_puts(fd, 0, 0, "Plant CWSI :D") != 0) {
        return -1;
    }
    if (display_show_vstate(fd, state) != 0) {
        return -1;
    }
    if (display_show_cwsi(fd, cwsi, valid) != 0) {
        return -1;
    }
    return 0;
}

// display_clear: blank out display
int display_clear(int fd) {
    uint8_t row[SSD1306_WIDTH + 1];
    row[0] = SSD1306_CTRL_DATA;
    memset(row + 1, 0x00, SSD1306_WIDTH);

    for (uint8_t page = 0; page < SSD1306_PAGES; page++) {
        if (ssd1306_cursor(fd, page, 0) != 0) {
            return -1;
        }
        if (i2c_write(fd, row, SSD1306_WIDTH + 1) != 0) { // ctrl + one page row
            return -1;
        }
    }
    return 0;
}

