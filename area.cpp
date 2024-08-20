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

#include <nori/emitter.h>
#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

class AreaEmitter : public Emitter {
public:
	AreaEmitter(const PropertyList &propList) : m_mesh(NULL) {
		/* Emitted radiance */
		m_radiance = propList.getColor("radiance");
	}

	Color3f sample(EmitterQueryRecord &lRec, const Point2f &sample) const {

		Point2f s(sample);

		// obtener un punto aleatorio en la malla asociada al emisor
		m_mesh->samplePoint(lRec, s);
		
		// Normalizar rayo incidente (el que entra) porque es vector director y siempre tiene módulo 1
		lRec.wi = (lRec.p - lRec.ref).normalized();

		// evaluar la probabilidad de la muestra desde el punto de vista del emisor (con probabilidad uniforme basada en el área)
			// Ya viene evaluado de Mesh cuando hago samplePoint

		// transformar dicha probabilidad al ángulo sólido del punto de destino de la luz
		float cosTheta = lRec.n.dot(-lRec.wi);
		float p = (lRec.p - lRec.ref).squaredNorm() / cosTheta * lRec.pdf;

		float solidAnglePdf = pdf(lRec);

		// comprobar la orientación relativa entre el rayo incidente en la luz y la normal de la malla
			// se hace en la funcion eval

		// Si el rayo incide por la parte interior de la malla, simplemente se devuelve radiancia nula
		if (lRec.pdf <= 0.f)
			return Color3f(0.0f);

		// evaluar la radiancia y ponderarla con la inversa de la probabilidad
		return eval(lRec) / solidAnglePdf;
	}

	float pdf(const EmitterQueryRecord& lRec) const {
		float cosTheta = lRec.n.dot(-lRec.wi);
		//return (lRec.p - lRec.ref).squaredNorm() / cosTheta * lRec.pdf;
		return m_mesh->getPDF() * (lRec.p - lRec.ref).squaredNorm() / cosTheta;
	}

	Color3f eval(const EmitterQueryRecord& lRec) const {
		return (lRec.n.dot(lRec.wi) < 0.f) *  m_radiance;
	}

	void setParent(NoriObject *object) {
		if (object->getClassType() != EMesh)
			throw NoriException("AreaEmitter: attached to a non-mesh object!");
		m_mesh = static_cast<Mesh *>(object);
	}

	std::string toString() const {
		return tfm::format("AreaEmitter[radiance=%s]", m_radiance.toString());
	}

	virtual float getLuminance() const {
		return m_radiance.getLuminance();
	}
private:
	Color3f m_radiance;
	Mesh *m_mesh;
};

NORI_REGISTER_CLASS(AreaEmitter, "area");
NORI_NAMESPACE_END