FONT_H = frontdoor/font.h

all: font

# some characters like * and $ are tricky ...
CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?,.-:\(\)"

font:
	convert -font MSXW32J.TTF -size 8x -pointsize 8 caption:$(CHARS) font.png
	convert font.png MONO:font.bin
	echo "const char chars[] = \"$(CHARS)\";" >$(FONT_H)
	echo "PROGMEM const" >>$(FONT_H)
	xxd -i font.bin >>$(FONT_H)

