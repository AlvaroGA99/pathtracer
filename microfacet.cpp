/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/bsdf.h>
#include <nori/frame.h>
#include <nori/warp.h>

NORI_NAMESPACE_BEGIN

class Microfacet : public BSDF {
public:
    Microfacet(const PropertyList &propList) {
        /* RMS surface roughness */
        m_alpha = propList.getFloat("alpha", 0.1f);

        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);

        /* Albedo of the diffuse base material (a.k.a "kd") */
        m_kd = propList.getColor("kd", Color3f(0.5f));

        /* To ensure energy conservation, we must scale the 
           specular component by 1-kd. 

           While that is not a particularly realistic model of what 
           happens in reality, this will greatly simplify the 
           implementation. Please see the course staff if you're 
           interested in implementing a more realistic version 
           of this BRDF. */
        m_ks = 1 - m_kd.maxCoeff();
    }

    /// Evaluate the BRDF for the given pair of directions
    Color3f eval(const BSDFQueryRecord &bRec) const {

        if (bRec.measure != ESolidAngle || Frame::cosTheta(bRec.wi) <= 0
            || Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        float cos_theta_i = Frame::cosTheta(bRec.wi);
        float cos_theta_o = Frame::cosTheta(bRec.wo);

        Normal3f wh = (bRec.wi + bRec.wo).normalized();

        float Dh = beckmann(wh);
        float F = fresnel(wh.dot(bRec.wi), m_extIOR, m_intIOR);
        float G = smithG1(bRec.wi, wh) * smithG1(bRec.wo, wh);

        float cos2 = cos_theta_i * cos_theta_o;
        /*if (cos2 < 0) {
            cos2 = -cos2;
        }*/

        return m_kd / M_PI + m_ks * (Dh * F * G) / (4.f * cos2);
    }

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
        float cosTheta = Frame::cosTheta(bRec.wo);
        Vector3f wh = (bRec.wi + bRec.wo).normalized();
        float D = beckmann(wh);
        float Jh = 1.f / (4.f * abs(wh.dot(bRec.wo)));

        return m_ks * D * Frame::cosTheta(wh) * Jh + (1 - m_ks) * cosTheta * INV_PI;
    }

    /// Sample the BRDF
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &_sample) const {

        // Note: Once you have implemented the part that computes the scattered
        // direction, the last part of this function should simply return the
        // BRDF value divided by the solid angle density and multiplied by the
        // cosine factor from the reflection equation, i.e.
        // return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);

        bRec.measure = ESolidAngle;

        Point2f sample(_sample);
        if (sample(0) <= m_ks) {
            //Specular
            sample(0) = sample(0) / m_ks; // transform sample into range [0;1]
            Normal3f n = Warp::squareToBeckmann(sample, m_alpha);
            bRec.wo = 2 * n.dot(bRec.wi) * n - bRec.wi;
        }
        else {
            //Diffuse
            sample(0) = (sample(0) - m_ks) / (1 - m_ks); // transform sample into range [0;1]
            bRec.wo = Warp::squareToCosineHemisphere(sample);
        }
        
        if ( Frame::cosTheta(bRec.wo) <= 0)
            return Color3f(0.0f);

        float pdfval = pdf(bRec);
        if (pdfval > 0)
            return eval(bRec) / pdfval * Frame::cosTheta(bRec.wo);
        else
            return Color3f(0.f);
    }

    bool isDiffuse() const {
        /* While microfacet BRDFs are not perfectly diffuse, they can be
           handled by sampling techniques for diffuse/non-specular materials, 
           hence we return true here */
        return true;
    }

    float beckmann(const Normal3f& n) const {
        float temp = Frame::tanTheta(n) / m_alpha,
            ct = Frame::cosTheta(n), ct2 = ct * ct;

        return std::exp(-temp * temp)
            / (M_PI * m_alpha * m_alpha * ct2 * ct2);
    }

    float smithG1(const Vector3f& v, const Normal3f& n) const {
        float tanTheta = Frame::tanTheta(v);

        if (tanTheta == 0.0f)
            return 1.0f;

        if (n.dot(v) * Frame::cosTheta(v) <= 0)
            return 0.0f;

        float a = 1.0f / (m_alpha * tanTheta);
        if (a >= 1.6f)
            return 1.0f;
        float a2 = a * a;

        return (3.535f * a + 2.181f * a2)
            / (1.0f + 2.276f * a + 2.577f * a2);
    }

    std::string toString() const {
        return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            m_kd.toString(),
            m_ks
        );
    }
private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;
};

NORI_REGISTER_CLASS(Microfacet, "microfacet");
NORI_NAMESPACE_END
