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

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f& sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f& sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

Point2f Warp::squareToTent(const Point2f& sample) {

    float s = 1.f - sqrt(1.f - sample.x());
    float t = (1.f - s) * sample.y();
    return Point2f(-1.f, 0.f) + s * Point2f(2.f, 0.f) + t * Point2f(1.f, 1.f);

}

float sign(const Point2f& p1, const Point2f& p2, const Point2f& p3)
{
    return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}


float Warp::squareToTentPdf(const Point2f& p) {
    float d1, d2, d3;
    bool has_neg, has_pos;
    Point2f v1 = Point2f(-1.f, 0.f);
    Point2f v2 = Point2f(1.f, 0.f);
    Point2f v3 = Point2f(0.f, 1.f);

    d1 = sign(p, v1, v2);
    d2 = sign(p, v2, v3);
    d3 = sign(p, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos) ? 1.f : 0.f;

}

Point2f Warp::squareToUniformDisk(const Point2f& sample) {
    float r = std::sqrt(sample.x());
    float theta = 2 * M_PI * sample.y();
    return Point2f(r * std::cos(theta), r * std::sin(theta));
}

float Warp::squareToUniformDiskPdf(const Point2f& p) {
    
    return ((p.norm() >= 0) && (p.norm() <= 1)) ? INV_PI : 0.0f;

    
}

Vector3f Warp::squareToUniformSphere(const Point2f& sample) {
    throw NoriException("Warp::squareToUniformSphere() is not yet implemented!");
}

float Warp::squareToUniformSpherePdf(const Vector3f& v) {
    throw NoriException("Warp::squareToUniformSpherePdf() is not yet implemented!");
}

Vector3f Warp::squareToUniformHemisphere(const Point2f& sample) {
    float phi = sample.x() * M_PI * 2;
    float theta = acos(1 - sample.y());

    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

}

float Warp::squareToUniformHemispherePdf(const Vector3f& v) {
    return v.z() >= 0 ? INV_TWOPI : 0;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f& sample) {
    float phi = sample.x() * M_PI * 2;
    float theta = acos(sqrt(1 - sample.y()));

    return Vector3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

}

float Warp::squareToCosineHemispherePdf(const Vector3f& v) {
    return v.z() >= 0 ? INV_PI * v.z() : 0;
}

Vector3f Warp::squareToBeckmann(const Point2f& sample, float alpha) {
    float phi = sample.y() * M_PI * 2;
    float tan2theta = -alpha * alpha * log(1.f - sample.x());
    float theta = acos(1.f / sqrt(1.f + tan2theta));
    

    return sphericalDirection(theta, phi);
}

float Warp::squareToBeckmannPdf(const Vector3f& m, float alpha) {
    float theta = atan(Frame::tanTheta(m));
    float beckmann = ((exp(-pow(tan(theta), 2) / pow(alpha, 2))) / (M_PI * pow(alpha, 2) * pow(cos(theta), 4)));

   return m.z() >= 0 ? beckmann*cos(theta) : 0;
}

NORI_NAMESPACE_END