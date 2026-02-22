#include "effects.h"
#include <cmath>



/**
Open jack:
jackd

Compile/Run:
g++ -std=c++11 Run_Program.cpp effects.cpp -o Program  -ljack
./main

Connect ports:
jack_connect system:capture_2 Aaron:input
jack_connect Aaron:output system:playback_1
**/ 



// Overdrive/Distortion effect
void tube_screamer(const float* input_buffer, float* output_buffer, 
                   float drive, float tone, float level, int num_samples){
    float gain_val = 12.0f + ((118.0f - 12.0f) * (drive / 10.0f));
    float alpha = tone / 10.0f;
    float volume = 0.5f + ((2.5f - 0.5f) * (level / 10.0f));
    float prev_filtered = 0;
    float sample = 0;
    for (int n = 0; n < num_samples; ++n){
        // Apply simple gain stage and clipping
        sample = gain_val * input_buffer[n];
        sample = tanh(sample / 0.5f) * 0.5f;
        // Apply LPF filter
        float filtered = alpha * sample + (1.0f - alpha) * prev_filtered;
        prev_filtered = filtered;
        // Change volume
        output_buffer[n] = level * filtered;
    }
}

// Tremolo effect
void TR2(const float* input_buffer, float* output_buffer, 
         float rate, float depth, float wave, int num_samples){
    // Determine the real values for parameters
    float rate_val = 1.0f + ((13.0f - 1.0f) * (rate / 10.0f)); 
    float depth_val = depth / 10.0f;
    float wave_val = wave / 10.0f;
    static float phase = 0.0f; // persist phase across calls so LFO continues between buffers
    float LFO, square, triangle;
    // Use a reasonable default sample rate for phase increment (matches JACK or ALSA setup)
    const float sample_rate = 48000.0f;
    float phase_inc = rate_val / sample_rate;
    // Loop through and modify signal
    for (int n = 0; n < num_samples; ++n){
        // Create values that will be updated after the current iteration
        if (phase >= 0.5){
            triangle = 2.0f - 2.0f * phase;
            square = 0.0f;
        }
        else {
            triangle = 2.0f * phase;
            square = 1.0f;
        }
        LFO = (1 - wave_val) * triangle + wave_val * square;
        output_buffer[n] = input_buffer[n] * ((1.0f - depth_val) + (depth_val * LFO));
        phase += phase_inc;
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
    static float last_lpf = 0.0f;
    
    // Determine the real values for parameters
    float regen_val = regen / 10.0f * 0.95f;
    float delay_val = 20.0f + ((900.0f - 20.0f) * (delay / 10.0f)); 
    float mix_val = mix / 10.0f;
    int D = static_cast<int>((delay_val / 1000.0f) * 44100.0f);
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
        feedback_buffer = new float[D];
        for (int i = 0; i < D; ++i) {
            feedback_buffer[i] = 0.0f;
        }
        buffer_size = D;
        index = 0;
        last_lpf = 0.0f;
    }
    
    // Process signal
    for (int n = 0; n < num_samples; ++n){
        float dry = input_buffer[n];
        float delayed = feedback_buffer[index];
        output_buffer[n] = ((1.0f - mix_val) * dry) + (mix_val * delayed);

        // Apply LPF
        last_lpf = (1 - alpha) * delayed + alpha * last_lpf;

        // Update the feedback and iterate index 
        feedback_buffer[index] = dry + (regen_val * last_lpf);
        index++;
        if (index >= D){
            index = 0;
        }
    }
}
