#include <mitsuba/render/bsdf.h>
#include <mitsuba/hw/basicshader.h>
#include <mitsuba/core/warp.h>
#include "microfacet.h"
#include "rtrans.h"
#include "ior.h"
#include "gausslegendre.hpp"

MTS_NAMESPACE_BEGIN

constexpr int INTEGRATOR_NUM_SAMPLE = 140;
constexpr int AZIMUTHAL_PRECOMPUTE_RESOLUTION = 64;
constexpr int GAUSSIAN_DETECTOR_SAMPLES = 2048;
class MarschnerHair : public BSDF
{
public:
    MarschnerHair(const Properties &props) : BSDF(props)
    {
        m_eta = props.getFloat("eta", 1.55f);
        m_beta = props.getFloat("beta", 2.2f);
        m_absorption = props.getSpectrum("absorption", Spectrum(.5f));
        m_beta = degToRad(m_beta);
        beta[0] = m_beta;
        beta[1] = m_beta / 2;
        beta[2] = m_beta * 2;
        m_alpha = props.getFloat("alpha", 3.f);
        m_alpha = degToRad(m_alpha);
        alpha[0] = m_alpha;
        alpha[1] = -m_alpha / 2;
        alpha[2] = -3 * m_alpha / 2;
        precompute();
    }

    MarschnerHair(Stream *stream, InstanceManager *manager)
        : BSDF(stream, manager)
    {
        m_eta = stream->readFloat();
        m_beta = stream->readFloat();
        m_alpha = stream->readFloat();
        m_absorption = Spectrum(stream);
        configure();
    }

    void serialize(Stream *stream, InstanceManager *manager) const
    {
        BSDF::serialize(stream, manager);

        // manager->serialize(stream, m_alpha.get());
        stream->writeFloat(m_eta);
        stream->writeFloat(m_beta);
        stream->writeFloat(m_alpha);
        m_absorption.serialize(stream);
    }

    void configure()
    {

        m_components.clear();

        m_components.push_back(EDiffuseReflection | EFrontSide);
        m_usesRayDifferentials = false;
        precompute();

        BSDF::configure();
    }
    inline Float gaussian_function(Float b, Float t) const
    {
        return std::exp(-t * t / (2 * b * b)) / std::sqrt(2.0f * M_PI) / b;
    }
    Float gaussian_detector(Float b, Float phi) const
    {
        Float result = 0;
        Float x, y, delta;
        int k = 0;
        constexpr Float M_PIx2 = M_PI * 2;
        do
        {
            x = gaussian_function(b, phi - M_PIx2 * Float(k));
            y = gaussian_function(b, phi + M_PIx2 * Float(k + 1));
            delta = x + y;
            k++;
            result += x + y;
        } while (delta > 1e-4f);
        return result;
    }

    inline Float Phi(int p, Float gamma_i, Float gamma_t) const
    {
        return 2 * p * gamma_t - 2 * gamma_i + p * M_PI;
    }

    Spectrum eval(const BSDFSamplingRecord &bRec, EMeasure measure = ESolidAngle) const
    {
        if (!(bRec.typeMask & EDiffuseReflection) || measure != ESolidAngle)
            return Spectrum(0.0f);
        // from shading coord to geo coord
        auto wi_geo = normalize(bRec.its.geoFrame.toLocal(normalize(bRec.its.toWorld(bRec.wi))));
        auto wo_geo = normalize(bRec.its.geoFrame.toLocal(normalize(bRec.its.toWorld(bRec.wo))));

        Float theta_i = std::asin(math::clamp(wi_geo.x, -1.0f, 1.0f));
        Float theta_r = std::asin(math::clamp(wo_geo.x, -1.0f, 1.0f));
        Float phi_i = std::atan2(wi_geo.y, wi_geo.z);
        Float phi_r = std::atan2(wo_geo.y, wo_geo.z);
        Float phi = phi_r - phi_i, theta_d = std::abs((theta_r - theta_i)) / 2, theta_h = (theta_i + theta_r) / 2;
        if (phi < 0.f)
            phi += M_PI * 2;
        if (phi > M_PI * 2)
            phi -= M_PI * 2;
        Spectrum result(0.f);

        // M term, we'll simply use gaussian distribution here
        Float Mp[3];
        for (int i = 0; i < 3; i++)
        {
            Mp[i] = gaussian_function(beta[i], theta_h - alpha[i]);
        }
        float cos_theta_d = std::cos(theta_d);
        Mp[0] = gaussian_function(beta[0], theta_h * 2);
        result += Mp[0] * approxNp(cos_theta_d, phi, 0);
        result += Mp[1] * approxNp(cos_theta_d, phi, 1);
        result += Mp[2] * approxNp(cos_theta_d, phi, 2);

        return result;
    }

    Float pdf(const BSDFSamplingRecord &bRec, EMeasure measure = ESolidAngle) const
    {
        if (!(bRec.typeMask & EDiffuseReflection) || measure != ESolidAngle)
            return 0.f;

        return warp::squareToUniformSpherePdf();
    }

    inline Spectrum sample(BSDFSamplingRecord &bRec, Float &_pdf, const Point2 &_sample) const
    {
        if (!(bRec.typeMask & EDiffuseReflection))
            return Spectrum(0.0f);

        bRec.wo = warp::squareToUniformSphere(_sample);
        bRec.eta = 1.0f;
        bRec.sampledComponent = 0;
        bRec.sampledType = EDiffuseReflection;
        _pdf = warp::squareToUniformSpherePdf();
        return eval(bRec);
    }

    Spectrum sample(BSDFSamplingRecord &bRec, const Point2 &sample) const
    {
        if (!(bRec.typeMask & EDiffuseReflection))
            return Spectrum(0.0f);
        Float pdf;
        return MarschnerHair::sample(bRec, pdf, sample);
    }

    void addChild(const std::string &name, ConfigurableObject *child)
    {
        BSDF::addChild(name, child);
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "MarschnerHair[" << endl
            << "  id = \"" << getID() << "\"," << endl
            << "  eta = " << m_eta << "," << endl
            << "]";
        return oss.str();
    }
    static inline Float dielectricReflectance(Float cosThetaI, Float eta)
    {
        Float cosThetaT;
        if (cosThetaI < 0.0f)
        {
            eta = 1.0f / eta;
            cosThetaI = -cosThetaI;
        }
        Float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);
        if (sinThetaTSq > 1.0f)
        {
            cosThetaT = 0.0f;
            return 1.0f;
        }
        cosThetaT = std::sqrt(std::max(1.0f - sinThetaTSq, 0.0f));

        Float Rs = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
        Float Rp = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);

        return (Rs * Rs + Rp * Rp) * 0.5f;
    }
    void precompute()
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < GAUSSIAN_DETECTOR_SAMPLES; j++)
            {
                GD_table[i][j] = gaussian_detector(beta[i], j / (GAUSSIAN_DETECTOR_SAMPLES - 1.0f) * M_PI * 2.0f);
            }
        }
        // precompute gamma_i table
        Float gamma_i_table[INTEGRATOR_NUM_SAMPLE];
        GaussLegendre<INTEGRATOR_NUM_SAMPLE> integrator;
        const auto weights = integrator.weights();
        const auto points = integrator.points();
        for (int i = 0; i < INTEGRATOR_NUM_SAMPLE; i++)
        {
            gamma_i_table[i] = std::asin(points[i]);
        }
        // precompute azimuthal scattering function from 2 dimension
        // cos(theta_d): [0, 1]
        // phi: [0, 2*PI]
        for (int y = 0; y < AZIMUTHAL_PRECOMPUTE_RESOLUTION; y++)
        {
            Float cos_theta_d = Float(y) / Float(AZIMUTHAL_PRECOMPUTE_RESOLUTION - 1);
            Float sin_td = std::sqrt(1.f - cos_theta_d * cos_theta_d);
            Float eta_prime = std::sqrt(m_eta * m_eta - sin_td * sin_td) / cos_theta_d;
            Float cos_theta_t = std::sqrt(1.f - sin_td * sin_td * math::sqr(1.f / m_eta));
            Spectrum absorption_prime = m_absorption / cos_theta_t;

            Float precom_fresnel[INTEGRATOR_NUM_SAMPLE], gamma_t_table[INTEGRATOR_NUM_SAMPLE];
            Spectrum absorption_table[INTEGRATOR_NUM_SAMPLE];
            for (int k = 0; k < INTEGRATOR_NUM_SAMPLE; k++)
            {
                precom_fresnel[k] = fresnelDielectricExt(cos_theta_d * std::cos(gamma_i_table[k]), m_eta);
                gamma_t_table[k] = std::asin(math::clamp(points[k] / eta_prime, -1.f, 1.f));
                absorption_table[k] = (-2.f * absorption_prime * (1.f + std::cos(2.f * gamma_t_table[k]))).exp();
            }

            for (int x = 0; x < AZIMUTHAL_PRECOMPUTE_RESOLUTION; x++)
            {
                Float phi = M_PI * 2.f * x / (AZIMUTHAL_PRECOMPUTE_RESOLUTION - 1.f);
                for (int i = 0; i < 3; i++)
                {
                    Spectrum Np(0.f);
                    for (int j = 0; j < INTEGRATOR_NUM_SAMPLE; j++)
                    {
                        Float Dp = approxGD(i, phi - Phi(i, gamma_i_table[j], gamma_t_table[j]));
                        Float f = precom_fresnel[j];
                        if (i == 0)
                        {
                            Np += Spectrum(0.5 * f * Dp * weights[j]);
                        }
                        else
                        {
                            Spectrum T = absorption_table[j];
                            Spectrum A = math::sqr(1.f - f) * std::pow(f, i - 1) * T.pow(i);
                            Np += 0.5 * A * Dp * weights[j];
                        }
                    }
                    Np_table[y][x][i] = Np;
                }
            }
        }
    }
    Float approxGD(int p, float phi) const
    {
        float u = std::abs(phi * (0.5f * M_1_PI * (GAUSSIAN_DETECTOR_SAMPLES - 1)));
        int x0 = int(u);
        int x1 = x0 + 1;
        u -= x0;
        return GD_table[p][x0 % GAUSSIAN_DETECTOR_SAMPLES] * (1.0f - u) + GD_table[p][x1 % GAUSSIAN_DETECTOR_SAMPLES] * u;
    };
    Spectrum approxNp(Float cos_theta_d, Float phi, int i) const
    {
        // costhetad -> phi
        float u = (AZIMUTHAL_PRECOMPUTE_RESOLUTION - 1) * phi * M_1_PI * 0.5f;
        float v = (AZIMUTHAL_PRECOMPUTE_RESOLUTION - 1) * cos_theta_d;
        int x0 = math::clamp(int(u), 0, AZIMUTHAL_PRECOMPUTE_RESOLUTION - 2);
        int y0 = math::clamp(int(v), 0, AZIMUTHAL_PRECOMPUTE_RESOLUTION - 2);
        int x1 = x0 + 1;
        int y1 = y0 + 1;
        u = math::clamp(u - x0, 0.0f, 1.0f);
        v = math::clamp(v - y0, 0.0f, 1.0f);

        return (Np_table[y0][x0][i] * (1.0f - u) + Np_table[y0][x1][i] * u) * (1.0f - v) +
               (Np_table[y1][x0][i] * (1.0f - u) + Np_table[y1][x1][i] * u) * v;
    }

    MTS_DECLARE_CLASS()
private:
    Float m_eta;
    // longtitudinal shift
    Float m_alpha;
    Float alpha[3];
    // longtitudinal width
    Float m_beta;
    Float beta[3];
    Float GD_table[3][GAUSSIAN_DETECTOR_SAMPLES];
    Spectrum m_absorption;
    Spectrum Np_table[AZIMUTHAL_PRECOMPUTE_RESOLUTION][AZIMUTHAL_PRECOMPUTE_RESOLUTION][3];
};

MTS_IMPLEMENT_CLASS_S(MarschnerHair, false, BSDF)
MTS_EXPORT_PLUGIN(MarschnerHair, "Marschner's Hair BSDF");
MTS_NAMESPACE_END
