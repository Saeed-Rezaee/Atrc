#include <agz/tracer/core/texture3d.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

class GrayGridPoint3D : public Texture3D
{
    texture::texture3d_t<real> data_;
    real max_value_;

protected:

    real sample_real_impl(const Vec3 &uvw) const noexcept override
    {
        real xf = math::saturate(uvw.x) * (data_.width()  - 1);
        real yf = math::saturate(uvw.y) * (data_.height() - 1);
        real zf = math::saturate(uvw.z) * (data_.depth()  - 1);

        int xi = (std::min)(static_cast<int>(std::floor(xf)), data_.width()  - 2);
        int yi = (std::min)(static_cast<int>(std::floor(yf)), data_.height() - 2);
        int zi = (std::min)(static_cast<int>(std::floor(zf)), data_.depth()  - 2);
        
        real xt = xf - static_cast<real>(xi);
        real yt = yf - static_cast<real>(yi);
        real zt = zf - static_cast<real>(zi);

        real y0z0 = math::lerp(data_(xi, yi,     zi), data_(xi + 1, yi,     zi), xt);
        real y1z0 = math::lerp(data_(xi, yi + 1, zi), data_(xi + 1, yi + 1, zi), xt);

        real y0z1 = math::lerp(data_(xi, yi,     zi + 1), data_(xi + 1, yi,     zi + 1), xt);
        real y1z1 = math::lerp(data_(xi, yi + 1, zi + 1), data_(xi + 1, yi + 1, zi + 1), xt);

        real z0 = math::lerp(y0z0, y1z0, yt);
        real z1 = math::lerp(y0z1, y1z1, yt);

        return math::lerp(z0, z1, zt);
    }

public:

    GrayGridPoint3D(const Texture3DCommonParams &common_params, texture::texture3d_t<real> data)
    {
        init_common_params(common_params);
        assert(data.is_available());
        data_ = std::move(data);

        max_value_ = (std::numeric_limits<real>::lowest)();
        for(int z = 0; z < data_.depth(); ++z)
        {
            for(int y = 0; y < data_.height(); ++y)
            {
                for(int x = 0; x < data_.width(); ++x)
                    max_value_ = (std::max)(max_value_, data_(z, y, x));
            }
        }
    }

    int width() const noexcept override
    {
        return data_.width() - 1;
    }

    int height() const noexcept override
    {
        return data_.height() - 1;
    }

    int depth() const noexcept override
    {
        return data_.depth() - 1;
    }

    Spectrum max_spectrum() const noexcept override
    {
        return Spectrum(max_value_);
    }

    real max_real() const noexcept override
    {
        return max_value_;
    }
};

class SpectrumGridPoint3D : public Texture3D
{
    texture::texture3d_t<Spectrum> data_;
    Spectrum max_value_;

protected:

    Spectrum sample_spectrum_impl(const Vec3 &uvw) const noexcept override
    {
        real xf = math::saturate(uvw.x) * (data_.width() - 1);
        real yf = math::saturate(uvw.y) * (data_.height() - 1);
        real zf = math::saturate(uvw.z) * (data_.depth() - 1);

        int xi = (std::min)(static_cast<int>(std::floor(xf)), data_.width() - 2);
        int yi = (std::min)(static_cast<int>(std::floor(yf)), data_.height() - 2);
        int zi = (std::min)(static_cast<int>(std::floor(zf)), data_.depth() - 2);

        real xt = xf - static_cast<real>(xi);
        real yt = yf - static_cast<real>(yi);
        real zt = zf - static_cast<real>(zi);

        Spectrum y0z0 = lerp(data_(xi, yi,     zi), data_(xi + 1, yi,     zi), xt);
        Spectrum y1z0 = lerp(data_(xi, yi + 1, zi), data_(xi + 1, yi + 1, zi), xt);

        Spectrum y0z1 = lerp(data_(xi, yi,     zi + 1), data_(xi + 1, yi,     zi + 1), xt);
        Spectrum y1z1 = lerp(data_(xi, yi + 1, zi + 1), data_(xi + 1, yi + 1, zi + 1), xt);

        Spectrum z0 = lerp(y0z0, y1z0, yt);
        Spectrum z1 = lerp(y0z1, y1z1, yt);

        return lerp(z0, z1, zt);
    }

public:

    SpectrumGridPoint3D(const Texture3DCommonParams &common_params, texture::texture3d_t<Spectrum> data)
    {
        init_common_params(common_params);
        assert(data.is_available());
        data_ = std::move(data);

        max_value_ = Spectrum((std::numeric_limits<real>::lowest)());
        for(int z = 0; z < data_.depth(); ++z)
        {
            for(int y = 0; y < data_.height(); ++y)
            {
                for(int x = 0; x < data_.width(); ++x)
                {
                    auto &texel = data_(z, y, x);
                    for(int c = 0; c < SPECTRUM_COMPONENT_COUNT; ++c)
                        max_value_[c] = (std::max)(max_value_[c], texel[c]);
                }
            }
        }
    }

    int width() const noexcept override
    {
        return data_.width() - 1;
    }

    int height() const noexcept override
    {
        return data_.height() - 1;
    }

    int depth() const noexcept override
    {
        return data_.depth() - 1;
    }

    Spectrum max_spectrum() const noexcept override
    {
        return max_value_;
    }

    real max_real() const noexcept override
    {
        return max_spectrum()[0];
    }
};

std::shared_ptr<Texture3D> create_gray_grid_point3d(
    const Texture3DCommonParams &common_params, texture::texture3d_t<real> data)
{
    return std::make_shared<GrayGridPoint3D>(common_params, std::move(data));
}

std::shared_ptr<Texture3D> create_spectrum_grid_point3d(
    const Texture3DCommonParams &common_params, texture::texture3d_t<Spectrum> data)
{
    return std::make_shared<SpectrumGridPoint3D>(common_params, std::move(data));
}

AGZ_TRACER_END
