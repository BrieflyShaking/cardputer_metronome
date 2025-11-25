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

// ------------ KEY DEBOUNCE ------------
unsigned long lastKeyTime[256];  
bool lastKeyState[256];

bool isKeyJustPressed(char c) {
    bool pressed = M5Cardputer.Keyboard.isKeyPressed(c);
    unsigned long now = millis();

    if (pressed && !lastKeyState[(uint8_t)c] && (now - lastKeyTime[(uint8_t)c] > 120)) {
        lastKeyTime[(uint8_t)c] = now;
        lastKeyState[(uint8_t)c] = true;
        return true;
    }

    if (!pressed) {
        lastKeyState[(uint8_t)c] = false;
    }

    return false;
}
// ---------------------------------------


void setupColors() {
    for (int i = 0; i < 16; i++) {
        float hue = i * (360.0 / 16.0);
        uint8_t r = (uint8_t)(127 + 127 * sin(hue * 3.14159 / 180.0));
        uint8_t g = (uint8_t)(127 + 127 * sin((hue+120) * 3.14159 / 180.0));
        uint8_t b = (uint8_t)(127 + 127 * sin((hue+240) * 3.14159 / 180.0));
        beatColors[i] = display.color565(r, g, b);
    }
}

void clickNormal() { M5Cardputer.Speaker.tone(1200, 40); }
void clickAccent() { M5Cardputer.Speaker.tone(1800, 60); }

void drawUI(int displayedBeat) {
    display.clear(TFT_BLACK);

    display.setTextSize(3);

    // BPM
    int textX_BPM = display.width()/2 - 70;
    int textY_BPM = 15;
    uint16_t purpleColor = beatColors[8];
    display.setCursor(textX_BPM, textY_BPM);
    display.setTextColor(purpleColor);
    display.print("BPM: ");
    display.print(bpm);

    // TIME
    int textX_Time = display.width()/2 - 80;
    int textY_Time = 55;
    display.setCursor(textX_Time, textY_Time);
    display.setTextColor(TFT_ORANGE);
    display.print("Time: ");
    display.print(beatsPerBar);
    display.print("/");
    display.print(noteValue);

    // BEAT DOTS
    int dotY = 115;
    int dotRadius = 12;
    int spacing = (display.width() - 20) / beatsPerBar;

    for (int i = 0; i < beatsPerBar; i++) {
        int x = 10 + i * spacing + spacing/2;
        uint16_t c = (i == displayedBeat-1) ? beatColors[i % 16] : TFT_DARKGREY;
        display.fillCircle(x, dotY, dotRadius, c);
    }
}

// ---------------- TAP TEMPO -------------------

unsigned long lastTap = 0;
unsigned long tapIntervals[5] = {0,0,0,0,0};
int tapIndex = 0;
bool tapPrimed = false;

void tapTempo() {
    unsigned long now = millis();

    if (!tapPrimed) {
        tapPrimed = true;
        lastTap = now;
        tapIndex = 0;
        memset(tapIntervals, 0, sizeof(tapIntervals));
        return;
    }

    unsigned long interval = now - lastTap;
    lastTap = now;

    // Reject accidental taps
    if (interval < 80 || interval > 1500) return;

    tapIntervals[tapIndex] = interval;
    tapIndex = (tapIndex + 1) % 5;

    unsigned long sum = 0;
    int count = 0;

    for (int i = 0; i < 5; i++) {
        if (tapIntervals[i] > 0) {
            sum += tapIntervals[i];
            count++;
        }
    }

    if (count > 0) {
        unsigned long avg = sum / count;
        bpm = constrain((int)(60000.0 / avg), 30, 250);
        drawUI(currentBeat);
    }
}

// ---------------------------------------------------

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    display = M5Cardputer.Display;
    display.setRotation(1);
    setupColors();
    memset(lastKeyTime, 0, sizeof(lastKeyTime));
    memset(lastKeyState, 0, sizeof(lastKeyState));

    drawUI(currentBeat);
}

void loop() {
    M5Cardputer.update();

    // Toggle running
    if (isKeyJustPressed(' ')) {
        running = !running;
        currentBeat = 1;
        drawUI(currentBeat);
    }

    if (isKeyJustPressed('w')) { bpm++; if (bpm > 250) bpm = 250; drawUI(currentBeat); }
    if (isKeyJustPressed('s')) { bpm--; if (bpm < 30) bpm = 30; drawUI(currentBeat); }

    if (isKeyJustPressed('a')) { beatsPerBar--; if (beatsPerBar < 1) beatsPerBar = 1; drawUI(currentBeat); }
    if (isKeyJustPressed('d')) { beatsPerBar++; if (beatsPerBar > 16) beatsPerBar = 16; drawUI(currentBeat); }

    if (isKeyJustPressed('q')) { noteValue = (noteValue == 4) ? 16 : (noteValue == 8) ? 4 : 8; drawUI(currentBeat); }
    if (isKeyJustPressed('e')) { noteValue = (noteValue == 4) ? 8 : (noteValue == 8) ? 16 : 4; drawUI(currentBeat); }

    // TAP TEMPO
    if (isKeyJustPressed('m')) {
        tapTempo();
    }

    // METRONOME ENGINE
    if (running) {
        float mult = 4.0 / noteValue;
        unsigned long interval = (unsigned long)((60000.0 / bpm) * mult);
        unsigned long now = millis();

        if (now - lastTick >= interval) {
            lastTick = now;

            if (currentBeat == 1) clickAccent(); 
            else clickNormal();

            drawUI(currentBeat);

            currentBeat++;
            if (currentBeat > beatsPerBar) currentBeat = 1;
        }
    }
}
