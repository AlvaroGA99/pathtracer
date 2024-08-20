/*
	File added for exercise P2
*/

#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>
#include <nori/sampler.h>
#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Direct illumination: integrator that implements material sampling, emitter light sampling,
 and some basic multiple importance sampling.
 */
class DirectIllumination : public Integrator {
    enum class SAMPLING_MODE {MATERIAL, LIGHT, MIS};

public:
    DirectIllumination(const PropertyList &propList) {
        /* Sampling mode */
        std::string sampling = propList.getString("sampling", "material");
        if (sampling == std::string("MIS"))
        {
            m_sampling = SAMPLING_MODE::MIS;
        }
        else if (sampling == std::string("light"))
        {
            m_sampling = SAMPLING_MODE::LIGHT;
        }
        else
        {
            m_sampling = SAMPLING_MODE::MATERIAL;
        }
    }

	Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
		/* Find the surface that is visible in the requested direction */

        Color3f color = Color3f(0.0f);

        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        if (its.mesh->isEmitter()) {
            const Emitter* emit = its.mesh->getEmitter();
            EmitterQueryRecord emitterQuery(ray.o,its.p,its.shFrame.n );
            color = emit->eval(emitterQuery);
            return color;
        }

        
       

        if (m_sampling == SAMPLING_MODE::MATERIAL) {
            BSDFQueryRecord bsdfQuery(its.shFrame.toLocal(-ray.d));

            const BSDF* bsdf = its.mesh->getBSDF();

            Color3f diff = bsdf->sample(bsdfQuery, sampler->next2D());

            Ray3f ray2(its.p, its.shFrame.toWorld(bsdfQuery.wo));

            Intersection its2;
            if (scene->rayIntersect(ray2, its2)) {
                if (its2.mesh->isEmitter()) {
                    const Emitter* emit = its2.mesh->getEmitter();

                    EmitterQueryRecord emitterQuery( ray2.o,its2.p,its2.shFrame.n);

                    color = emit->eval(emitterQuery)*diff;
                }
            }
        }
        else if (m_sampling == SAMPLING_MODE::LIGHT) {
            //Intersection its;
           
            ////Si no se produce una intersección de devuelve 0
            //if (!scene->rayIntersect(ray, its))
            //    return color;

            //Si se produce:
            Normal3f n = its.shFrame.n;  //normal
            Point3f p = its.p; //punto de intersección del rayo
            Point3f o = ray.o;

            EmitterQueryRecord recE = EmitterQueryRecord(o, p, n);

            ////Añadir al color
            //Para obtener el color del segundo rayo lanzado internamente
            Color3f colorMuestra = scene->sampleEmitter(recE, sampler->next2D());//li

            ////Evaluar el material de la superficie BSDFQueryRecord eval
            BSDFQueryRecord bRec = BSDFQueryRecord(its.shFrame.toLocal(-ray.d), its.shFrame.toLocal(recE.wi), ESolidAngle);
            Color3f colorBSDF = 0;
            colorBSDF = its.mesh->getBSDF()->eval(bRec);

            ////Multiplicar radiancia por el valor de la muestra de luz y el coseno de ambos
            float cos = Frame::cosTheta(its.shFrame.toLocal(recE.wi));

            Color3f colorDef = cos * colorBSDF * colorMuestra;
            color = colorDef;


        }

        else if (m_sampling == SAMPLING_MODE::MIS) {

            Color3f colorMat(0);
            Color3f colorLuz(0);
            float pdfMat = 0.0f;
            float pdfLuz = 0.0f;


            BSDFQueryRecord bsdfQuery(its.shFrame.toLocal(-ray.d));

            const BSDF* bsdf = its.mesh->getBSDF();

            Color3f diff = bsdf->sample(bsdfQuery, sampler->next2D());

            Color3f Li(0.0f);

            Ray3f ray2(its.p, its.shFrame.toWorld(bsdfQuery.wo));

            Intersection its2;
            if (scene->rayIntersect(ray2, its2)) {
                if (its2.mesh->isEmitter()) {
                    const Emitter* emit = its2.mesh->getEmitter();

                    EmitterQueryRecord emitterQuery(ray2.o,its2.p,its2.shFrame.n);

                    colorMat = emit->eval(emitterQuery) * diff;
                }
            }

            pdfMat = bsdf->pdf(bsdfQuery);


            Normal3f n = its.shFrame.n;  //normal
            Point3f p = its.p; //punto de intersección del rayo
            Point3f o = ray.o;

            EmitterQueryRecord recE = EmitterQueryRecord(o, p, n);

            ////Añadir al color
            //Para obtener el color del segundo rayo lanzado internamente
            Color3f colorMuestra = scene->sampleEmitter(recE, sampler->next2D());//li

            ////Evaluar el material de la superficie BSDFQueryRecord eval
            BSDFQueryRecord bRec = BSDFQueryRecord(its.shFrame.toLocal(-ray.d), its.shFrame.toLocal(recE.wi), ESolidAngle);
            Color3f colorBSDF = 0;
            colorBSDF = its.mesh->getBSDF()->eval(bRec);

            ////Multiplicar radiancia por el valor de la muestra de luz y el coseno de ambos
            float cos = Frame::cosTheta(its.shFrame.toLocal(recE.wi));

            colorLuz = cos * colorBSDF * colorMuestra;
            
            pdfLuz = recE.pdf;

            if ((pdfLuz + pdfMat) != 0) {
                color = (pdfMat * colorMat + pdfLuz * colorLuz) / (pdfMat + pdfLuz);
                
            }

           

        }

       

        return color;
	}

    std::string toString() const {
        return tfm::format(
            "DirectIllumination[\n"
            "  method = %s,\n"
            "]",
            m_sampling == SAMPLING_MODE::MATERIAL ? "material" :
                (m_sampling == SAMPLING_MODE::LIGHT ? "light" : "MIS")
        );
    }
private:
    SAMPLING_MODE m_sampling;
};

NORI_REGISTER_CLASS(DirectIllumination, "direct");
NORI_NAMESPACE_END
