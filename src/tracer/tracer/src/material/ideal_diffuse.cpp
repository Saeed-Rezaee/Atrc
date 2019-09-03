#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>

#include "./utility/normal_mapper.h"

AGZ_TRACER_BEGIN

namespace
{
    class IdealDiffuseBRDF : public LocalBSDF
    {
        Spectrum albedo_;

    public:

        IdealDiffuseBRDF(const Coord &geometry_coord, const Coord &shading_coord, const Spectrum &albedo) noexcept
            : LocalBSDF(geometry_coord, shading_coord), albedo_(albedo)
        {
            
        }

        Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            if(cause_black_fringes(in_dir, out_dir))
                return eval_for_black_fringes(in_dir, out_dir);

            Vec3 local_in  = shading_coord_.global_to_local(in_dir);
            Vec3 local_out = shading_coord_.global_to_local(out_dir);
            if(local_in.z <= 0 || local_out.z <= 0)
                return { };
            return albedo_ / PI_r * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, in_dir);
        }

        BSDFSampleResult sample(const Vec3 &dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
        {
            if(cause_black_fringes(dir))
                return sample_for_black_fringes(dir, transport_mode, sam);

            if(!shading_coord_.in_positive_z_hemisphere(dir))
                return BSDF_SAMPLE_RESULT_INVALID;

            auto [local_in, pdf] = math::distribution::zweighted_on_hemisphere(sam.u, sam.v);
            if(pdf < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            BSDFSampleResult ret;
            ret.dir      = shading_coord_.local_to_global(local_in).normalize();
            ret.f        = albedo_ / PI_r * local_angle::normal_corr_factor(geometry_coord_, shading_coord_, ret.dir);
            ret.pdf      = pdf;
            ret.mode     = transport_mode;
            ret.is_delta = false;
            
            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            if(cause_black_fringes(in_dir, out_dir))
                return pdf_for_black_fringes(in_dir, out_dir);

            if(!shading_coord_.in_positive_z_hemisphere(in_dir) || !shading_coord_.in_positive_z_hemisphere(out_dir))
                return 0;
            Vec3 local_in = shading_coord_.global_to_local(in_dir).normalize();
            return math::distribution::zweighted_on_hemisphere_pdf(local_in.z);
        }

        Spectrum albedo() const noexcept override
        {
            return albedo_;
        }
    };
}

class IdealDiffuse : public Material
{
    const Texture *albedo_ = nullptr;
    NormalMapper normal_mapper_;

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
ideal_diffuse [Material]
    albedo [Texture] albedo map
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &context) override
    {
        albedo_ = TextureFactory.create(params.child_group("albedo"), context);
        normal_mapper_.initialize(params, context);
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        auto albedo = albedo_->sample_spectrum(inct.uv);
        Coord shading_coord = normal_mapper_.reorient(inct.uv, inct.user_coord);

        ShadingPoint shd;
        shd.bsdf = arena.create<IdealDiffuseBRDF>(inct.geometry_coord, shading_coord, albedo);

        return shd;
    }
};

AGZT_IMPLEMENTATION(Material, IdealDiffuse, "ideal_diffuse")

AGZ_TRACER_END