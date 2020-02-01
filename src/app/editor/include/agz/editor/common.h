#pragma once

#include <QColor>

#include <agz/tracer/tracer.h>

#define AGZ_EDITOR_BEGIN namespace agz::editor {
#define AGZ_EDITOR_END   }

AGZ_EDITOR_BEGIN

using tracer::real;

using tracer::Vec2;
using tracer::Vec3;
using tracer::Vec4;

using tracer::Vec2i;
using tracer::Vec3i;

using tracer::Rect2;
using tracer::Rect2i;

using tracer::Mat3;
using tracer::Mat4;

using Vec3d = math::vec3d;

using tracer::Spectrum;

using tracer::Image2D;

inline Spectrum qcolor_to_spectrum(const QColor &color) noexcept
{
    return {
        real(color.redF()),
        real(color.greenF()),
        real(color.blueF())
    };
}

AGZ_EDITOR_END
