#include "microfacet.h"
#include "mitsuba/core/formatter.h"
#include "mitsuba/core/fwd.h"
#include "mitsuba/core/math.h"
#include "mitsuba/core/spectrum.h"
#include "mitsuba/core/util.h"
#include "rtrans.h"
#include <mitsuba/core/warp.h>
#include <mitsuba/hw/basicshader.h>
#include <mitsuba/render/bsdf.h>

MTS_NAMESPACE_BEGIN

struct MarschnerHairParams {
    // Fiber props
    const Float IoR = 1.55;
    Spectrum albedo;
    const Float eccentricity = 0.9;

    // Surface props
    // R, TT, TRT sequentially
    Float shifts[3];
    Float stdev[3];

    // Glints props
    Float glintScale;
    Float causticWidth;
    const Float causticFade = 0.3;
    const Float causticLimit = 0.5;

    void parse(const Properties& props) {
        albedo = props.getSpectrum("albedo");

        shifts[0] = degToRad(props.getFloat("r_shift", -7.f));
        shifts[1] = -shifts[0] * 0.5;
        shifts[2] = -shifts[0] * 1.5;

        stdev[0] = degToRad(props.getFloat("r_width", 7.f));
        stdev[1] = stdev[0] * 0.5;
        stdev[2] = stdev[0] * 2.0;

        glintScale = props.getFloat("glint_scale", 1.f);
        causticWidth = degToRad(props.getFloat("caustic_width", 15.f));
    }
    void serialize(Stream* stream, InstanceManager* manager) const {
        albedo.serialize(stream);
        for (int i = 0; i < 3; i++) {
            stream->writeFloat(shifts[i]);
        }
        for (int i = 0; i < 3; i++) {
            stream->writeFloat(stdev[i]);
        }

        stream->writeFloat(glintScale);
        stream->writeFloat(causticWidth);
    }
    void deserialize(Stream* stream, InstanceManager* manager) {
        albedo = Spectrum(stream);

        for (int i = 0; i < 3; i++) {
            shifts[i] = stream->readFloat();
        }
        for (int i = 0; i < 3; i++) {
            stdev[i] = stream->readFloat();
        }

        glintScale = stream->readFloat();
        causticWidth = stream->readFloat();
    }
};

inline Float gaussianDistribution(Float x, Float mu, Float sigma) {
    Float sigmaSqr = sigma * sigma;
    Float dev = (x - mu);
    return 1 / sqrt(2.f * M_PI * sigmaSqr) *
           std::exp(-dev * dev * 0.5 / sigmaSqr);
}
inline Float safeAtan2(Float y, Float x) {
    return std::atan2(y, x);
}
inline Point2f toCurveCoordinates(const Intersection& it,
                                  const Vector3f& dLocal) {
    Vector3f dWorld = it.toWorld(dLocal);
    Vector3f dGeoLocal = it.geoFrame.toLocal(dWorld);

    Float theta = math::safe_asin(dGeoLocal.x);
    Float phi = safeAtan2(dGeoLocal.z, dGeoLocal.y);
    return Point2f(theta, phi);
}

Point2f modEta(Float eta, Float cosTheta) {
    Float sinThetaSqr = 1 - cosTheta * cosTheta;
    Float eta1 = std::sqrt(eta * eta - sinThetaSqr) / cosTheta;
    Float eta2 = eta * eta / eta1;
    return {eta1, eta2};
}

Float fresnelDielectric(Float cosThetaI, Float cosThetaT, Float eta1,
                        Float eta2) {
    cosThetaI = std::abs(cosThetaI);
    cosThetaT = std::abs(cosThetaT);
    Float Rs = (cosThetaI - eta1 * cosThetaT) / (cosThetaI + eta1 * cosThetaT);
    Float Rp = (eta2 * cosThetaI - cosThetaT) / (eta2 * cosThetaI + cosThetaT);

    /* No polarization -- return the unpolarized reflectance */
    return 0.5f * (Rs * Rs + Rp * Rp);
}

Spectrum computeAttenuation(Float cosGammaI, Float cosGammaT,
                            const Point2f& etas, const Spectrum& albedoPrime,
                            int p) {
    Float fresnel0 = fresnelDielectric(cosGammaI, cosGammaT, etas.x, etas.y);
    Float fresnel1 =
        fresnelDielectric(cosGammaI, cosGammaT, 1.f / etas.x, 1.f / etas.y);
    Spectrum transmissionFactor =
        Spectrum(-2.f * albedoPrime * (2 * cosGammaT * cosGammaT)).exp();
    Spectrum attenuation = (1 - fresnel0) * (1 - fresnel0) * transmissionFactor;
    for (int i = 0; i < p - 1; i++) {
        attenuation *= fresnel1 * transmissionFactor;
    }
    return attenuation;
}

Float computePathLength(Float h, Float c, Float gammaI, int p) {
    const Float kPiCubic = M_PI * M_PI * M_PI;
    Float denom =
        6 * p * c / M_PI - 2 - 24 * p * c / kPiCubic * gammaI * gammaI;

    return std::sqrt(1 - h * h) * 0.5 / std::abs(denom);
}

Spectrum computeConeContribution(Float c, Float gammaI, const Point2f& etas,
                                 const Spectrum& albedoPrime, int p) {

    // TT term

    Float s = gammaI / M_PI;
    Float gammaT = 3 * c * s - 4 * c * s * s * s;

    Float h = std::sin(gammaI);
    Float cosGammaI = std::cos(gammaI);
    Float cosGammaT = std::cos(gammaT);

    // Attenuation
    Spectrum attenuation =
        computeAttenuation(cosGammaI, cosGammaT, etas, albedoPrime, p);
    Float pathLength = computePathLength(h, c, gammaI, p);
    return attenuation * pathLength;
}

class MarschnerHair : public BSDF {
  public:
    MarschnerHair(const Properties& props) : BSDF(props) {
        mParams.parse(props);
    }

    MarschnerHair(Stream* stream, InstanceManager* manager)
        : BSDF(stream, manager) {
        mParams.deserialize(stream, manager);

        configure();
    }

    void serialize(Stream* stream, InstanceManager* manager) const {
        BSDF::serialize(stream, manager);
        mParams.serialize(stream, manager);
    }

    void configure() {
        m_components.clear();

        m_components.push_back(EGlossyReflection | EGlossyTransmission |
                               EFrontSide);

        m_usesRayDifferentials = false;

        BSDF::configure();
    }

    Spectrum eval(const BSDFSamplingRecord& bRec, EMeasure measure) const {
        // Light direction
        auto sphI = toCurveCoordinates(bRec.its, bRec.wo);
        // Observation direction
        auto sphR = toCurveCoordinates(bRec.its, bRec.wi);

        Float thetaD = (sphR.x - sphI.x) * 0.5;
        Float cosThetaD = std::cos(thetaD);
        Float cosThetaDSqr = cosThetaD * cosThetaD;
        Float thetaH = (sphR.x + sphI.x) * 0.5;
        Float sinThetaT = std::sin(sphI.x) / mParams.IoR;

        const Spectrum albedoPrime =
            mParams.albedo / std::sqrt(1 - sinThetaT * sinThetaT);

        // longtitudinal scattering M
        Float longtR = gaussianDistribution(thetaH - mParams.shifts[0], 0.f,
                                            mParams.stdev[0]);
        Float longtTT = gaussianDistribution(thetaH - mParams.shifts[1], 0.f,
                                             mParams.stdev[1]);
        Float longtTRT = gaussianDistribution(thetaH - mParams.shifts[2], 0.f,
                                              mParams.stdev[2]);

        // eta', eta''
        auto etas = modEta(mParams.IoR, cosThetaD);

        // For Snell's law approximation
        Float c = std::asin(1 / etas.x);

        Float azimR;
        Spectrum azimTT(0.f), azimTRT(0.f);

        Float phi = sphI.y;

        {
            // Phi(p, h) = 2p * gamma_t - 2 * gamma_i + p * pi
            // Phi - phiI = 0
            // One root for R and TT
            Float gammaI = -phi * 0.5;
            Float s = gammaI / M_PI;
            Float gammaT = 3 * c / M_PI * gammaI - 4 * c * s * s * s;

            Float h = std::sin(gammaI);
            Float cosGammaI = std::cos(gammaI);
            Float cosGammaT = std::cos(gammaT);

            // Attenuation
            Float attenuation =
                fresnelDielectric(cosGammaI, cosGammaT, etas.x, etas.y);
            Float pathLength = std::sqrt(1 - h * h) / 4;
            azimR = attenuation * pathLength;
        }

        {

            // TT term
            Float gammaI =
                (1.0 / 12.0) * pow(6, 2.0 / 3.0) *
                    cbrt((1.0 / 3.0) * sqrt(3) * pow(M_PI, 3) *
                             sqrt((-108 * pow(c, 3) + 108 * M_PI * pow(c, 2) +
                                   27 * c * pow(phi, 2) - 54 * M_PI * c * phi -
                                   9 * pow(M_PI, 2) * c + 4 * pow(M_PI, 3)) /
                                  c) /
                             c +
                         3 * (-pow(M_PI, 3) * phi + pow(M_PI, 4)) / c) -
                1.0 / 6.0 * cbrt(6) * (-3 * pow(M_PI, 2) * c + pow(M_PI, 3)) /
                    (c *
                     cbrt((1.0 / 3.0) * sqrt(3) * pow(M_PI, 3) *
                              sqrt((-108 * pow(c, 3) + 108 * M_PI * pow(c, 2) +
                                    27 * c * pow(phi, 2) - 54 * M_PI * c * phi -
                                    9 * pow(M_PI, 2) * c + 4 * pow(M_PI, 3)) /
                                   c) /
                              c +
                          3 * (-pow(M_PI, 3) * phi + pow(M_PI, 4)) / c));

            azimTT = computeConeContribution(c, gammaI, etas, albedoPrime, 1);
        }

        {
            int p = 2;
            Float delta = (1.0 / 256.0) * pow(M_PI, 8) / pow(c, 2) -
                          1.0 / 1728.0 * pow(M_PI, 9) *
                              pow(3 * c * p / M_PI - 1, 3) /
                              (pow(c, 3) * pow(p, 3));
            Float gammaI0 =
                (1.0 / 12.0) * cbrt(18) *
                    cbrt((1.0 / 3.0) * sqrt(3) * pow(M_PI, 3) *
                             sqrt((-432 * pow(c, 3) + 216 * M_PI * pow(c, 2) +
                                   27 * c * pow(phi, 2) - 108 * M_PI * c * phi +
                                   72 * pow(M_PI, 2) * c + 2 * pow(M_PI, 3)) /
                                  c) /
                             c +
                         3 * (-pow(M_PI, 3) * phi + 2 * pow(M_PI, 4)) / c) -
                1.0 / 12.0 * cbrt(12) * (-6 * pow(M_PI, 2) * c + pow(M_PI, 3)) /
                    (c *
                     cbrt(
                         (1.0 / 3.0) * sqrt(3) * pow(M_PI, 3) *
                             sqrt((-432 * pow(c, 3) + 216 * M_PI * pow(c, 2) +
                                   27 * c * pow(phi, 2) - 108 * M_PI * c * phi +
                                   72 * pow(M_PI, 2) * c + 2 * pow(M_PI, 3)) /
                                  c) /
                             c +
                         3 * (-pow(M_PI, 3) * phi + 2 * pow(M_PI, 4)) / c));
            azimTRT +=
                computeConeContribution(c, gammaI0, etas, albedoPrime, 2);

        }

        return (Spectrum(longtR * azimR) + longtTT * azimTT + longtTRT * azimTRT) / cosThetaDSqr;
        // return (azimTT);
    }

    bool validateTypeMask(unsigned int mask) const {
        return (mask & EGlossyReflection) && (mask & EGlossyTransmission);
    }

    Float pdf(const BSDFSamplingRecord& bRec, EMeasure measure) const {
        if (!validateTypeMask(bRec.typeMask) || measure != ESolidAngle) {
            return 0.f;
        }

        return warp::squareToUniformSpherePdf();
    }

    Spectrum sample(BSDFSamplingRecord& bRec, Float& _pdf,
                    const Point2& _sample) const {
        if (!validateTypeMask(bRec.typeMask)) {
            _pdf = 0.f;
            return Spectrum(0.f);
        }
        _pdf = warp::squareToUniformSpherePdf();
        bRec.sampledComponent = 1;
        bRec.sampledType =
            dot(bRec.wo, bRec.wi) > 0 ? EGlossyReflection : EGlossyTransmission;
        return eval(bRec, ESolidAngle) / _pdf;
    }

    Spectrum sample(BSDFSamplingRecord& bRec, const Point2& sample) const {
        Float pdf;
        return MarschnerHair::sample(bRec, pdf, sample);
    }

    void addChild(const std::string& name, ConfigurableObject* child) {
        BSDF::addChild(name, child);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "MarschnerHairParams[" << endl
            << "  id = \"" << getID() << "\"," << endl
            << "  IoR = " << mParams.IoR << "," << endl
            << "  albedo = " << mParams.albedo.toString() << "," << endl
            << "  eccentricity = " << mParams.eccentricity << "," << endl
            << "  shifts = [" << mParams.shifts[0] << ", " << mParams.shifts[1]
            << ", " << mParams.shifts[2] << "]," << endl
            << "  stdev = [" << mParams.stdev[0] << ", " << mParams.stdev[1]
            << ", " << mParams.stdev[2] << "]," << endl
            << "  glintScale = " << mParams.glintScale << "," << endl
            << "  causticWidth = " << mParams.causticWidth << "," << endl
            << "  causticFade = " << mParams.causticFade << "," << endl
            << "  causticLimit = " << mParams.causticLimit << endl
            << "]";
        return oss.str();
    }

    MTS_DECLARE_CLASS()
  private:
    MarschnerHairParams mParams;
};

MTS_IMPLEMENT_CLASS_S(MarschnerHair, false, BSDF)
MTS_EXPORT_PLUGIN(MarschnerHair, "Marschner'03 hair BSCDF");
MTS_NAMESPACE_END
