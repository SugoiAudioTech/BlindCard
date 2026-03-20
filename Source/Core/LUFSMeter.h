/*
 * Copyright (c) 2026 Sugoi Audio Tech
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#pragma once

#include <cmath>
#include <vector>
#include <algorithm>

namespace blindcard
{

/**
 * EBU R128 Integrated Loudness (LUFS) meter.
 *
 * Implements the K-weighting filter and gated integration
 * described in ITU-R BS.1770-4.
 */
class LUFSMeter
{
public:
    LUFSMeter() = default;

    /** Prepare the meter for a given sample rate and channel count. */
    void prepare (double sampleRate, int numChannels)
    {
        sr_ = sampleRate;
        numCh_ = std::max (1, std::min (numChannels, 16));

        // Pre-compute K-weighting biquad coefficients
        computeKWeightingCoeffs (sampleRate);

        // Reset filter state
        stageA_.assign (static_cast<size_t> (numCh_), BiquadState{});
        stageB_.assign (static_cast<size_t> (numCh_), BiquadState{});

        // Reset gating blocks
        blocks_.clear();
        blockSumPerChannel_.assign (static_cast<size_t> (numCh_), 0.0);
        blockSampleCount_ = 0;
        blockSize_ = static_cast<int> (sr_ * 0.4);  // 400 ms blocks
    }

    /** Feed audio samples. channelData[ch][i] for i in [0, numSamples). */
    void process (const float* const* channelData, int numChannels, int numSamples)
    {
        const int ch = std::min (numChannels, numCh_);

        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                // K-weighting: two cascaded biquads (pre-filter + RLB)
                double x = static_cast<double> (channelData[c][i]);
                double y1 = processBiquad (stageA_[static_cast<size_t> (c)], preA_, x);
                double y2 = processBiquad (stageB_[static_cast<size_t> (c)], rlbA_, y1);
                blockSumPerChannel_[static_cast<size_t> (c)] += y2 * y2;
            }

            ++blockSampleCount_;

            if (blockSampleCount_ >= blockSize_)
            {
                // Compute mean-square for this 400 ms block
                double blockLoudness = 0.0;
                for (int c = 0; c < ch; ++c)
                {
                    double chWeight = channelWeight (c, ch);
                    blockLoudness += chWeight * (blockSumPerChannel_[static_cast<size_t> (c)] / blockSize_);
                }
                blocks_.push_back (blockLoudness);

                // Reset for next block
                std::fill (blockSumPerChannel_.begin(), blockSumPerChannel_.end(), 0.0);
                blockSampleCount_ = 0;
            }
        }
    }

    /** Return EBU R128 integrated loudness in LUFS. */
    float getIntegratedLUFS() const
    {
        if (blocks_.empty())
            return -100.0f;

        // Absolute-gated loudness (−70 LUFS threshold)
        double absThreshold = -70.0;
        double sumAbove = 0.0;
        int countAbove = 0;

        for (auto z : blocks_)
        {
            double lufs = -0.691 + 10.0 * std::log10 (std::max (z, 1e-20));
            if (lufs > absThreshold)
            {
                sumAbove += z;
                ++countAbove;
            }
        }

        if (countAbove == 0)
            return -100.0f;

        double meanAbsGated = sumAbove / countAbove;
        double relThreshold = -0.691 + 10.0 * std::log10 (std::max (meanAbsGated, 1e-20)) - 10.0;

        // Relative-gated loudness
        double sumRel = 0.0;
        int countRel = 0;

        for (auto z : blocks_)
        {
            double lufs = -0.691 + 10.0 * std::log10 (std::max (z, 1e-20));
            if (lufs > relThreshold)
            {
                sumRel += z;
                ++countRel;
            }
        }

        if (countRel == 0)
            return -100.0f;

        double meanRelGated = sumRel / countRel;
        return static_cast<float> (-0.691 + 10.0 * std::log10 (std::max (meanRelGated, 1e-20)));
    }

private:
    // --- Biquad filter ---
    struct BiquadCoeffs { double b0, b1, b2, a1, a2; };
    struct BiquadState  { double z1 = 0.0, z2 = 0.0; };

    static double processBiquad (BiquadState& s, const BiquadCoeffs& c, double x)
    {
        double y = c.b0 * x + s.z1;
        s.z1 = c.b1 * x - c.a1 * y + s.z2;
        s.z2 = c.b2 * x - c.a2 * y;
        return y;
    }

    /** Compute the two-stage K-weighting filter coefficients per ITU-R BS.1770-4. */
    void computeKWeightingCoeffs (double fs)
    {
        // Stage 1: Pre-filter (shelving boost ≈ +4 dB at high frequencies)
        {
            double f0 = 1681.974450955533;
            double G  = 3.999843853973347;
            double Q  = 0.7071752369554196;

            double K  = std::tan (3.14159265358979323846 * f0 / fs);
            double Vh = std::pow (10.0, G / 20.0);
            double Vb = std::pow (Vh, 0.4996667741545416);

            double a0 = 1.0 + K / Q + K * K;
            preA_.b0 = (Vh + Vb * K / Q + K * K) / a0;
            preA_.b1 = 2.0 * (K * K - Vh) / a0;
            preA_.b2 = (Vh - Vb * K / Q + K * K) / a0;
            preA_.a1 = 2.0 * (K * K - 1.0) / a0;
            preA_.a2 = (1.0 - K / Q + K * K) / a0;
        }

        // Stage 2: Revised Low-frequency B-weighting (RLB, high-pass)
        {
            double f0 = 38.13547087602444;
            double Q  = 0.5003270373238773;

            double K  = std::tan (3.14159265358979323846 * f0 / fs);
            double a0 = 1.0 + K / Q + K * K;
            rlbA_.b0 = 1.0 / a0;
            rlbA_.b1 = -2.0 / a0;
            rlbA_.b2 = 1.0 / a0;
            rlbA_.a1 = 2.0 * (K * K - 1.0) / a0;
            rlbA_.a2 = (1.0 - K / Q + K * K) / a0;
        }
    }

    /** ITU-R BS.1770 channel weights (for up to 5.1 layouts). */
    static double channelWeight (int ch, int totalCh)
    {
        if (totalCh <= 2)
            return 1.0;  // Mono / stereo: equal weight
        // 5.x surround: LFE (ch 3) = 0, surround L/R (ch 4,5) = 1.41
        if (ch == 3) return 0.0;
        if (ch >= 4) return 1.41;
        return 1.0;
    }

    double sr_ = 44100.0;
    int numCh_ = 2;

    BiquadCoeffs preA_{}, rlbA_{};
    std::vector<BiquadState> stageA_, stageB_;

    std::vector<double> blocks_;
    std::vector<double> blockSumPerChannel_;
    int blockSampleCount_ = 0;
    int blockSize_ = 0;
};

} // namespace blindcard
