/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_MESHCLIPPING_ALGO_H
#define IVW_MESHCLIPPING_ALGO_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>

namespace inviwo {

namespace meshutil {

/**
 * Clip mesh against plane using Sutherland-Hodgman.
 * If holes should be closed, the input mesh must be manifold.
 * Vertex attributes that are interpolated and saved: tex coords and color.
 * Supported mesh types: SimpleMesh and BasicMesh.
 * Supported draw types: Triangle.
 * Supported connectivity types: Strip and None.
 * @param mesh to clip
 * @param plane in world space coordinate system
 * @param capClippedHoles: replaces removed parts with triangles aligned with the plane
 * @throws Exception if mesh is not supported, or if capClippedHoles is set, but
 * mesh is not manifold.
 * @returns SimpleMesh with connectivity None
 */
IVW_MODULE_BASE_API std::shared_ptr<Mesh> clipMeshAgainstPlane(const Mesh&, const Plane&,
                                                               bool capClippedHoles = true);

/**
 * Compute barycentric coordinates/weights for
 * point p (which is inside the polygon) with respect to polygons of vertices (v)
 * Based on Mean Value Coordinates by Hormann/Floater
 */
IVW_MODULE_BASE_API std::vector<float> barycentricInsidePolygon2D(vec2, const std::vector<vec2>&);

}  // namespace meshutil

}  // namespace inviwo

#endif  // IVW_MESHCLIPPING_ALGO_H
