
typedef struct {
    float a0, a1, a2, b1, b2;
} LRCoefficients;


struct Filter {
    LRCoefficients hpfCoeffs;
    LRCoefficients lpfCoeffs;

    void hpfLRCoeffs(float f_crossover, float fs)
    {
        float omega = juce::MathConstants<float>::pi * f_crossover;
        float kappa = omega / std::tan(juce::MathConstants<float>::pi * f_crossover / fs);
        float delta = std::pow(kappa, 2.0f) + std::pow(omega, 2.0f) + 2.0f * kappa * omega;

        hpfCoeffs.a0 = std::pow(kappa, 2.0f) / delta;
        hpfCoeffs.a1 = -2.0f * std::pow(kappa, 2.0f) / delta;
        hpfCoeffs.a2 = std::pow(kappa, 2.0f) / delta;
        hpfCoeffs.b1 = (-2.0f * std::pow(kappa, 2.0f) + 2.0f * std::pow(omega, 2.0f)) / delta;
        hpfCoeffs.b2 = (-2.0f * kappa * omega + std::pow(kappa, 2.0f) + std::pow(omega, 2.0f)) / delta;
    }

    void lpfLRCoeffs(float f_crossover, float fs)
    {
        float omega = juce::MathConstants<float>::pi * f_crossover;
        float kappa = omega / std::tan(juce::MathConstants<float>::pi * f_crossover / fs);
        float delta = std::pow(kappa, 2.0f) + std::pow(omega, 2.0f) + 2.0f * kappa * omega;

        lpfCoeffs.a0 = std::pow(omega, 2.0f) / delta;
        lpfCoeffs.a1 = 2.0f * std::pow(omega, 2.0f) / delta;
        lpfCoeffs.a2 = lpfCoeffs.a0;
        lpfCoeffs.b1 = (-2.0f * std::pow(kappa, 2.0f) + 2.0f * std::pow(omega, 2.0f)) / delta;
        lpfCoeffs.b2 = (-2.0f * kappa * omega + std::pow(kappa, 2.0f) + std::pow(omega, 2.0f)) / delta;
    }

    float lowpass_filter(float input, float& s1, float& s2, const LRCoefficients& c) {
        float output = c.a0 * input + c.a1 * s1 + c.a2 * s2;
        float next_s1 = input - c.b1 * s1 - c.b2 * s2;
        s2 = s1;
        s1 = next_s1;
        return output;
    }

    float highpass_filter(float input, float& s1, float& s2, const LRCoefficients& c) {
        float output = c.a0 * input + c.a1 * s1 + c.a2 * s2;
        float next_s1 = input - c.b1 * s1 - c.b2 * s2;
        s2 = s1;
        s1 = next_s1;
        return output;
    }
};

struct DSP
{
    std::vector<float> h_s1, h_s2, l_s1, l_s2;
    std::vector<std::vector<float>> highBandBuffer, lowBandBuffer;
    std::vector<Filter> filters;

    float f_crossover = 500.0f;
    float fs = 44100.0f;
    float _fGain_01 = 1.0f;
    int _nMaxChannels = 0;
    int _nMaxBlockSize = 0;

    void Init() {}

    void SetMaxBlockSize(int a_nMaxBlockSize) {
        _nMaxBlockSize = a_nMaxBlockSize;
        _ReAllocInternalBuffers(_nMaxChannels);
    }

    void SetMaxChannels(int a_nMaxChannels) {
        if (_nMaxChannels != a_nMaxChannels) {
            _nMaxChannels = a_nMaxChannels;
            _ReAllocInternalBuffers(_nMaxChannels);
        }
    }

    void SetSampleRate(float a_fSampleRate_Hz) {
        fs = a_fSampleRate_Hz;
        UpdateFilters();
    }

    void SetCrossoverFrequency(float a_nCrossoverFreq) {
        f_crossover = a_nCrossoverFreq;
        UpdateFilters();
    }

    void UpdateFilters() {
        for (auto& f : filters) {
            f.hpfLRCoeffs(f_crossover, fs);
            f.lpfLRCoeffs(f_crossover, fs);
        }
    }

    void SetGain(float a_fGain_01) { _fGain_01 = a_fGain_01; }

    void Release() {
        filters.clear();
        highBandBuffer.clear();
        lowBandBuffer.clear();
    }

    void Process(float** a_vAudioBlocksInPlace, int a_nChannels, int a_nSampleCount)
    {
        if (_nMaxChannels < a_nChannels) SetMaxChannels(a_nChannels);

        for (int ch = 0; ch < a_nChannels; ++ch)
        {
            for (int i = 0; i < a_nSampleCount; ++i)
            {
                float inputSample = a_vAudioBlocksInPlace[ch][i];

                float low = filters[ch].lowpass_filter(inputSample, l_s1[ch], l_s2[ch], filters[ch].lpfCoeffs);
                float high = filters[ch].highpass_filter(inputSample, h_s1[ch], h_s2[ch], filters[ch].hpfCoeffs);

                float saturatedLow = tubeSaturation(low, _fGain_01);

                // Add the bands (the 0.707 factor compensates for the peak at the crossover)
                a_vAudioBlocksInPlace[ch][i] = (high + saturatedLow) * 0.707f;
            }
        }
    }

    void _ReAllocInternalBuffers(int numChannels)
    {
        if (numChannels <= 0) return;
        filters.assign(numChannels, Filter());
        h_s1.assign(numChannels, 0.0f);
        h_s2.assign(numChannels, 0.0f);
        l_s1.assign(numChannels, 0.0f);
        l_s2.assign(numChannels, 0.0f);
        UpdateFilters();
    }

    float tubeSaturation(float x, float mixAmount)
    {
        if (mixAmount <= 0.0f) return x;

        float x_drive = x * (1.0f + mixAmount * 2.0f); // Increase intensity based on mix
        float y = std::tanh(x_drive); // more natural soft clipping

        return (y * mixAmount) + (x * (1.0f - mixAmount));
    }
};

