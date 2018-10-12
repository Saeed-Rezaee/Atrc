#include <Atrc/Material/MatOpr.h>

AGZ_NS_BEG(Atrc)

namespace
{
    class BxDFOpr_Add
        : ATRC_IMPLEMENTS BxDF
    {
        RC<BxDF> lhs_, rhs_;
        Real lProb_;
        SS coef_;

    public:

        BxDFOpr_Add(RC<BxDF> lhs, RC<BxDF> rhs, Real lProb, SS coef);

        BxDFType GetType() const override;

        Spectrum Eval(const Vec3r &wi, const Vec3r &wo) const override;

        Option<BxDFSample> Sample(const Vec3r &wi, BxDFType type) const override;

        Spectrum EmittedRadiance(const Intersection &inct) const override;
    };

    BxDFOpr_Add::BxDFOpr_Add(RC<BxDF> lhs, RC<BxDF> rhs, Real lProb, SS coef)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)), lProb_(lProb), coef_(coef)
    {

    }

    BxDFType BxDFOpr_Add::GetType() const
    {
        return lhs_->GetType() | rhs_->GetType();
    }

    Spectrum BxDFOpr_Add::Eval(const Vec3r &wi, const Vec3r &wo) const
    {
        return coef_ * (Rand() < lProb_ ? lhs_ : rhs_)->Eval(wi, wo);
    }

    Option<BxDFSample> BxDFOpr_Add::Sample(const Vec3r &wi, BxDFType type) const
    {
        auto ret = (Rand() < lProb_ ? lhs_ : rhs_)->Sample(wi, type);
        if(ret)
            ret->coef *= coef_;
        return ret;
    }

    Spectrum BxDFOpr_Add::EmittedRadiance(const Intersection &inct) const
    {
        return coef_ * (SS(lProb_) * lhs_->EmittedRadiance(inct)
            + (1 - SS(lProb_)) * rhs_->EmittedRadiance(inct));
    }
}

MatOpr_Add::MatOpr_Add(RC<Material> lhs, RC<Material> rhs, Real wL, Real wR)
    : lhs_(std::move(lhs)), rhs_(std::move(rhs))
{
    Real dem = wL + wR;
    lProb_ = wL / dem;
    coef_ = SS(dem);
}

Box<BxDF> MatOpr_Add::GetBxDF(const Intersection &inct, const Vec2r &matParam) const
{
    return NewBox<BxDFOpr_Add>(
        lhs_->GetBxDF(inct, matParam), rhs_->GetBxDF(inct, matParam), lProb_, coef_);
}

AGZ_NS_END(Atrc)
