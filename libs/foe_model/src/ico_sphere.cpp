// Copyright (C) 2020 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/model/ico_sphere.hpp>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "model_log.hpp"

#include <cmath>
#include <map>
#include <tuple>

namespace {

// IcoSphere code derived from origin by `Andreas Kahler` @
// http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

uint32_t getMiddlePoint(uint32_t index1,
                        uint32_t index2,
                        std::map<std::tuple<uint32_t, uint32_t>, uint32_t> &middlePointCache,
                        std::vector<glm::vec3> &vertices) {
    // Check cache
    auto searchIt = middlePointCache.find(std::make_tuple(index1, index2));
    if (searchIt != middlePointCache.end()) {
        return searchIt->second;
    }
    searchIt = middlePointCache.find(std::make_tuple(index2, index1));
    if (searchIt != middlePointCache.end()) {
        return searchIt->second;
    }

    // Generate new point
    glm::vec3 point1 = vertices[index1];
    glm::vec3 point2 = vertices[index2];

    glm::vec3 middle = point1 + point2;
    middle /= 2.0;

    uint32_t newIndex = vertices.size();
    vertices.emplace_back(glm::normalize(middle));
    middlePointCache[std::make_tuple(index1, index2)] = newIndex;
    return newIndex;
}

glm::vec3 calculateTangent(glm::vec3 const &normal) {
    glm::vec3 c1 = glm::cross(normal, glm::vec3{0, 0, 1});
    glm::vec3 c2 = glm::cross(normal, glm::vec3{0, 1, 0});

    if (glm::length(c1) > glm::length(c2)) {
        return glm::normalize(c1);
    } else {
        return glm::normalize(c2);
    }
}

glm::vec3 calculateBitangent(glm::vec3 const &normal) {
    return glm::normalize(glm::cross(normal, calculateTangent(normal)));
}

auto createIcoSphere(int recursion) -> std::tuple<std::vector<glm::vec3>, std::vector<glm::uvec3>> {
    std::vector<glm::vec3> vertices;
    std::vector<glm::uvec3> faces;

    std::map<std::tuple<uint32_t, uint32_t>, uint32_t> middlePointCache;

    vertices.reserve(12);
    faces.reserve(20);

    { // Create initial 12 vertices
        double distance = (1.0 + sqrt(5.0)) / 2.0;

        vertices.emplace_back(glm::normalize(glm::vec3(-1, distance, 0)));
        vertices.emplace_back(glm::normalize(glm::vec3(1, distance, 0)));
        vertices.emplace_back(glm::normalize(glm::vec3(-1, -distance, 0)));
        vertices.emplace_back(glm::normalize(glm::vec3(1, -distance, 0)));

        vertices.emplace_back(glm::normalize(glm::vec3(0, -1, distance)));
        vertices.emplace_back(glm::normalize(glm::vec3(0, 1, distance)));
        vertices.emplace_back(glm::normalize(glm::vec3(0, -1, -distance)));
        vertices.emplace_back(glm::normalize(glm::vec3(0, 1, -distance)));

        vertices.emplace_back(glm::normalize(glm::vec3(distance, 0, -1)));
        vertices.emplace_back(glm::normalize(glm::vec3(distance, 0, 1)));
        vertices.emplace_back(glm::normalize(glm::vec3(-distance, 0, -1)));
        vertices.emplace_back(glm::normalize(glm::vec3(-distance, 0, 1)));
    }

    { // Create 20 base triangles
      // 5 faces around point 0
        faces.emplace_back(0, 11, 5);
        faces.emplace_back(0, 5, 1);
        faces.emplace_back(0, 1, 7);
        faces.emplace_back(0, 7, 10);
        faces.emplace_back(0, 10, 11);

        // 5 adjacent faces
        faces.emplace_back(1, 5, 9);
        faces.emplace_back(5, 11, 4);
        faces.emplace_back(11, 10, 2);
        faces.emplace_back(10, 7, 6);
        faces.emplace_back(7, 1, 8);

        // 5 faces around point 3
        faces.emplace_back(3, 9, 4);
        faces.emplace_back(3, 4, 2);
        faces.emplace_back(3, 2, 6);
        faces.emplace_back(3, 6, 8);
        faces.emplace_back(3, 8, 9);

        // 5 adjacent faces
        faces.emplace_back(4, 9, 5);
        faces.emplace_back(2, 4, 11);
        faces.emplace_back(6, 2, 10);
        faces.emplace_back(8, 6, 7);
        faces.emplace_back(9, 8, 1);
    }

    // Refine the triangles
    for (int i = 0; i < recursion; ++i) {
        faces.reserve(faces.size() * 4);

        auto endTri = faces.end();
        for (auto tri = faces.begin(); tri != endTri; ++tri) {
            // Subdivide the original triangle
            uint32_t a = getMiddlePoint(tri->x, tri->y, middlePointCache, vertices);
            uint32_t b = getMiddlePoint(tri->y, tri->z, middlePointCache, vertices);
            uint32_t c = getMiddlePoint(tri->z, tri->x, middlePointCache, vertices);

            faces.emplace_back(tri->x, a, c);
            faces.emplace_back(tri->y, b, a);
            faces.emplace_back(tri->z, c, b);
            // The current triangle is updated, the old one has been replaced with the four
            // subdivided ones instead
            *tri = glm::uvec3{a, b, c};
        }
    }

    return std::make_tuple(vertices, faces);
}

template <typename T>
auto sphereVertexData(int recursion,
                      uint32_t componentCount,
                      foeVertexComponent const *pComponents,
                      T *pData) {
    auto [vertices, faces] = createIcoSphere(recursion);

    for (auto &vertex : vertices) {
        for (uint32_t c = 0; c < componentCount; ++c) {

            switch (pComponents[c]) {
            case foeVertexComponent::Position:
            case foeVertexComponent::Normal:
                *pData++ = vertex.x;
                *pData++ = vertex.y;
                *pData++ = vertex.z;
                break;

            case foeVertexComponent::UV:
                // Need to calculate the UV coordinates, where X/Z determine the U, and Y determines
                // the V
                {
                    constexpr float cOverPi = 1.0 / glm::pi<float>();
                    float u = 0.5 - 0.5 * glm::atan(vertex.x, -vertex.z) * cOverPi;
                    float v = 1.0 - acos(vertex.y) * cOverPi;

                    *pData++ = u;
                    *pData++ = v;
                }
                break;

            case foeVertexComponent::Tangent: {
                auto tangent = calculateTangent(vertex);
                *pData++ = tangent.x;
                *pData++ = tangent.y;
                *pData++ = tangent.z;
            } break;

            case foeVertexComponent::Bitangent: {
                auto tangent = calculateBitangent(vertex);
                *pData++ = tangent.x;
                *pData++ = tangent.y;
                *pData++ = tangent.z;
            } break;

            case foeVertexComponent::Colour:
                FOE_LOG(Model, FOE_LOG_LEVEL_FATAL,
                        "Tried to generate cube vertex data with an unsupported colour component");
                std::abort();
            }
        }
    }
}

template <typename T>
void sphereIndexData(int recursion, T offset, T *pData) {
    auto [vertices, faces] = createIcoSphere(recursion);

    for (auto &face : faces) {
        *pData++ = offset + face.x;
        *pData++ = offset + face.y;
        *pData++ = offset + face.z;
    }
}

} // namespace

void foeModelIcoSphereNums(int recursion, int *pNumVertices, int *pNumIndices) {
    auto [vertices, faces] = createIcoSphere(recursion);

    *pNumVertices = vertices.size();
    *pNumIndices = faces.size() * 3;
}

int foeModelIcoSphereNumVertices(int recursion) noexcept {
    auto [vertices, faces] = createIcoSphere(recursion);

    return vertices.size();
}

void foeModelIcoSphereVertexData(int recursion,
                                 uint32_t componentCount,
                                 foeVertexComponent const *pComponents,
                                 float *pData) {
    ::sphereVertexData<float>(recursion, componentCount, pComponents, pData);
}

int foeModelIcoSphereNumIndices(int recursion) noexcept {
    auto [vertices, faces] = createIcoSphere(recursion);

    return faces.size() * 3;
}

void foeModelIcoSphereIndexData16(int recursion, uint16_t offset, uint16_t *pData) {
    ::sphereIndexData<uint16_t>(recursion, offset, pData);
}

void foeModelIcoSphereIndexData32(int recursion, uint32_t offset, uint32_t *pData) {
    ::sphereIndexData<uint32_t>(recursion, offset, pData);
}