#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/material.h>
#include <agz/tracer/core/texture.h>
#include <agz/tracer_utility/math.h>

#include "./microfacet.h"

AGZ_TRACER_BEGIN

namespace
{
    class DisneyBRDF : public LocalBSDF
    {
        Spectrum base_color_;
        real metallic_;
        real subsurface_;
        real specular_;
        real specular_tint_;
        real roughness_;
        real anisotropic_;
        real sheen_;
        real sheen_tint_;
        real clearcoat_;

        real ax_, ay_;
        real clearcoat_roughness_;

        struct SampleWeights { real wd = 0, ws = 0, wc = 0; } sample_weights_;

        static real f_diffuse(real FI, real FO, real cos_theta_d_2, real roughness) noexcept
        {
            real FD90 = real(0.5) + 2 * cos_theta_d_2 * roughness;
            return (1 + (FD90 - 1) * FI) * (1 + (FD90 - 1) * FO);
        }

        static real f_subsurface(real cos_theta_i, real cos_theta_o, real FI, real FO, real cos_theta_d_2, real roughness) noexcept
        {
            real Fss90 = cos_theta_d_2 * roughness;
            real Fss = (1 + (Fss90 - 1) * FI) * (1 + (Fss90 - 1) * FO);
            return real(1.25) * (Fss * (1 / (cos_theta_i + cos_theta_o) - real(0.5)) + real(0.5));
        }

    public:

        DisneyBRDF(const Coord &geometry_coord, const Coord &shading_coord,
                   const Spectrum &base_color,
                   real metallic,
                   real subsurface,
                   real specular,
                   real specular_tint,
                   real roughness,
                   real anisotropic,
                   real sheen,
                   real sheen_tint,
                   real clearcoat,
                   real clearcoat_gloss) noexcept
            : LocalBSDF(geometry_coord, shading_coord),
              base_color_   (base_color),
              metallic_     (metallic),
              subsurface_   (subsurface),
              specular_     (specular),
              specular_tint_(specular_tint),
              roughness_    (roughness),
              anisotropic_  (anisotropic),
              sheen_        (sheen),
              sheen_tint_   (sheen_tint),
              clearcoat_    (clearcoat)
        {
            real aspect = anisotropic_ > 0 ? std::sqrt(1 - real(0.9) * anisotropic) : real(1);
            ax_ = roughness * roughness / aspect;
            ay_ = roughness * roughness * aspect;

            clearcoat_roughness_ = math::mix(real(0.1), real(0.01), clearcoat_gloss);

            real wd = (std::min)(real(0.8), 1 - metallic);
            real ws = (1 - sample_weights_.wd) * 2 / (2 + clearcoat);
            real wc = (1 - sample_weights_.wd) * clearcoat / (2 + clearcoat);
            sample_weights_ = { wd, ws, wc };
        }

        Spectrum eval(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            Vec3 wi = shading_coord_.global_to_local(in_dir).normalize();
            Vec3 wo = shading_coord_.global_to_local(out_dir).normalize();
            if(wi.z <= 0 || wo.z <= 0)
                return Spectrum();

            Vec3 wh = (wi + wo).normalize();

            real cos_theta_d = dot(wi, wh), cos_theta_d_2 = cos_theta_d * cos_theta_d;

            real cos_theta_h = local_angle::cos_theta(wh), sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real phi_h = local_angle::phi(wh), cos_phi_h = std::cos(phi_h), sin_phi_h = local_angle::cos_2_sin(cos_phi_h);

            real cos_theta_i = local_angle::cos_theta(wi),
                 sin_theta_i = local_angle::cos_2_sin(cos_theta_i), tan_theta_i = sin_theta_i / cos_theta_i;
            real cos_theta_o = local_angle::cos_theta(wo),
                 sin_theta_o = local_angle::cos_2_sin(cos_theta_o), tan_theta_o = sin_theta_o / cos_theta_o;

            real phi_i = local_angle::phi(wi), cos_phi_i = std::cos(phi_i), sin_phi_i = local_angle::cos_2_sin(cos_phi_i);
            real phi_o = local_angle::phi(wo), cos_phi_o = std::cos(phi_o), sin_phi_o = local_angle::cos_2_sin(cos_phi_o);

            real FI = microfacet::one_minus_5(cos_theta_i);
            real FO = microfacet::one_minus_5(cos_theta_o);
            real FD = microfacet::one_minus_5(cos_theta_d);

            real lum = base_color_.lum();;
            if(lum < EPS)
                return Spectrum();
            Spectrum Ctint = base_color_ / lum;

            // diffuse, subsurface and sheen

            real f_d = 0, f_ss = 0;
            Spectrum f_sh;

            if(metallic_ < 1)
            {
                if(subsurface_ < 1)
                    f_d = f_diffuse(FI, FO, cos_theta_d_2, roughness_);
                if(subsurface_ > 0)
                    f_ss = f_subsurface(cos_theta_i, cos_theta_o, FI, FO, cos_theta_d_2, roughness_);
                if(sheen_ > 0)
                    f_sh = math::mix(Spectrum(1), Ctint, sheen_tint_) * sheen_ * FD;
            }

            // specular

            Spectrum Cs = math::mix(real(0.08) * specular_ * math::mix(Spectrum(1), Ctint, specular_tint_), base_color_, metallic_);
            Spectrum Fs = Cs + (Spectrum(1) - Cs) * FD;

            real Gs = microfacet::smith_anisotropic_gtr2(cos_phi_i, sin_phi_i, ax_, ay_, tan_theta_i)
                    * microfacet::smith_anisotropic_gtr2(cos_phi_o, sin_phi_o, ax_, ay_, tan_theta_o);

            real Ds = microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_);

            // clearcoat

            real Fc = 0, Gc = 0, Dc = 0;
            if(clearcoat_ > 0)
            {
                Fc = real(0.04) + real(0.96) * FD;
                Gc = microfacet::smith_gtr2(tan_theta_i, real(0.25))
                   * microfacet::smith_gtr2(tan_theta_o, real(0.25));
                Dc = microfacet::gtr1(sin_theta_h, cos_theta_h, clearcoat_roughness_);
            }

            real microfacet_dem = 1 / (4 * cos_theta_i * cos_theta_o);

            Spectrum val = (1 - metallic_) * (base_color_ / PI_r * math::mix(f_d, f_ss, subsurface_) + f_sh)
                           + Fs * Gs * Ds * microfacet_dem
                           + clearcoat_ * Fc * Gc * Dc * microfacet_dem;
            return val;
        }

        real proj_wi_factor(const Vec3 &wi) const noexcept override
        {
            return std::abs(cos(wi, shading_coord_.z));
        }

        BSDFSampleResult sample(const Vec3 &out_dir, TransportMode transport_mode, const Sample3 &sam) const noexcept override
        {
            Vec3 wo = shading_coord_.global_to_local(out_dir).normalize();

            if(wo.z <= 0)
                return BSDF_SAMPLE_RESULT_INVALID;

            Vec3 wi;

            if(sam.u < sample_weights_.wd)
            {
                // sample diffuse
                wi = math::distribution::zweighted_on_hemisphere(sam.v, sam.w).first;
                wi = wi.normalize();
            }
            else if(sam.u - sample_weights_.wd < sample_weights_.ws)
            {
                // sample specular

                Vec3 wh = microfacet::sample_anisotropic_gtr2(ax_, ay_, { sam.v, sam.w });
                if(wh.z <= 0)
                    return BSDF_SAMPLE_RESULT_INVALID;

                wi = (2 * dot(wo, wh) * wh - wo).normalize();
                if(wi.z <= 0 || dot(wi, wh) <= 0)
                    return BSDF_SAMPLE_RESULT_INVALID;
            }
            else
            {
                // sample clearcoat

                Vec3 wh = microfacet::sample_gtr1(clearcoat_roughness_, { sam.v, sam.w });
                if(wh.z <= 0)
                    return BSDF_SAMPLE_RESULT_INVALID;

                wi = (2 * dot(wo, wh) * wh - wo).normalize();
                if(wi.z <= 0 || dot(wi, wh) <= 0)
                    return BSDF_SAMPLE_RESULT_INVALID;
            }

            Vec3 global_wi = shading_coord_.local_to_global(wi);

            real pdf_val = pdf(global_wi, out_dir, transport_mode);
            if(pdf_val < EPS)
                return BSDF_SAMPLE_RESULT_INVALID;

            BSDFSampleResult ret;

            ret.dir      = global_wi;
            ret.f        = eval(global_wi, out_dir, transport_mode);
            ret.is_delta = false;
            ret.pdf      = pdf_val;
            ret.mode     = transport_mode;

            return ret;
        }

        real pdf(const Vec3 &in_dir, const Vec3 &out_dir, TransportMode) const noexcept override
        {
            Vec3 wi = shading_coord_.global_to_local(in_dir).normalize();
            Vec3 wo = shading_coord_.global_to_local(out_dir).normalize();

            if(wi.z <= 0 || wo.z <= 0)
                return 0;

            Vec3 wh = (wi + wo).normalize();
            real cos_theta_h = local_angle::cos_theta(wh), sin_theta_h = local_angle::cos_2_sin(cos_theta_h);
            real phi_h = local_angle::phi(wh), cos_phi_h = std::cos(phi_h), sin_phi_h = local_angle::cos_2_sin(cos_phi_h);
            real cos_theta_d = dot(wi, wh);

            real diffuse_PDF = math::distribution::zweighted_on_hemisphere_pdf(wi.z);
            real specular_PDF = cos_theta_h * microfacet::anisotropic_gtr2(sin_phi_h, cos_phi_h, sin_theta_h, cos_theta_h, ax_, ay_) / (4 * cos_theta_d);
            real clearcoat_PDF = cos_theta_h * microfacet::gtr1(sin_theta_h, cos_theta_h, clearcoat_roughness_) / (4 * cos_theta_d);

            return sample_weights_.wd * diffuse_PDF + sample_weights_.ws * specular_PDF + sample_weights_.wc * clearcoat_PDF;
        }

        Spectrum albedo() const noexcept override
        {
            return base_color_;
        }
    };
}

class DisneyReflection : public Material
{
    const Texture *base_color_      = nullptr;
    const Texture *metallic_        = nullptr;
    const Texture *subsurface_      = nullptr;
    const Texture *specular_        = nullptr;
    const Texture *specular_tint_   = nullptr;
    const Texture *roughness_       = nullptr;
    const Texture *anisotropic_     = nullptr;
    const Texture *sheen_           = nullptr;
    const Texture *sheen_tint_      = nullptr;
    const Texture *clearcoat_       = nullptr;
    const Texture *clearcoat_gloss_ = nullptr;

public:

    using Material::Material;

    static std::string description()
    {
        return R"___(
disney_reflection [Material]
    base_color      [Texture]
    metallic        [Texture]
    roughness       [Texture]
    subsurface      [Texture] (optional; defaultly set to all_zero)
    specular        [Texture] (optional; defaultly set to all_zero)
    specular_tint   [Texture] (optional; defaultly set to all_zero)
    anisotropic     [Texture] (optional; defaultly set to all_zero)
    sheen           [Texture] (optional; defaultly set to all_zero)
    sheen_tint      [Texture] (optional; defaultly set to all_zero)
    clearcoat       [Texture] (optional; defaultly set to all_zero)
    clearcoat_gloss [Texture] (optional; defaultly set to all_zero)

    see https://airguanz.github.io/2019/02/20/disney-brdf.html
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        ConfigGroup all_zero_group;
        all_zero_group.insert_child("type", std::make_shared<ConfigValue>("all_zero"));

        auto defaulty_all_zero = [&](const char *name)
        {
            if(auto node = params.find_child(name))
                return TextureFactory.create(node->as_group(), init_ctx);
            return TextureFactory.create(all_zero_group, init_ctx);
        };

        base_color_      = TextureFactory.create(params.child_group("base_color"),      init_ctx);
        metallic_        = TextureFactory.create(params.child_group("metallic"),        init_ctx);
        roughness_       = TextureFactory.create(params.child_group("roughness"),       init_ctx);

        subsurface_      = defaulty_all_zero("subsurface");
        specular_        = defaulty_all_zero("specular");
        specular_tint_   = defaulty_all_zero("specular_tint");
        anisotropic_     = defaulty_all_zero("anisotropic");
        sheen_           = defaulty_all_zero("sheen");
        sheen_tint_      = defaulty_all_zero("sheen_tint");
        clearcoat_       = defaulty_all_zero("clearcoat");
        clearcoat_gloss_ = defaulty_all_zero("clearcoat_gloss");

        AGZ_HIERARCHY_WRAP("in initializing disney reflection material")
    }

    ShadingPoint shade(const EntityIntersection &inct, Arena &arena) const override
    {
        Spectrum base_color      = base_color_     ->sample_spectrum(inct.uv);
        real     metallic        = metallic_       ->sample_real(inct.uv);
        real     subsurface      = subsurface_     ->sample_real(inct.uv);
        real     specular        = specular_       ->sample_real(inct.uv);
        real     specular_tint   = specular_tint_  ->sample_real(inct.uv);
        real     roughness       = roughness_      ->sample_real(inct.uv);
        real     anisotropic     = anisotropic_    ->sample_real(inct.uv);
        real     sheen           = sheen_          ->sample_real(inct.uv);
        real     sheen_tint      = sheen_tint_     ->sample_real(inct.uv);
        real     clearcoat       = clearcoat_      ->sample_real(inct.uv);
        real     clearcoat_gloss = clearcoat_gloss_->sample_real(inct.uv);

        auto bsdf = arena.create<DisneyBRDF>(inct.geometry_coord, inct.user_coord,
                                             base_color,
                                             metallic,
                                             subsurface,
                                             specular,
                                             specular_tint,
                                             roughness,
                                             anisotropic,
                                             sheen,
                                             sheen_tint,
                                             clearcoat,
                                             clearcoat_gloss);

        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Material, DisneyReflection, "disney_reflection")

AGZ_TRACER_END
