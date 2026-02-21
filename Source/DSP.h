typedef struct {
    float a0, a1, a2, b1, b2;
} LRCoefficients;


struct Filter {
    LRCoefficients hpfCoeffs;
    LRCoefficients lpfCoeffs;

    void hpfLRCoeffs(float f_crossover, float fs)
    {
        float kappa = std::tan(juce::MathConstants<float>::pi * f_crossover / fs);
        float delta = kappa * kappa + 1.0f + std::sqrt(2.0f) * kappa;

        hpfCoeffs.a0 = 1.0f / delta;
        hpfCoeffs.a1 = -2.0f / delta;
        hpfCoeffs.a2 = 1.0f / delta;
        hpfCoeffs.b1 = 2.0f * (kappa * kappa - 1.0f) / delta;
        hpfCoeffs.b2 = (kappa * kappa + 1.0f - std::sqrt(2.0f) * kappa) / delta;
    }

    void lpfLRCoeffs(float f_crossover, float fs)
    {
        float kappa = std::tan(juce::MathConstants<float>::pi * f_crossover / fs);
        float delta = kappa * kappa + 1.0f + std::sqrt(2.0f) * kappa;

        lpfCoeffs.a0 = (kappa * kappa) / delta;
        lpfCoeffs.a1 = 2.0f * (kappa * kappa) / delta;
        lpfCoeffs.a2 = (kappa * kappa) / delta;
        lpfCoeffs.b1 = 2.0f * (kappa * kappa - 1.0f) / delta;
        lpfCoeffs.b2 = (kappa * kappa + 1.0f - std::sqrt(2.0f) * kappa) / delta;
    }

    float lowpass_filter(float input, float& s1, float& s2, const LRCoefficients& c) {
        float output = c.a0 * input + s1;
        s1 = c.a1 * input - c.b1 * output + s2;
        s2 = c.a2 * input - c.b2 * output;
        return output;
    }

    float highpass_filter(float input, float& s1, float& s2, const LRCoefficients& c) {
        float output = c.a0 * input + s1;
        s1 = c.a1 * input - c.b1 * output + s2;
        s2 = c.a2 * input - c.b2 * output;
        return output;
    }
};

struct DSP
{
    std::vector<float> h_s1_a, h_s2_a, h_s1_b, h_s2_b; // States of High-pass (A e B)
    std::vector<float> l_s1_a, l_s2_a, l_s1_b, l_s2_b; // States of Low-pass (A e B)
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
        float safeFreq = std::clamp(f_crossover, 20.0f, (fs * 0.45f));

        for (auto& f : filters) {
            f.hpfLRCoeffs(safeFreq, fs);
            f.lpfLRCoeffs(safeFreq, fs);
        }
    }

    void SetGain(float a_fGain_01) { _fGain_01 = a_fGain_01; }

    float _fGain_01_Output = 1.0f;
    void SetOutputGain(float a_fOutputGain) { _fGain_01_Output = a_fOutputGain; }

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

                // LOW PASS (LR4 = LPF cascade LPF)
                float lp_stage1 = filters[ch].lowpass_filter(inputSample, l_s1_a[ch], l_s2_a[ch], filters[ch].lpfCoeffs);
                float low = filters[ch].lowpass_filter(lp_stage1, l_s1_b[ch], l_s2_b[ch], filters[ch].lpfCoeffs);

                // HIGH PASS (LR4 = HPF cascade HPF)
                float hp_stage1 = filters[ch].highpass_filter(inputSample, h_s1_a[ch], h_s2_a[ch], filters[ch].hpfCoeffs);
                float high = filters[ch].highpass_filter(hp_stage1, h_s1_b[ch], h_s2_b[ch], filters[ch].hpfCoeffs);

                // Saturation only on the bass
                float saturatedLow = tubeSaturation(low, _fGain_01);

                // Direct sum of the bands and application of final output gain (LR4 crosses at -6dB, flat sum at 0dB)
                a_vAudioBlocksInPlace[ch][i] = (high + saturatedLow) * _fGain_01_Output;
            }
        }
    }

    void _ReAllocInternalBuffers(int numChannels)
    {
        if (numChannels <= 0) return;
        filters.assign(numChannels, Filter());
        h_s1_a.assign(numChannels, 0.0f); h_s2_a.assign(numChannels, 0.0f);
        h_s1_b.assign(numChannels, 0.0f); h_s2_b.assign(numChannels, 0.0f);
        l_s1_a.assign(numChannels, 0.0f); l_s2_a.assign(numChannels, 0.0f);
        l_s1_b.assign(numChannels, 0.0f); l_s2_b.assign(numChannels, 0.0f);
        UpdateFilters();
    }

    float tubeSaturation(float x, float mixAmount)
    {
        if (mixAmount <= 0.0f) return x;

        float x_drive = x * (1.0f + mixAmount * 2.0f);
        float y = std::tanh(x_drive);

        return (y * mixAmount) + (x * (1.0f - mixAmount));
    }
};
