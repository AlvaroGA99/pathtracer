#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>
#include <nori/sampler.h>
#include <nori/emitter.h>

#define MAX_PATH_LENGTH 128

NORI_NAMESPACE_BEGIN

class PathTracingNEE : public Integrator {

	struct PathInfo {
	public:
		unsigned int depth;
		Color3f pathThroughput; // la acomulación de G(diapo 23 PathTracing)
		/// Return a human-readable string summary
		std::string toString() const {
			return tfm::format("[%d, %f, %f, %f]", depth, pathThroughput[0], pathThroughput[1], pathThroughput[2]);
		}
	};

public:
	PathTracingNEE(const PropertyList& props) {}

	Color3f Li(const Scene* scene, Sampler* sampler, const Ray3f& ray) const {
		PathInfo pathInfo;
		pathInfo.depth = 0;
		pathInfo.pathThroughput.setOnes();
		return LiRecursive(scene, sampler, ray, pathInfo);
	}

	Color3f LiRecursive(const Scene* scene, Sampler* sampler, const Ray3f& ray, PathInfo& pathInfo) const {
		Color3f myLi = Color3f(0.f);
		Intersection its;
		// calcular myLi: es lo que vale la luz en un vertice

		if (pathInfo.depth >= MAX_PATH_LENGTH)
			return myLi;

		if (!scene->rayIntersect(ray, its))
			return myLi;

		// Ruleta rusa
		float probRR = std::min(pathInfo.pathThroughput.getLuminance(), 1.0f);
		if (sampler->next1D() >= probRR)
			return myLi;
		pathInfo.pathThroughput /= probRR;

		if (its.mesh->isEmitter()) {
			EmitterQueryRecord eQR(ray.o, its.p, its.shFrame.n);
			return its.mesh->getEmitter()->eval(eQR) * pathInfo.pathThroughput;
		}

		//EMS
		EmitterQueryRecord lRec(its.p);
		Color3f lRef = scene->sampleEmitter(lRec, sampler->next2D());
		float lR_pdf = lRec.pdf;

		BSDFQueryRecord bsdfQR_EMS = BSDFQueryRecord(its.toLocal(-ray.d), its.toLocal(lRec.wi), ESolidAngle);
		Color3f bsdfColor = its.mesh->getBSDF()->eval(bsdfQR_EMS);
		float bsdf_pdf = its.mesh->getBSDF()->pdf(bsdfQR_EMS);

		float cosTheta = Frame::cosTheta(its.shFrame.toLocal(lRec.wi));

		float w_ems = bsdf_pdf + lR_pdf > 0.f ? lR_pdf / (bsdf_pdf + lR_pdf) : lR_pdf;

		Color3f ems = lRef * bsdfColor * cosTheta * pathInfo.pathThroughput;

		//BSDF
		BSDFQueryRecord bsdfQR(its.toLocal(-ray.d));
		Color3f fr = its.mesh->getBSDF()->sample(bsdfQR, sampler->next2D());
		float pdf_mat = its.mesh->getBSDF()->pdf(bsdfQR);

		if (bsdfQR.measure == EDiscrete)
			pdf_mat = 1.0f;

		pathInfo.pathThroughput *= fr;

		// recursive
		Ray3f rRay = Ray3f(its.p, its.toWorld(bsdfQR.wo));
		pathInfo.depth++;

		Point3f origin = its.p;

		float pdf_em = 0.f;
		if (scene->rayIntersect(rRay, its) && its.mesh->isEmitter()) {
			EmitterQueryRecord leEmitterQR(origin, its.p, its.shFrame.n);
			pdf_em = its.mesh->getEmitter()->pdf(leEmitterQR);
		}
		float w_mats = pdf_mat + pdf_em > 0.f ? pdf_mat / (pdf_mat + pdf_em) : pdf_mat;

		return (LiRecursive(scene, sampler, rRay, pathInfo) * w_mats + ems * w_ems); // = myLi


	}

	std::string toString() const {
		return "PathTracingNEE[]";
	}
};


NORI_REGISTER_CLASS(PathTracingNEE, "pathtracer_nee");
NORI_NAMESPACE_END