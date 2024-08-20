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

#include <nori/scene.h>
#include <nori/bitmap.h>
#include <nori/integrator.h>
#include <nori/sampler.h>
#include <nori/camera.h>
#include <nori/emitter.h>
#include <nori/dpdf.h>

NORI_NAMESPACE_BEGIN

Scene::Scene(const PropertyList &) {
    m_accel = new Accel();
}

Scene::~Scene() {
    delete m_accel;
    delete m_sampler;
    delete m_camera;
    delete m_integrator;
}

const Color3f Scene::sampleEmitter(EmitterQueryRecord &lRec, const Point2f &sample) const
{
    // 1. Muestrear el emisor concreto y obtener su radiancia emitida
    Point2f s(sample);

    // 1.1 Calcular un index aleatorio para la seleccion del emitter
    //int i = std::min((int)(m_emitters.size() * s.x()), (int)m_emitters.size() - 1);

    // 1.2 Calcular la probabilidad del emitter
    //float pdf_emitter = 1.0f / (float)m_emitters.size();

    // Parte: Muestreo de emisores con probabilidad asociada a radiancia
    float pdf_emitter;
    int index = dpdf.sampleReuse(s.x(), pdf_emitter);

    // 1.3 Seleccionar el emitter con el index aleatorio
    Emitter *e = m_emitters[index];

    // 1.4 Renormalizar el index aleatorio (comentado para muestreo de emisores con probabilidad asociada a radiancia)
    //s.x() = m_emitters.size() * s.x() - i;

    //  1.5 Dividir la radiancia ponderada por la probabilidad del emisor
    Color3f rad = e->sample(lRec, s);
    rad = rad / pdf_emitter;

    // 2. comprobar la visibilidad de la muestra generada en el emisor.Si no es visible, la radiancia a considerar es nula
    if (rayIntersect(Ray3f(lRec.ref, lRec.wi, Epsilon, (lRec.p - lRec.ref).norm() - Epsilon)))
        return Color3f(0.0f);
    
    return rad;

}

void Scene::activate() {
    m_accel->build();

    if (!m_integrator)
        throw NoriException("No integrator was specified!");
    if (!m_camera)
        throw NoriException("No camera was specified!");
    
    if (!m_sampler) {
        /* Create a default (independent) sampler */
        m_sampler = static_cast<Sampler*>(
            NoriObjectFactory::createInstance("independent", PropertyList()));
    }

    // Activar DiscretePDF de Nori
    dpdf.clear();
    dpdf.reserve(m_emitters.size());
    for (int i = 0; i < m_emitters.size(); ++i)
    {
        dpdf.append(m_emitters[i]->getLuminance());
    }
    dpdf.normalize();

    cout << endl;
    cout << "Configuration: " << toString() << endl;
    cout << endl;
}

void Scene::addChild(NoriObject *obj) {
    switch (obj->getClassType()) {
        case EMesh: {
                Mesh *mesh = static_cast<Mesh *>(obj);
                m_accel->addMesh(mesh);
                m_meshes.push_back(mesh);
                if (mesh->isEmitter())
                    m_emitters.push_back(mesh->getEmitter());
            }
            break;
        
        case EEmitter: {
                //Emitter *emitter = static_cast<Emitter *>(obj);
                /* TBD */
                m_emitters.push_back(static_cast<Emitter*>(obj));
            }
            break;

        case ESampler:
            if (m_sampler)
                throw NoriException("There can only be one sampler per scene!");
            m_sampler = static_cast<Sampler *>(obj);
            break;

        case ECamera:
            if (m_camera)
                throw NoriException("There can only be one camera per scene!");
            m_camera = static_cast<Camera *>(obj);
            break;
        
        case EIntegrator:
            if (m_integrator)
                throw NoriException("There can only be one integrator per scene!");
            m_integrator = static_cast<Integrator *>(obj);
            break;

        default:
            throw NoriException("Scene::addChild(<%s>) is not supported!",
                classTypeName(obj->getClassType()));
    }
}

std::string Scene::toString() const {
    std::string meshes;
    for (size_t i=0; i<m_meshes.size(); ++i) {
        meshes += std::string("  ") + indent(m_meshes[i]->toString(), 2);
        if (i + 1 < m_meshes.size())
            meshes += ",";
        meshes += "\n";
    }

    return tfm::format(
        "Scene[\n"
        "  integrator = %s,\n"
        "  sampler = %s\n"
        "  camera = %s,\n"
        "  meshes = {\n"
        "  %s  }\n"
        "]",
        indent(m_integrator->toString()),
        indent(m_sampler->toString()),
        indent(m_camera->toString()),
        indent(meshes, 2)
    );
}

NORI_REGISTER_CLASS(Scene, "scene");
NORI_NAMESPACE_END
