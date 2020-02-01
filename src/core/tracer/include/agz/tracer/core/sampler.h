﻿#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief rng sampler interface
 */
class Sampler
{
public:

    virtual ~Sampler() = default;

    /**
     * @brief combine the new seed with internal seed to create a new sampler instance
     */
    virtual Sampler *clone(int seed, Arena &arena) const = 0;

    virtual Sample1 sample1() noexcept = 0;

    virtual Sample2 sample2() noexcept;
    virtual Sample3 sample3() noexcept;
    virtual Sample4 sample4() noexcept;
    virtual Sample5 sample5() noexcept;

    template<int N>
    SampleN<N> SampleN() noexcept;

    virtual void start_pixel(int x, int y) = 0;

    virtual bool next_sample() = 0;

    virtual int get_sample_count() const noexcept = 0;
};

inline Sample2 Sampler::sample2() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    return { u, v };
}

inline Sample3 Sampler::sample3() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    return { u, v, w };
}

inline Sample4 Sampler::sample4() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    return { u, v, w, r };
}

inline Sample5 Sampler::sample5() noexcept
{
    const real u = sample1().u;
    const real v = sample1().u;
    const real w = sample1().u;
    const real r = sample1().u;
    const real s = sample1().u;
    return { u, v, w, r, s };
}

template<int N>
SampleN<N> Sampler::SampleN() noexcept
{
    tracer::SampleN<N> ret;
    for(int i = 0; i < N; ++i)
        ret.u[i] = sample1().u;
    return ret;
}

AGZ_TRACER_END
