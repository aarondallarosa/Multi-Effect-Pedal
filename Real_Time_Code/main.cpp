#include <iostream>
#include <csignal>
#include <vector>
#include <cstring>
#include <jack/jack.h>
#include "effects.h"

static jack_port_t *input_port = nullptr;
static jack_port_t *output_port = nullptr;
static jack_client_t *client = nullptr;

static volatile sig_atomic_t keep_running = 1;

// Effect choice and parameters
static int g_effect_choice = 4;
static float g_drive = 5.0f, g_tone = 5.0f, g_level = 5.0f;
static float g_rate = 5.0f, g_depth = 5.0f, g_wave = 5.0f;
static float g_regen = 5.0f, g_delay = 5.0f, g_mix = 5.0f;
static bool g_warm = false;

static void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;
}

int process(jack_nframes_t nframes, void *arg) {
    (void)arg;
    const float *in = (const float *)jack_port_get_buffer(input_port, nframes);
    float *out = (float *)jack_port_get_buffer(output_port, nframes);

    int num_samples = static_cast<int>(nframes);

    if (g_effect_choice == 4) {
        // bypass copy
        std::memcpy(out, in, sizeof(float) * nframes);
        return 0;
    }

    std::vector<float> temp(nframes);

    switch (g_effect_choice) {
        case 1:
            tube_screamer(in, temp.data(), g_drive, g_tone, g_level, num_samples);
            break;
        case 2:
            TR2(in, temp.data(), g_rate, g_depth, g_wave, num_samples);
            break;
        case 3:
            carbon_copy(in, temp.data(), g_regen, g_delay, g_mix, g_warm, num_samples);
            break;
        default:
            // fallback bypass
            std::memcpy(out, in, sizeof(float) * nframes);
            return 0;
    }

    // copy processed buffer to JACK output
    for (jack_nframes_t i = 0; i < nframes; ++i) {
        out[i] = temp[i];
    }

    return 0;
}

int main() {
    const char *client_name = "Aaron";
    jack_status_t status;
    
    // Determine which effect will be used and the parameters
    // This will change a lot when I implement a GUI with "knobs" and the ability to select which pedal and turn on/off
    std::cout << "Available effects:" << std::endl;
    std::cout << "1. Tube Screamer (Overdrive)" << std::endl;
    std::cout << "2. TR2 (Tremolo)" << std::endl;
    std::cout << "3. Carbon Copy (Delay)" << std::endl;
    std::cout << "4. No effect (bypass)" << std::endl;
    std::cout << "Choose an effect (1-4): ";
    std::cin >> g_effect_choice;

    if (g_effect_choice == 1) {
        std::cout << "\nTube Screamer Parameters:" << std::endl;
        std::cout << "Enter Drive Value (0-10): ";
        std::cin >> g_drive;
        std::cout << "Enter Tone Value (0-10): ";
        std::cin >> g_tone;
        std::cout << "Enter Level Value (0-10): ";
        std::cin >> g_level;
    } else if (g_effect_choice == 2) {
        std::cout << "\nTremolo Parameters:" << std::endl;
        std::cout << "Enter Rate Value (0-10): ";
        std::cin >> g_rate;
        std::cout << "Enter Depth Value (0-10): ";
        std::cin >> g_depth;
        std::cout << "Enter Wave Value (0-10): ";
        std::cin >> g_wave;
    } else if (g_effect_choice == 3) {
        std::cout << "\nDelay Parameters:" << std::endl;
        std::cout << "Enter Delay Value (0-10): ";
        std::cin >> g_delay;
        std::cout << "Enter Regen Value (0-10): ";
        std::cin >> g_regen;
        std::cout << "Enter Mix Value (0-10): ";
        std::cin >> g_mix;
        int warm_int = 0;
        std::cout << "Enter Warm (0 or 1): ";
        std::cin >> warm_int;
        g_warm = (warm_int == 1);
    }

    client = jack_client_open(client_name, JackNullOption, &status);
    if (client == nullptr) {
        std::cerr << "Failed to open JACK client, status = " << status << std::endl;
        return 1;
    }

    // Register ports
    input_port = jack_port_register(client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    output_port = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (!input_port || !output_port) {
        std::cerr << "Failed to register JACK ports" << std::endl;
        jack_client_close(client);
        return 1;
    }

    jack_set_process_callback(client, process, nullptr);

    if (jack_activate(client)) {
        std::cerr << "Cannot activate JACK client" << std::endl;
        jack_client_close(client);
        return 1;
    }

    jack_nframes_t sr = jack_get_sample_rate(client);
    jack_nframes_t bs = jack_get_buffer_size(client);
    std::cout << "JACK client active. Sample rate: " << sr << " Buffer size: " << bs << "\n";
    std::cout << "Connect your system capture/playback to the 'input' and 'output' ports." << std::endl;

    // Setup signal handler to allow clean shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    while (keep_running) {
        sleep(1);
    }

    jack_client_close(client);
    std::cout << "JACK client closed, exiting." << std::endl;
    return 0;
}
