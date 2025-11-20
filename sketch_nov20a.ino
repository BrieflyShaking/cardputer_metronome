#include <M5Cardputer.h>

// Modern, minimal, colorful metronome for M5Stack Cardputer

M5GFX display;

int bpm = 120;
int beatsPerBar = 4;
int noteValue = 4;
bool running = false;
unsigned long lastTick = 0;
int currentBeat = 1;

uint16_t beatColors[16];

void setupColors() {
    for (int i = 0; i < 16; i++) {
        float hue = i * (360.0 / 16.0);
        uint8_t r = (uint8_t)(127 + 127 * sin(hue * 3.14159 / 180.0));
        uint8_t g = (uint8_t)(127 + 127 * sin((hue+120) * 3.14159 / 180.0));
        uint8_t b = (uint8_t)(127 + 127 * sin((hue+240) * 3.14159 / 180.0));
        beatColors[i] = display.color565(r, g, b);
    }
}

void clickNormal() {
    M5Cardputer.Speaker.tone(1200, 40);
}

void clickAccent() {
    M5Cardputer.Speaker.tone(1800, 60);
}

void drawUI(int displayedBeat) {
    display.clear(TFT_BLACK);

    display.setTextSize(3);

    // BPM line slightly to the right, color matching the purple in the dots
    int textX_BPM = display.width()/2 - 70;
    int textY_BPM = 15;
    uint16_t purpleColor = beatColors[8]; // choose one of the purple-ish colors from the gradient
    display.setCursor(textX_BPM, textY_BPM);
    display.setTextColor(purpleColor);
    display.print("BPM: "); display.print(bpm);

    // Time line slightly down, centered-ish, color remains
    int textX_Time = display.width()/2 - 80;
    int textY_Time = 55;
    display.setCursor(textX_Time, textY_Time);
    display.setTextColor(TFT_ORANGE);
    display.print("Time: "); display.print(beatsPerBar); display.print("/"); display.print(noteValue);

    // Draw beat dots
    int dotY = 115;
    int dotRadius = 12;
    int spacing = (display.width() - 20) / beatsPerBar;
    for (int i = 0; i < beatsPerBar; i++) {
        int x = 10 + i * spacing + spacing/2;
        uint16_t c = (i == displayedBeat-1) ? beatColors[i % 16] : TFT_DARKGREY;
        display.fillCircle(x, dotY, dotRadius, c);
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    display = M5Cardputer.Display;
    display.setRotation(1);
    setupColors();
    drawUI(currentBeat);
}

void loop() {
    M5Cardputer.update();

    if (M5Cardputer.Keyboard.isKeyPressed(' ')) { running = !running; currentBeat=1; drawUI(currentBeat); delay(200); }
    if (M5Cardputer.Keyboard.isKeyPressed('w')) { bpm++; if(bpm>250) bpm=250; drawUI(currentBeat); delay(120); }
    if (M5Cardputer.Keyboard.isKeyPressed('s')) { bpm--; if(bpm<60) bpm=60; drawUI(currentBeat); delay(120); }
    if (M5Cardputer.Keyboard.isKeyPressed('a')) { beatsPerBar--; if(beatsPerBar<1) beatsPerBar=1; drawUI(currentBeat); delay(120); }
    if (M5Cardputer.Keyboard.isKeyPressed('d')) { beatsPerBar++; if(beatsPerBar>16) beatsPerBar=16; drawUI(currentBeat); delay(120); }
    if (M5Cardputer.Keyboard.isKeyPressed('q')) { noteValue = (noteValue==4)?16:(noteValue==8)?4:8; drawUI(currentBeat); delay(120); }
    if (M5Cardputer.Keyboard.isKeyPressed('e')) { noteValue = (noteValue==4)?8:(noteValue==8)?16:4; drawUI(currentBeat); delay(120); }

    if (running) {
        float multiplier = 4.0 / noteValue;
        unsigned long interval = (unsigned long)((60000.0 / bpm) * multiplier);
        unsigned long now = millis();
        if (now - lastTick >= interval) {
            lastTick = now;
            if (currentBeat == 1) clickAccent(); else clickNormal();
            drawUI(currentBeat);
            currentBeat++; if(currentBeat>beatsPerBar) currentBeat=1;
        }
    }
}
