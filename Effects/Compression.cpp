#include <iostream>
#include <sndfile.h>
#include <cmath>
#include <string>

// Open the file
SNDFILE* open_wav(const char* filename, SF_INFO* sfinfo){
    // Clears SF_INFO
    *sfinfo = SF_INFO();
    // Open the file to read
    SNDFILE* infile = sf_open(filename, SFM_READ, sfinfo);
    // Ensure the file is opened properly
    if (!infile) {
        std::cerr << "Error opening file " << filename << ": "
                  << sf_strerror(NULL) << std::endl;
        return nullptr;
    }
    return infile;
}

// Function for compression
void compressor(const float* input_buffer, float* output_buffer, float threshold, std::string ratio, float attack, float release, float output_gain, int num_samples){
    // Convert parameters to usable values
    // The dB value where the signals peaks will be compressed it is converted to linear
    float threshold_val = -60.0f + ((0.0f - -60.0f) * (threshold / 10.0f));
    threshold_val = std::pow(10.0f, (threshold_val / 20.0f));
    // These determine how quickly the compressor turns on and off
    float attack_val = 0.0001f + ((0.1f - 0.0001f) * (attack / 10.0f));
    float attack_coeff = exp(-1.0f / (attack_val * 44100.0f));
    float release_val = 0.01f + ((1.0f - 0.01f) * (release / 10.0f));
    float release_coeff = exp(-1.0f / (release_val * 44100.0f));
    // This is used to boost the total signal 
    float output_gain_val = 24.0f * (output_gain / 10.0f);
    // Determine how much to compress
    float ratio_val = 0.0f;
    if (ratio == "A") {
        ratio_val = 2.0f;
    } else if (ratio == "B") {
        ratio_val = 4.0f;   
    } else if (ratio == "C") {
        ratio_val = 8.0f;
    } else if (ratio == "D") {
        ratio_val = 20.0f;
    }
    // The current gain is how much each sample is being attenuated
    float curr_gain = 1.0f;
    // Loop through and apply algorithm
    for (int n = 0; n < num_samples; ++n){
        // Convert level to dB
        float input_db = 20.0f * log10(std::abs(input_buffer[n]) + 1e-6f);
        // Caclulate Gain Reduction then convert to linear
        float gain_reduction = 0.0f;
        float target_gain = 1.0f;
        // Determine if compression is needed
        if (input_db > threshold_val){
            gain_reduction = input_db  - (threshold_val + ((input_db - threshold_val) / ratio_val));
            target_gain = std::pow(10.0f, (-gain_reduction / 20.0f));            
        }

        // Apply attack and release smoothing
        if (curr_gain > target_gain){
            // Use smoothing equation for attack
            curr_gain = attack_coeff * curr_gain + (1 - attack_coeff) * target_gain;
        } else {
            // Use smoothing equation for release
            curr_gain = release_coeff * curr_gain + (1 - release_coeff) * target_gain;
        }
        // Apply compressor and compesate volume with output gain
        output_buffer[n] = input_buffer[n] * curr_gain * output_gain_val;
    }
}


int main() {
    // Open input WAV file
    SF_INFO sfinfo;
    SNDFILE* infile = open_wav("Compression_Input.wav", &sfinfo);

    if (!infile) {
        return 1;
    }

    // Frames contain one sample per channel
    sf_count_t num_frames = sfinfo.frames;
    sf_count_t num_samples = num_frames * sfinfo.channels;

    float* input_buffer = new float[num_samples];
    float* output_buffer = new float[num_samples];

    // Read the WAV file
    sf_count_t frames_read =
        sf_readf_float(infile, input_buffer, num_frames);

    sf_close(infile);

    if (frames_read != num_frames) {
        std::cerr << "Error: Could not read the entire WAV file.\n";
        delete[] input_buffer;
        delete[] output_buffer;
        return 1;
    }

    // Gather compressor values
    float threshold;
    float attack;
    float release;
    float output_gain;
    std::string ratio;

    while (true) {
        std::cout << "Enter Threshold Value (0-10): ";
        std::cin >> threshold;

        std::cout << "Enter Ratio Value:\n"
                  << "A: 2:1\n"
                  << "B: 4:1\n"
                  << "C: 8:1\n"
                  << "D: 20:1\n";
        std::cin >> ratio;

        std::cout << "Enter Attack Value (0-10): ";
        std::cin >> attack;

        std::cout << "Enter Release Value (0-10): ";
        std::cin >> release;

        std::cout << "Enter Output Gain Value (0-10): ";
        std::cin >> output_gain;

        if (threshold >= 0.0f && threshold <= 10.0f &&
            attack >= 0.0f && attack <= 10.0f &&
            release >= 0.0f && release <= 10.0f &&
            output_gain >= 0.0f && output_gain <= 10.0f &&
            (ratio == "A" || ratio == "B" ||
             ratio == "C" || ratio == "D")) {
            break;
        }

        std::cout << "Invalid input. Try again.\n\n";
    }

    // Apply compression to every sample
    compressor(
        input_buffer,
        output_buffer,
        threshold,
        ratio,
        attack,
        release,
        output_gain,
        static_cast<int>(num_samples)
    );

    // Open output WAV file
    SF_INFO sfoutinfo = sfinfo;
    sfoutinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    const char* output_filename = "Compression_Output.wav";

    SNDFILE* outfile =
        sf_open(output_filename, SFM_WRITE, &sfoutinfo);

    if (!outfile) {
        std::cerr << "Error opening " << output_filename
                  << " for writing: "
                  << sf_strerror(nullptr) << '\n';

        delete[] input_buffer;
        delete[] output_buffer;
        return 1;
    }

    // Write processed audio
    sf_count_t frames_written =
        sf_writef_float(outfile, output_buffer, num_frames);

    if (frames_written != num_frames) {
        std::cerr << "Error: Only wrote "
                  << frames_written << " of "
                  << num_frames << " frames.\n";
    }

    sf_close(outfile);

    delete[] input_buffer;
    delete[] output_buffer;

    std::cout << "Created " << output_filename << '\n';

    return 0;
}