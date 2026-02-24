#include "effects.h"
#include <cmath>



/**
Open jack:
jackd

Compile/Run:
g++ -std=c++11 main.cpp effects.cpp -o main -ljack
./main

Connect ports:
jack_connect system:capture_2 Aaron:input
jack_connect Aaron:output system:playback_1
**/ 


/** So I dont really think this is correct
It sounds very digital, and very noticable when playing chords I think because the intervals of the harmonics are off
Drive circuits add even harmonics from soft clipping to make it sound warmer (2nd, 4th, 6th, etc.)
Distortion and/or Fuzz add odd harmonics from hard clipping to sound harsher and gritty (3rd, 5th, 7th, etc.) 
Im honestly open to switching to whichever is easier to implement for me to understand the fundamentals
**/

// Overdrive/Distortion effect
void tube_screamer(const float* input_buffer, float* output_buffer, 
                   float drive, float tone, float level, int num_samples){  
    // Gain from 12-118 Im still fine tuning this effect to minimize the digital sound
    float gain_val = 12.0f + ((118.0f - 12.0f) * (drive / 10.0f));
    // Determines LPF frequencies (smoothing constant)
    float alpha = tone / 10.0f;
    // (.5 - 2) Times the volume
    float volume = 0.5f + ((2.5f - 0.5f) * (level / 10.0f));
    // LPF[n - 1]
    float lpf = 0.0f;
    // x[n]
    float sample = 0.0f;

    // Loop through and apply DSP algorithm
    for (int n = 0; n < num_samples; ++n){
        // Apply simple gain stage and clipping
        sample = gain_val * input_buffer[n];
        // Unsure if this is the best way to clip
        // It just looks like I am amplifying/attenuating rather than clipping
        // I found this formula: y[n] = (2/pi) * arctan(c + x[n]) where c = [1-9]
        // This might give me odd harmonics
        sample = tanh(sample / 0.5f) * 0.5f;
        // Apply LPF filter
        //  LPF[n] = α * d[n] + (1 - α) * LPF[n - 1] 
        // I think this might be messed up I could just code in an actual buffer array but Im not 100% sure I am updating LPF[n-1] correctly
        float lpf = alpha * sample + (1.0f - alpha) * lpf;
        // Change volume
        output_buffer[n] = volume * filtered;
    }
}

// Tremolo effect
void TR2(const float* input_buffer, float* output_buffer, 
         float rate, float depth, float wave, int num_samples){
    // Determine the real values for paramters
    // Determines the frequency of the LFO 1-13Hz
    float rate_val = 1.0f + ((13.0f - 1.0f) * (rate / 10.0f)); 
    // Determines the intensity of the volume changes
    float depth_val = depth / 10.0f;
    // Determines the shape of the LFO essientially the mix between square and triangular amplitude modulation
    float wave_val = wave / 10.0f;
    // This is the current position of the LFO
    static float phase = 0.0f;
    // LFO that modulates the singals
    float LFO, square, triangle;
    // 48kHz for JACK/ALSA
    const float sample_rate = 48000.0f;
    // Determines how much the phase is incremented by allowing for a specific number of increments before revolving
    float phase_inc = rate_val / sample_rate;

  
    // Loop through and apply DSP algorithm
    for (int n = 0; n < num_samples; ++n){
        // Create values that will be updated after the current iteration
        // Determines if the phase is on the upper/lower part of the circle
        if (phase >= 0.5){
            // When on the lower part of the circle the amplitude modulator is decreasing or at the minimum if the LFO is a square
            triangle = 2.0f - 2.0f * phase;
            square = 0.0f;
        }
        else {
            // When on the upper part of the circle the amplitude modulator is increasing or at a max if a square
            triangle = 2.0f * phase;
            square = 1.0f;
        }
        // Determine how square or triangular the LFO is
        LFO = (1 - wave_val) * triangle + wave_val * square;
        // Apply the LFO to the dry signal while considering the depth of it
        // y[n] = x[n](1 - d) + LFO[n](d)
        output_buffer[n] = input_buffer[n] * ((1.0f - depth_val) + (depth_val * LFO));
        // Update the LFO's position
        phase += phase_inc;
        // Determine if it revolves
        if (phase >= 1){
            phase -= 1;
        }
    }
}

// Digital Delay effect
void carbon_copy(const float* input_buffer, float* output_buffer, 
                 float regen, float delay, float mix, bool warm, int num_samples){
    // Use static buffers to persist across calls
    static float* feedback_buffer = nullptr;
    static int buffer_size = 0;
    static int index = 0;
    static float lpf = 0.0f;
    
    // Determine the real values for parameters
    // Determines how much delayed signal is in the feeback buffer (Cannot be too high)
    float regen_val = regen / 10.0f * 0.95f;
    // Determines the time of the delay 20-900ms real pedal goes to 1200 but that is excessive
    float delay_val = 20.0f + ((900.0f - 20.0f) * (delay / 10.0f)); 
    // Determines the ratio of dry and delayed signal
    float mix_val = mix / 10.0f;
    // Determine number of delayed samples
    const float sample_rate = 48000.0f;
    int D = static_cast<int>((delay_val / 1000.0f) * sample_rate);
    // Determines the frequency of LPF with the smoothing factor alpha
    float alpha;
    if (warm == true){
        alpha = 0.16f;
    }
    else {
        alpha = 0.41f;
    }
    
    // Initialize buffer only once or when size changes
    if (feedback_buffer == nullptr || buffer_size != D) {
        if (feedback_buffer != nullptr) {
            delete[] feedback_buffer;
        }
        // Buffer with D samples
        feedback_buffer = new float[D];
        for (int i = 0; i < D; ++i) {
            feedback_buffer[i] = 0.0f;
        }
        buffer_size = D;
        index = 0;
        lpf = 0.0f;
    }
    
    // Loop through and apply DSP algorithm
    for (int n = 0; n < num_samples; ++n){
        // Get the value of x[n]
        float dry = input_buffer[n];
        // Get the value of F[n - D]
        float delayed = feedback_buffer[index];
      
        // This is the formula I am using to blend the dry and delayed signal
        // y[n] = (1 - m) * x[n] + m * (d[n])
        output_buffer[n] = ((1.0f - mix_val) * dry) + (mix_val * delayed);

        // Apply LPF
        //  LPF[n] = α * d[n] + (1 - α) * LPF[n - 1] 
        lpf = alpha * delayed + (1.0f - alpha) * lpf;

        // Update the feedback and iterate index 
        // F[n] = x[n] + R * LPF[n]
        feedback_buffer[index] = dry + (regen_val * lpf);
        // I
        index++;
        if (index >= D){
            index = 0;
        }
    }
}
