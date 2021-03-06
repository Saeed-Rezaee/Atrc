#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

namespace sphere
{

inline std::tuple<real, real, real> get_abc(real radius_square, const Ray &r)
{
    const real A = r.d.length_square();
    const real B = 2 * dot(r.d, r.o);
    const real C = r.o.length_square() - radius_square;
    return { A, B, C };
}

inline void local_geometry_uv_and_coord(
    const FVec3 &local_pos, Vec2 *uv, FCoord *local_coord, real radius) noexcept
{
    real phi = (!local_pos.x && !local_pos.y) ?
               real(0) : std::atan2(local_pos.y, local_pos.x);
    if(phi < 0)
        phi += 2 * PI_r;
    const real sin_theta = math::clamp<real>(local_pos.z / radius, -1, 1);
    const real theta = std::asin(sin_theta);

    *uv = Vec2(real(0.5) * invPI_r * phi, invPI_r * theta + real(0.5));

    FVec3 ex, ey, ez = local_pos.normalize();
    if(std::abs(ez.z - 1) < EPS())
    {
        ex = FVec3(1, 0, 0);
        ey = FVec3(0, 1, 0);
        ez = FVec3(0, 0, 1);
    }
    else if(std::abs(ez.z + 1) < EPS())
    {
        ex = FVec3(0, 1, 0);
        ey = FVec3(1, 0, 0);
        ez = FVec3(0, 0, -1);
    }
    else
    {
        ex = cross({ 0, 0, 1 }, ez);
        ey = cross(ez, ex);
    }

    *local_coord = FCoord(ex, ey, ez);
}

inline bool has_intersection(const Ray &r, real radius) noexcept
{
    const auto [A, B, C] = get_abc(radius * radius, r);
    real delta = B * B - 4 * A * C;
    if(delta < 0)
        return false;
    delta = std::sqrt(delta);

    const real inv2A = real(0.5) / A;
    const real t0 = (-B - delta) * inv2A;
    const real t1 = (-B + delta) * inv2A;

    return r.between(t0) || r.between(t1);
}

inline bool closest_intersection(const Ray &r, real *t, real radius) noexcept
{
    auto [A, B, C] = sphere::get_abc(radius * radius, r);
    real delta = B * B - 4 * A * C;
    if(delta < 0)
        return false;
    delta = std::sqrt(delta);

    real inv2A = real(0.5) / A;
    real t0 = (-B - delta) * inv2A;
    real t1 = (-B + delta) * inv2A;

    if(r.t_max < t0 || t1 < r.t_min)
        return false;
    bool not_t0 = t0 < r.t_min;
    if(not_t0 && t1 > r.t_max)
        return false;

    *t = not_t0 ? t1 : t0;
    return true;
}

} // namespace sphere

AGZ_TRACER_END

