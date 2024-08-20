#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>
#include <nori/sampler.h>
#include <nori/emitter.h>

#define MAX_PATH_LENGTH 128

NORI_NAMESPACE_BEGIN

class PathTracing : public Integrator {

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
	PathTracing(const PropertyList& props) {}

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

		BSDFQueryRecord bsdfQR(its.toLocal(-ray.d));
		Color3f fr = its.mesh->getBSDF()->sample(bsdfQR, sampler->next2D());
		pathInfo.pathThroughput *= fr;

		// recursive
		Ray3f rRay = Ray3f(its.p, its.toWorld(bsdfQR.wo));
		pathInfo.depth++;
		return LiRecursive(scene, sampler, rRay, pathInfo); // = myLi

		
	}

	std::string toString() const {
		return "PathTracing[]";
	}
};


NORI_REGISTER_CLASS(PathTracing, "pathtracer");
NORI_NAMESPACE_END