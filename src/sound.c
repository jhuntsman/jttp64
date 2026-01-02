#include <c64/sid.h>

void init_sid_enhanced() {
    unsigned char* sid_reg = (unsigned char*)0xD400;

    // 1. Hard reset all 29 SID registers to 0
    for (unsigned char i = 0; i < 29; i++) sid_reg[i] = 0;

    // 2. Master Volume + Filter Type
    // 0x1F = Volume 15 + Low-Pass Filter enabled
    sid_reg[24] = 0x1F;

    // 3. Filter Cutoff (The "Muffle" control)
    // Range is 0-2047. We want it low for a deep sound.
    // Try 0x15 for deep rumble, 0x30 for more "hiss"
    sid_reg[21] = 0x00;  // Cutoff Low
    sid_reg[22] = 0x20;  // Cutoff High (Main control)

    // 4. Filter Routing + Resonance
    // 0xF0 = Maximum Resonance (sharper, more character)
    // 0x07 = Route Voice 1, 2, AND 3 through the filter
    sid_reg[23] = 0xF7;
}

void play_thud() {
    sid.voices[0].freq = 0x0800;  // Low frequency for a "heavy" sound
    sid.voices[0].attdec = 0x00;  // Attack: 0, Decay: 0 (Instant start)
    sid.voices[0].susrel = 0x40;  // Sustain: 0, Release: 4 (Fast fade out)

    // Trigger the Noise waveform (0x80) with the Gate bit (0x01)
    sid.voices[0].ctrl = 0x81;

    // Small delay to let the sound start
    for (int i = 0; i < 200; i++);

    // Release the gate to start the decay/release phase
    sid.voices[0].ctrl = 0x80;
}

void play_dragon_snore(const unsigned char phase) {
    if (phase == 0) {  // INHALE
        // Voice 1 (Index 0) - Pulse Rattle
        sid.voices[0].freq = 0x0180;
        sid.voices[0].attdec = 0x8A;
        sid.voices[0].pwm = 0x0200;  // 11-bit Pulse Width
        sid.voices[0].ctrl = 0x41;   // Pulse + Gate

        // Voice 2 (Index 1) - Low Triangle
        sid.voices[1].freq = 0x00C0;
        sid.voices[1].attdec = 0x9A;
        sid.voices[1].ctrl = 0x11;  // Triangle + Gate
    } else {                        // EXHALE
        // Voice 1 (Index 0) - Noise Rumble
        sid.voices[0].freq = 0x0080;
        sid.voices[0].attdec = 0x09;
        sid.voices[0].ctrl = 0x81;

        // Subtle tweak: slightly open the filter during exhale to
        // simulate the dragon opening its mouth
        *((volatile unsigned char*)0xD416) = 0x28;

        // Voice 2 (Index 1) - Deep Sawtooth
        sid.voices[1].freq = 0x0070;
        sid.voices[1].attdec = 0x0C;
        sid.voices[1].ctrl = 0x21;  // Sawtooth + Gate
    }
}

void play_levelup(void) {
    // Inspired by SoundLibrary patterns: data-driven steps
    // Fast attack, short sustain/release for brassy articulation
    sid.voices[0].attdec = 0x06;   // A=0x0, D=6 (quick)
    sid.voices[0].susrel = 0x18;   // S small, R small
    sid.voices[1].attdec = 0x06;
    sid.voices[1].susrel = 0x18;
    sid.voices[2].attdec = 0x08;
    sid.voices[2].susrel = 0x20;

    // Route all voices through the filter and set master volume high
    unsigned char* sid_reg = (unsigned char*)0xD400;
    sid_reg[24] = (sid_reg[24] & 0xF0) | 0x0F; // volume = 15
    sid_reg[23] = 0x07 | 0x70; // route voices into filter + modest resonance

    // Base frequencies
    unsigned int lead_base = 0x0360;
    unsigned int body_base = 0x0420;

    // Pulse-width for body voice for a brassy timbre
    sid.voices[1].pwm = 0x0300;

    // Start voices (lead = saw, body = pulse+saw mix, sub = triangle)
    sid.voices[0].freq = lead_base; sid.voices[0].ctrl = 0x21;
    sid.voices[1].freq = body_base; sid.voices[1].ctrl = 0x61; // pulse + saw bits mixed + gate
    sid.voices[2].freq = lead_base - 0x00C0; sid.voices[2].ctrl = 0x11;

    // Small vibrato LFO table and arpeggio offsets (semi-tone-ish steps)
    const int vib[] = {0, 6, -4, 8};
    const int arp[] = {0, 0x30, 0x60, 0x120};

    // Initial filter open
    sid_reg[21] = 0xF0; // cutoff low
    sid_reg[22] = 0x07; // cutoff high + resonance

    // Play short arpeggio with vibrato modulation
    for (int i = 0; i < 4; ++i) {
        sid.voices[0].freq = lead_base + arp[i] + vib[i];
        sid.voices[1].freq = body_base + (arp[i] / 2) + (vib[i] / 2);

        // Slight filter bite on stronger notes
        unsigned char cutoff_low = 0xC0 - (i * 0x10);
        sid_reg[21] = cutoff_low;

        // Hold duration slightly decreasing for a lively flourish
        for (volatile int t = 0; t < 260 - (i * 30); ++t);
    }

    // Quick upward flourish
    sid.voices[0].freq = lead_base + 0x1A0;
    sid.voices[1].freq = body_base + 0x160;
    for (volatile int t = 0; t < 340; ++t);

    // Release gates crisply
    sid.voices[0].ctrl = 0x20;
    sid.voices[1].ctrl = 0x40;
    sid.voices[2].ctrl = 0x10;

    // Gentle filter close to end for a natural decay
    sid_reg[21] = 0x00;
    sid_reg[22] = 0x20;
}


