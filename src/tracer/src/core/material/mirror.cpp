#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture2d.h>
#include <agz/utility/misc.h>

#include "./utility/fresnel_point.h"

AGZ_TRACER_BEGIN

namespace
{
    class MirrorBSDF : public LocalBSDF
    {
        const ConductorPoint *fresnel_point_;
        Spectrum rc_;

    public:

        MirrorBSDF(const Coord &geometry_coord, const Coord &shading_coord,
                   const ConductorPoint *fresnel_point, const Spectrum &rc) noexcept
            : LocalBSDF(geometry_coord, shading_coord), fresnel_point_(fresnel_point), rc_(rc)
        {
            
        }

        Spectrum eval(const Vec3 &, const Vec3 &, TransportMode) const noexcept override
        {
            return Spectrum();
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
        {
            const Vec3 local_out = shading_coord_.global_to_local(out_dir);
            if(local_out.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;
            
            const Vec3 nwo = local_out.normalize();
            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(Vec3(0, 0, 2 * nwo.z) - nwo);
            ret.pdf      = 1;
            ret.f        = fresnel_point_->eval(nwo.z) * rc_ / std::abs(nwo.z);
            ret.is_delta = true;

            ret.f *= local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir) const noexcept override
        {
            return 0;
        }

        Spectrum albedo() const noexcept override
        {
            return rc_;
        }

        bool is_delta() const noexcept override
        {
            return true;
        }
    };
}

class Mirror : public Material
{
    std::shared_ptr<const Texture2D> ior_;
    std::shared_ptr<const Texture2D> k_;
    std::shared_ptr<const Texture2D> rc_map_;

public:

    Mirror(std::shared_ptr<const Texture2D> color_map, std::shared_ptr<const Texture2D> ior, std::shared_ptr<const Texture2D> k)
    {
        rc_map_ = color_map;
        ior_    = ior;
        k_      = k;
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        const Spectrum rc  = rc_map_->sample_spectrum(inct.uv);
        const Spectrum ior = ior_   ->sample_spectrum(inct.uv);
        const Spectrum k   = k_     ->sample_spectrum(inct.uv);

        const ConductorPoint *fresnel = arena.create<ConductorPoint>(ior, Spectrum(1), k);

        ShadingPoint ret;

        const BSDF *bsdf = arena.create<MirrorBSDF >(inct.geometry_coord, inct.user_coord, fresnel, rc);
        ret.bsdf = bsdf;
        ret.shading_normal = inct.user_coord.z;

        return ret;
    }
};

std::shared_ptr<Material> create_mirror(
    std::shared_ptr<const Texture2D> color_map,
    std::shared_ptr<const Texture2D> eta,
    std::shared_ptr<const Texture2D> k)
{
    return std::make_shared<Mirror>(color_map, eta, k);
}

AGZ_TRACER_END
