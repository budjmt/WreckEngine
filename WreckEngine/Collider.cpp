#include "Collider.h"

#include <iostream>

Collider::Collider(Transform* t, const vec3 d, const bool fudge) : Collider(Type::BOX, nullptr, t, d, fudge) { }
Collider::Collider(shared<Mesh> m, Transform* t) : Collider(Type::MESH, m, t, m->getPreciseDims()) { }
Collider::Collider(const Type type, shared<Mesh> m, Transform* t, const vec3 d, const bool fudge) : _type(type), mesh(m), _transform(t), fudgeAABB(fudge) 
{
    dims(d);
    base_aabb.center = _transform->getComputed()->position();
    transformed_aabb = base_aabb;
    updateDims();
    // the order is important;
    // edges depend on the gauss map, which depends on the normals
    genVerts();
    genNormals();
    genGaussMap();
    genEdges();
    update();
}

// makes sure the radius and AABB scale are up to date
// IMPORTANT NOTE:
// using non-uniform scaling requires the use of the inverse-transpose of the transformation matrix to transform normals properly
// additionally, these normals then must be normalized. (the rotation is changed by the scale)
// the work involved is too much to be worth it; use non-uniform colliders if and only if you know they don't depend on rotation,
// e.g. a distended cube. Otherwise, your results will be inaccurate.
void Collider::updateDims() {
    const auto scale = _transform->getComputed()->scale();
    _radius = maxf(maxf(_dims.x * scale.x, _dims.y * scale.y), _dims.z * scale.z);
    
    const auto factor = fudgeAABB ? 1.2f : 1.f;
    const auto rot = _transform->getMats()->rotate;
    transformed_aabb.halfDims =  scale * factor * base_aabb.halfDims; // scaling is applied before rotation

    // applies the transformation R * (w,h,d) == R0 * w + R1 * h + R2 * d
    transformed_aabb.halfDims = vec3(abs(rot[0]) * transformed_aabb.halfDims.x 
                                   + abs(rot[1]) * transformed_aabb.halfDims.y 
                                   + abs(rot[2]) * transformed_aabb.halfDims.z);
}

void Collider::update() {
    _framePos = _transform->getComputed()->position();
    base_aabb.center = _framePos;
    transformed_aabb.center = base_aabb.center;
    
    updateDims();
    
    //DrawDebug::get().drawDebugSphere(_framePos, _radius);
    DrawDebug::get().drawDebugBox(transformed_aabb.center, transformed_aabb.halfDims.x * 2.f, transformed_aabb.halfDims.y * 2.f, transformed_aabb.halfDims.z * 2.f);
}

// gets the vertex of the collider furthest in the direction of dir
SupportPoint Collider::getSupportPoint(const vec3 dir) {
    auto& verts = getCurrVerts();

    SupportPoint support{ verts[0], glm::dot(support.point, dir) };
    for (size_t i = 1, numVerts = verts.size(); i < numVerts; ++i) {
        const auto vert = verts[i];
        const auto proj = glm::dot(vert, dir);
        if (proj > support.proj) {
            support.point = vert;
            support.proj = proj;
        }
    }
    return support;
}

/*
----------------------------------------------------------------------
- Returns the normal and vertex on this collider with the greatest penetration into the other collider
- This is [possibly] the axis of least penetration for the collision (confusingly enough)
- The reasoning is that if the value is negative, there is penetration,
  so the greatest NEGATIVE value has the least penetration

- If the value is positive, then there is no penetration i.e. there is a separating axis
----------------------------------------------------------------------
*/
FaceManifold Collider::getAxisMinPen(Collider* other) {
    // penetration starts at -FLT_MAX
    FaceManifold axis;
    axis.originator = this;
    axis.other = other;

    auto& meshVerts = getCurrVerts();
    auto& faceVerts = mesh->indices().verts;
    auto& normals = getCurrNormals();

    for (size_t i = 0, numAxes = normals.size(); axis.pen < PEN_TOLERANCE && i < numAxes; ++i) {
        const auto norm = normals[i];
        const auto support = other->getSupportPoint(-norm);
        const auto vert = meshVerts[faceVerts[i * 3]]; // some vert from the face corresponding to the normal

        // point-plane signed distance, negative if penetrating, positive if not
        const auto pen = glm::dot(norm, support.point - vert);
        if (pen > axis.pen) {
            axis.norm = i;
            axis.pen = pen;
        }
    }

    if (axis.pen > -FLT_MAX) axis.axis = normals[axis.norm];
    return axis;
}

/*
------------------------------------------------------------------------------------------------------------------------------------------------------------------
The principles of using Gauss Maps are as follows:
- A Gauss Map is defined here as the conversion of all the face normals on a body to points on the unit sphere,
and representing adjacencies (i.e. edges) between the faces the normals refer to as [greater] arcs on the sphere.
- Practically speaking, it's a mapping of Euclidean (R^3 aka 3D) space onto the unit sphere (S^2)
- We can make a Gauss map for any given 3D surface X because the Gauss map is mathematically defined as N: X -> S^2
such that N(p) is a normal to X at point p. The only requirement is that X is orientable so we can determine its normals.

- By overlaying the gauss maps of two colliders, one can ascertain which edges on each collider will actually need to be compared for Separating Axis Theorem.

- We determine this by checking for intersecting arcs; recall that arcs represent adjacencies between faces, or in other words common edges.
  Any arcs that intersect indicate that those edges form a face on the Minkowski difference of the two colliders, and therefore their cross product is a normal that must be tested.

- We determine whether two arcs A and B intersect by performing 3 separating axis tests:
- The vertices P and Q of arc B fall on opposite sides of the plane through arc A
- The vertices R and S of arc A fall on opposite sides of the plane through arc B
- Vertices P and S are on the same side of the plane formed by Q and R (this is a hemisphere test, as the tests will fail if they are not on the same side of the sphere)
  If all 3 tests are passed, then we know the edges form a face on the Minkowski difference
- I should probably explain the Minkowski difference. Essentially, it's the "point cloud" formed by subtracting all the vertices of one body from each of the vertices from another.

- It is a useful tool for collision detection, as all the faces of both original bodies are present in its convex hull,
  in addition to faces formed by edges that may potentially be separating axes.

- It also has its own form of collision detection; it can be said if the origin is contained within the Minkowski difference, there is overlap between the bodies that form it,
  as it means at least two points in them are located at the same position in space

- For practical purposes, however, it is almost useless as assembling it is extremely expensive, (you have to form a new one each frame, for each collision check)
  which is why Gauss maps are valuable; they only need to be assembled once, as all they represent are associations and current data can be easily referenced.

- This means that we gain the benefit of not having to find a support point for every possible combination of axes on the two bodies that the Minkowski difference offers
  while not having to actually assemble one.

- Face normals can be tested as they would normally, and should be tested before overlaying the gauss maps, as it requires fewer comparisons.
  (though the necessity of the support points may offset this)
- Incidentally, overlaying Gauss maps is also referred to as checking for Voronoi region overlap.
- I recommend http://twvideo01.ubm-us.net/o1/vault/gdc2013/slides/822403Gregorius_Dirk_TheSeparatingAxisTest.pdf if you're interested in reading more on this technique,
  as this is where most of my research originates.
------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
EdgeManifold Collider::overlayGaussMaps(Collider* other) {
    EdgeManifold manifold;
    manifold.originator = this;
    manifold.other = other;

    auto& verts = getCurrVerts();
    auto& otherVerts = other->getCurrVerts();

    auto& normals = getCurrNormals();
    auto& otherNormals = other->getCurrNormals();

    auto& othergauss = other->getGaussMap();

    for (const auto& pair : gauss.adjacencies) {
        for (const auto curr : pair.second) {

            const auto a = normals[curr.faces.first], b = normals[curr.faces.second];
            const auto bxa = glm::cross(b, a);

            for (const auto& otherPair : othergauss.adjacencies) {
                for (const auto otherCurr : otherPair.second) {

                    // these must be negative to account for the Minkowski DIFFERENCE
                    // however, that's more expensive than negating on demand, so that's what we'll do
                    const auto c = otherNormals[otherCurr.faces.first], d = otherNormals[otherCurr.faces.second];

                    // checks if the arcs between arc(a,b) and arc(c,d) intersect
                    const bfloat cba{ -glm::dot(c, bxa) }
                               , dba{ -glm::dot(d, bxa) }; // negate for MD

                    // if c and d are on different sides of arc BA
                    // test with whether the signs or different or either is 0
                    // bitwise ops and int to bool ends up being faster than the multiplication
                    // if (CBA * DBA < 0) {
                    if (cba.i && dba.i && (cba.i ^ dba.i) < 0) {

                        const auto dxc = glm::cross(d, c); // the MD negations cancel out here
                        const bfloat adc{ glm::dot(a, dxc) }
                                   , bdc{ glm::dot(b, dxc) };

                        // if a and b are on different sides of arc DC &&
                        // if a and d are on the same side of the plane formed by b and c (c . (b x a) * b . (d x c) > 0)
                        // (this works because a . (b x c) == c . (b x a) and d . (b x c) == b . (d x c))(scalar triple product identity)
                        // [scalar triple product of a,b,c == [ABC] = a . (b x c)]
                        // same principle as previous
                        // if (ADC * BDC < 0 && CBA * BDC > 0) {
                        if (adc.i && bdc.i && (adc.i ^ bdc.i) < 0 && (cba.i ^ bdc.i) > 0) {
                            const auto edge      = getEdge(curr.edge), 
                                       otherEdge = other->getEdge(otherCurr.edge);

                            // if edges are parallel, we don't care since they don't define a plane
                            if (fuzzyParallel(edge, otherEdge)) continue;

                            const auto v1 = verts[curr.edge.first()],
                                       v2 = otherVerts[otherCurr.edge.first()];

                            // check distance from plane defined by edge normal and one vertex on this body's edge
                            auto edgeNormal = glm::normalize(glm::cross(edge, otherEdge));
                            edgeNormal *= signf(glm::dot(edgeNormal, v1 - _framePos)); // make sure the edge normal is facing outwards from the body

                            const auto pen = glm::dot(edgeNormal, v2 - v1); // does this work regardless of the edges' points used?
                            if (pen > manifold.pen) {
                                manifold.edgePair = { curr, otherCurr };
                                manifold.pen = pen;
                                manifold.axis = edgeNormal;
                                // we found a separating axis boys
                                if (manifold.pen > PEN_TOLERANCE) return manifold;
                            }
                        }

                    } // end edge culling

                }
            } // end other gauss loop

        }
    } // end gauss loop
    return manifold;
}

// Colliders assume their meshes are centered at the origin; if they aren't there will be inaccuracy
// we return a valid manifold, or if there was no collision one with a nullptr originator
Manifold Collider::intersects(Collider* other) {

    // quick sphere collision optimization
    const auto d = _framePos - other->framePos(); // ignores displaced colliders
    const auto distSq = dot(d, d);
    const auto rad = _radius + other->radius(); // should be using AABBs
    if (distSq > rad * rad)
        return Manifold();

    // separating axis theorem 2: electric boogaloo
    // axis of min pen on this collider
    auto minAxis = getAxisMinPen(other);
    if (minAxis.pen > PEN_TOLERANCE) {
        //std::cout << "This: " << minAxis.pen << "; " << minAxis.axis.x << ", " << minAxis.axis.y << ", " << minAxis.axis.z << '\n';
        return Manifold();
    }

    // axis of min pen on other collider
    auto otherMinAxis = other->getAxisMinPen(this);
    if (otherMinAxis.pen > PEN_TOLERANCE) {
        //std::cout << "Other: " << otherMinAxis.pen << "; " << otherMinAxis.axis.x << ", " << otherMinAxis.axis.y << ", " << otherMinAxis.axis.z << '\n';
        return Manifold();
    }

    // closest penetrating edges on both colliders
    auto minEdge = overlayGaussMaps(other);
    if (minEdge.pen > PEN_TOLERANCE) {
        // debug code
        const auto  v1  = getVert(minEdge.edgePair.first.edge.first())
                  , v2  = getVert(minEdge.edgePair.first.edge.second())
                  , ov1 = other->getVert(minEdge.edgePair.second.edge.first())
                  , ov2 = other->getVert(minEdge.edgePair.second.edge.second());
        DrawDebug::get().drawDebugVector(v1, v2, vec3(1, 0, 0));
        DrawDebug::get().drawDebugVector(ov1, ov2, vec3(1, 1, 0));
        //std::cout << "Edge: " << minEdge.pen << '\n';
        return Manifold();
    }

    auto& minFace = (minAxis.pen > otherMinAxis.pen) ? minAxis : otherMinAxis;

    // edge-edge collision
    if (minEdge.pen > minFace.pen) {
        std::cout << "EDGE ";
        // get the points defining both edges in the collision in world space
        const auto p0 = getVert(minEdge.edgePair.first.edge.first())
                 , p1 = getVert(minEdge.edgePair.first.edge.second())
                 , q0 = other->getVert(minEdge.edgePair.second.edge.first())
                 , q1 = other->getVert(minEdge.edgePair.second.edge.second());

        // find the closest point between the two edges
        minEdge.colPoints.push_back(closestPointBtwnSegments(p0, p1, q0, q1));
        DrawDebug::get().drawDebugSphere(minEdge.colPoints[0], 0.2f, vec3(0,1,0), 0.8f);
        return minEdge;
    }
    // face-* collision
    else {
        assert(minFace.axis != vec3()); // Axis has not been set, collision probably got through with NaN errors

        // find the possible incident faces; the axis is the reference normal
        const auto incidents = minFace.other->getIncidentFaces(minFace.axis);

        // clip the incident face(s) against the reference face
        minFace.originator->clipPolygons(minFace, incidents);

        for (const auto& colPoint : minFace.colPoints)
            DrawDebug::get().drawDebugSphere(colPoint, 0.1f, vec3(1,0,0), 0.8f);

        return minFace;
    }
}

// Gets the indices of the faces on this body that are most anti-parallel to the reference normal
std::vector<GLuint> Collider::getIncidentFaces(const vec3 refNormal) {
    std::vector<GLuint> faces;
    auto& normals = getCurrNormals();

    auto antiProj = glm::dot(normals[0], refNormal);
    faces.push_back(0);
    for (size_t i = 1, numFaces = normals.size(); i < numFaces; ++i) {
        // if + -, diff > 0; if - +, diff < 0; if same sign, magnitude of projection determines sign
        const auto proj = glm::dot(normals[i], refNormal);
        const auto diff = antiProj - proj;

        // if the face has close to 0 difference to the last anti-normal projection, then it's another incident face
        if (epsCheck(diff)) {
            faces.push_back(i);
        }
        // if the face is more anti-parallel than the previous, we replace our previous incident faces with this one
        else if (diff > 0) {
            faces.clear();
            faces.push_back(i);
            antiProj = proj;
        }
    }
    return faces;
}

/*
------------------------------------------------------------------------------------------------------------------------------------
- Finds collision points based on Sutherland-Hodgman Clipping
- For each possible incident face on the other collider
- Clip the vertices against the side planes of the reference face on this collider
- Reject results that fall outside, include intersections and points inside
- Add final results to the set of collision points

- http://gamedevelopment.tutsplus.com/tutorials/understanding-sutherland-hodgman-clipping-for-physics-engines--gamedev-11917
------------------------------------------------------------------------------------------------------------------------------------
*/
void Collider::clipPolygons(FaceManifold& reference, const std::vector<GLuint>& incidents) {

    // get the transformed center point of the reference face
    auto& vertFaces = mesh->indices().verts;
    const auto index = reference.norm * 3;
    auto refCenter = getVert(vertFaces[index]) + getVert(vertFaces[index + 1]) + getVert(vertFaces[index + 2]);
    refCenter /= 3.f;

    struct plane { vec3 normal, vert; };
    plane refFace{ reference.axis, refCenter };

    //DrawDebug::get().drawDebugVector(refCenter, refCenter + refNorm);

    // get the side planes of the reference face
    // These are supposed to be the normals of the faces adjacent to the reference face, at least according to Bullet
    // I use the actual side planes of the face, 
    // i.e. normals from the edges perpendicular to the edge and face normal facing outward from the face's center
    std::vector<plane> sidePlanes;
    sidePlanes.reserve(3);

    for (auto i = 0; i < 3; ++i) {
        auto& v = vertFaces[reference.norm * 3 + i];
        const auto vert = getVert(v)
                 , edge = getEdge({ v, vertFaces[reference.norm * 3 + (i + 1) % 3] });

        auto norm = glm::cross(reference.axis, edge);
        norm *= signf(glm::dot(norm, vert - refCenter));

        sidePlanes.push_back({ norm, vert });

        //DrawDebug::get().drawDebugVector(vert, vert + edge, vec3(1, 1, 0));
        //DrawDebug::get().drawDebugVector(vert, vert + norm, vec3(1, 0, 1));
    }

    for (const auto incidentFace : incidents) {

        std::vector<vec3> incidentVerts = {
            reference.other->getVert(reference.other->getFaceVert(incidentFace * 3)),
            reference.other->getVert(reference.other->getFaceVert(incidentFace * 3 + 1)),
            reference.other->getVert(reference.other->getFaceVert(incidentFace * 3 + 2))
        };

        auto clipped = incidentVerts;
        for (auto s = 0; s < 3 && clipped.size(); ++s) {
            auto& plane = sidePlanes[s];
            clipped = clipPolyAgainstEdge(clipped, plane.normal, plane.vert, refFace.normal, refFace.vert);
        }

        reference.colPoints.reserve(reference.colPoints.size() + clipped.size());
        reference.colPoints.insert(end(reference.colPoints), begin(clipped), end(clipped));
    }
}

/*
------------------------------------------------------------------------------------------------------------------------------------
- Takes an input of a set of vertices, clips them against a single edge of the reference face (a side plane) defined by sidePlane (the normal) and sideVert (point on plane)

- Culls those that don't fall below the reference face, defined by refNorm and refCenter (normal and point on plane respectively)
- Clipping has 4 cases to determine which vertices are kept, going through sets of start and end points:
- Start and End both inside the reference face (here defined as behind it, since these are side planes facing outward): we keep only the end pt
- The reason we keep the end point and not the start point is because this algorithm ensures that all start points will be end points at one point, and vice versa
- This means enclosed start points will be added at some point no matter what, so we only worry about end points
- This case is more important for telling us there will be no intersection between the plane and the edge between start and end, so we don't need to add it

- Start and End both outside: we keep nothing, as nothing falls within the clip polygon

- Start inside and End outside: we keep the intersection between the edge and the side plane
- Because the points are on different side of the plane, there must be an intersection with the plane

- Start outside and End inside: we keep both the intersection and the end pt
- Because of how the first case actually works (telling us there is no intersection), this case and the first can be condensed together
- All we're saying is that since the start pt is NOT inside and the end pt IS, there must be an intersection, so we have to add that on top of the end pt
- Another way of describing these cases is to base it on the positioning of the end pt
- If the end pt is inside, we'll be keeping the end pt, otherwise we won't since it falls outside
- If the start pt is on the opposite side of the plane of the end pt, we also keep the intersection between the plane and the edge between the two points

- These 2 simpler cases with possible overlap lead to the more complex explanation and the original 4 cases

- This is the version I use; it's less for optimization and more for reducing obfuscation, as the meaning here is easier to remember than the original 4 cases
- I should mention that it isn't really any less efficient than the original version; it adds a bitwise XOR between startInside and endInside, but that's extremely negligible
- It's also less code, as the intersection code doesn't need to be duplicated

- There is one additional case that occurs for our purposes to account for floating point error
- Either the Start or End falls ON the side plane: we keep the end pt
- We define this as the dot product of the side plane normal and the vector from the plane to either vertex being within -FLT_EPSILON and FLT_EPSILON
- We call this using thick planes, rather than planes of indeterminably small thickness which is what we normally use
------------------------------------------------------------------------------------------------------------------------------------
*/
std::vector<vec3> Collider::clipPolyAgainstEdge(std::vector<vec3>& input, const vec3 sideNormal, const vec3 sideVert, const vec3 refNorm, const vec3 refCenter) const {
    std::vector<vec3> output;

    // regular conditions protect against this, but just to be safe
    if (input.empty()) return output;

    vec3 startpt = input.back(), endpt;
    for (size_t i = 0, numInputs = input.size(); i < numInputs; ++i, startpt = endpt) {
        endpt = input[i];

        const auto clipStart = glm::dot(sideNormal, startpt - sideVert);
        const auto clipEnd   = glm::dot(sideNormal, endpt - sideVert);

        // the edge is "on the plane" (thick planes); keep end pt if it falls below reference face
        if (epsCheck(clipStart) || epsCheck(clipEnd)) {
            if (glm::dot(refNorm, endpt - refCenter) < 0) output.push_back(endpt);
            continue;
        }

        bool startInside = clipStart < 0, endInside = clipEnd < 0; // if they fall behind the side plane, they're inside the clip polygon

        // end pt is inside, keep it if it falls below the reference face
        if (endInside) {
            if (glm::dot(refNorm, endpt - refCenter) < 0) output.push_back(endpt);
        }

        // start pt and end pt fall on different sides of the plane, keep intersection if it falls below the reference face
        if (startInside != endInside) {
            // find intersection
            const auto e = glm::dot(sideNormal, startpt - endpt);
            const auto t = (e) ? clipStart / e : 0; // e is 0 if start == end or sideNormal == the normal of the clipped face
            //float t = clipStart / (clipStart - clipEnd);
            const auto intersect = glm::mix(startpt, endpt, t);
            if (glm::dot(refNorm, intersect - refCenter) < 0) output.push_back(intersect);
        }
    }

    return output;
}

/*
------------------------------------------------------------------------------------------------------------------------------------
- Finds the closest point between two line segments

- For such a seemingly simple task, the solution is quite complex, and requires a bit of explanation
- Too much for here, I wrote a long post on the topic on my blog: *insert link*
- You can also find a PDF copy in the proofs folder in the root of the repository (pending)
- As an alternative, my main source was http://geomalgorithms.com/a07-_distance.html

- The general thought process is that you start with the problem of finding an intersection between two lines
- The lines are defined by p = { p0, u = p1 - p0 } and q = { q0, v = q1 - q0 }
- This uses the parametric equations for the lines and the vector between them, w.
- The smallest w is defined as w(Sc, Tc) or Wc
  - Sc = (b*e - c*d) / (a*c - b^2)
  - Tc = (a*e - b*d) / (a*c - b^2)
- You can find the values for a,b,c,d,e in the code below
- If a*c - b^2 = 0, then the lines are parallel and Sc = 0, Tc = e / c
- The closest point to q on p is p0 + Sc * u
- The closest point to p on q is q0 + Tc * v
- To get the closest point in space we just compute p0 - Wc / 2 or q0 + Wc / 2

- The line problem isn't complex enough to account for the fact that these are segments
- The closest point between the two lines may not occur within the 0 to 1 range dictated by their segments
- We account for this ourselves by doing some range checks
- When Sc or Tc goes outside the 0 to 1 range, we have to change how we're solving for the other value
  - if Sc < 0, Tc =  e / c. if Sc > 1, Tc = ( e + b) / c
  - if Tc < 0, Sc = -d / a. if Tc > 1, Sc = (-d + b) / a
- After accounting for these, just do a basic clamp on these values to the 0 to 1 range
- We can save computations by just storing the numerators and denominators and doing the range checks with them
- That way there's only two divisions (Sc and Tc) at the end
------------------------------------------------------------------------------------------------------------------------------------
*/
vec3 Collider::closestPointBtwnSegments(const vec3 p0, const vec3 p1, const vec3 q0, const vec3 q1) const {
    auto u = p1 - p0, v = q1 - q0, w0 = p0 - q0;

    const auto a = dot(u, u)  // |u|^2
             , b = dot(u, v)  // projection of u/v onto v/u == |u||v|cos(theta)
             , c = dot(v, v)  // |v|^2
             , d = dot(u, w0) // projection of u onto w0
             , e = dot(v, w0) // projection of v onto w0
             , D = a * c - b * b; // equivalent to ac * sin^2(theta); if parallel == 0, if perpendicular == ac

    float sNumer, sDenom = D;
    float tNumer, tDenom = D;

    // if D ~ 0, i.e. segments are parallel
    if (D < FLT_EPSILON) {
        sNumer = 0; sDenom = 1;
        tNumer = e; tDenom = c;
    }
    else {
        sNumer = b * e - c * d;
        tNumer = a * e - b * d;

        // if Sc < 0, Sc = 0 and Tc = e / c
        if (sNumer < 0) {
            sNumer = 0;
            tNumer = e; tDenom = c;
        }
        // if Sc > 1, Sc = 1 and Tc = (e + b) / c
        else if (sNumer > sDenom) {
            sNumer = sDenom;
            tNumer = e + b; tDenom = c;
        }
    }

    // if Tc < 0, Tc = 0 and Sc = -d / a
    if (tNumer < 0) {
        tNumer = 0;
        // if Sc < 0, Sc = 0; Tc = 0
        if (-d < 0)
            sNumer = 0;
        // if Sc > 1, Sc = 1; Tc = 0
        else if (-d > a)
            sNumer = sDenom;
        else {
            sNumer = -d; sDenom = a;
        }
    }
    // if Tc > 1, Tc = 1 and Sc = (-d + b) / a
    else if (tNumer > tDenom) {
        tNumer = tDenom;
        // if Sc < 0, Sc = 0; Tc = 1
        if (-d + b < 0)
            sNumer = 0;
        // if Sc > 1, Sc = 1; Tc = 1
        else if (-d + b > a)
            sNumer = sDenom;
        else {
            sNumer = -d + b; sDenom = a;
        }
    }

    // prevents possible divide by zero
    float sc = epsCheck(sNumer) ? 0 : sNumer / sDenom;
    float tc = epsCheck(tNumer) ? 0 : tNumer / tDenom;

    v *= tc;
    auto wc = (sc * u) + (w0 - v); // the vector between the 2 closest points on the 2 segments (W_c = w0 + (Sc * u) - (Tc * v))
    return (wc * 0.5f) + (q0 + v); // the closest point between the 2 segments in the world (q0 + (Tc * v) + W_c * 0.5f)
}

void Collider::genVerts() {
    if (_type == Type::BOX) {
        Mesh::FaceData data;//norms and UVs are empty
        Mesh::FaceIndex indices;
        data.verts = { {  _dims.x,  _dims.y,  _dims.z },
                       { -_dims.x,  _dims.y,  _dims.z },
                       {  _dims.x, -_dims.y,  _dims.z },
                       { -_dims.x, -_dims.y,  _dims.z },
                       {  _dims.x,  _dims.y, -_dims.z },
                       { -_dims.x,  _dims.y, -_dims.z },
                       {  _dims.x, -_dims.y, -_dims.z },
                       { -_dims.x, -_dims.y, -_dims.z } };

        indices.verts = { 7, 3, 0, 7, 4, 0,
                        6, 7, 4, 6, 5, 4,
                        2, 6, 5, 2, 1, 5,
                        3, 2, 1, 3, 0, 1,
                        0, 1, 5, 0, 4, 5,
                        7, 6, 2, 7, 3, 2 };

        mesh = make_shared<Mesh>(data, indices);
    }
    currVerts->clear();
    currVerts->resize(mesh->data().verts.size());
}

void Collider::genNormals() {
    switch (_type) {
    case Type::SPHERE:
        break;
    case Type::BOX:
        faceNormals = { vec3( 1,  0,  0),
                        vec3(-1,  0,  0),
                        vec3( 0,  0,  1),
                        vec3( 0,  0, -1),
                        vec3( 0,  1,  0),
                        vec3( 0, -1,  0) };
        break;
    case Type::MESH:
        // generate the face normals from the mesh's vertices
        // when iterating over normals, to retrieve the vertices of the face corresponding to the normal at index i,
        // the nth (0, 1, or 2) vertex in the face is meshVerts[faceVerts[i * 3 + n]]
        // alternatively, if you aren't getting meshVerts, use the function getVert(faceVerts[i * 3 + n])
        auto& faceVerts = mesh->indices().verts;
        auto& meshVerts = mesh->data().verts;
        for (size_t i = 0, numFaces = faceVerts.size(); i < numFaces; i += 3) {
            const auto v = meshVerts[faceVerts[i]];

            const auto e1 = meshVerts[faceVerts[i + 1]] - v;
            const auto e2 = meshVerts[faceVerts[i + 2]] - v;

            faceNormals.push_back(normalize(cross(e1, e2)));
        }
        break;
    }
    currNormals->clear();
    currNormals->resize(faceNormals.size());
}

void Collider::genEdges() {
    switch (_type) {
    case Type::SPHERE:
        break;
    case Type::BOX:
    case Type::MESH:
        auto& meshVerts = mesh->data().verts;
        for (const auto& pair : gauss.adjacencies) {
            for (const auto& adj : pair.second) {
                setEdge(adj.edge, edges.size());
                edges.push_back(meshVerts[adj.edge.second()] - meshVerts[adj.edge.first()]);
            }
        }
        break;
    }
    currEdges->clear();
    currEdges->resize(edges.size());
}

void Collider::genGaussMap() {
    switch (_type) {
    case Type::SPHERE:
        break;
    case Type::BOX:
        // need to set up proper handling for box colliders for vertices
        //gauss.addAdj(faceNormals[0], Adj{  });
        //gauss.adjacencies[faceNormals[1]] = { Adj{ 2, {,} } };
        break;
    case Type::MESH:
        // set up the edge associations
        auto& faceVerts = mesh->indices().verts;
        //adjacencies = std::vector<std::vector<Adj>>(numFaces / 3, std::vector<Adj>());
        for (size_t i = 0, numFaces = faceVerts.size(); i < numFaces; i += 3) {
            for (auto j = i + 3; j < numFaces; j += 3) {
                // if (fuzzyParallelUnit(faceNormals[i / 3], faceNormals[j / 3])) continue;

                auto src = -1;
                auto added = false;
                for (size_t p1 = 0; !added && p1 < 3; ++p1) {
                    for (size_t p2 = 0; p2 < 3; ++p2) {
                        // continue if there's no match
                        if (faceVerts[i + p1] != faceVerts[j + p2]) continue;
                        
                        // if a match hasn't been found yet, just record it
                        if (src < 0) src = faceVerts[i + p1];
                        // otherwise, record it, set the normal, push it back, and end the loop
                        else {
                            const GLuint usrc = src;
                            const auto dst = faceVerts[i + p1];
                            Adj adj{ { i/3, j/3 }, { usrc, dst } };
                            gauss.addAdj(faceNormals[adj.faces.first], adj);
                            added = true;
                        }
                        // none of the other vertices can be equal to this one now, so move to the next one
                        break;
                    }
                } // end edge loop

            }
        } // end face loop
        break;
    }
}

void Collider::updateVerts(std::vector<vec3>& currVerts) {
    switch (_type) {
    case Type::SPHERE:
        break;
    case Type::BOX:
    case Type::MESH:
        const auto world = _transform->getMats()->world;
        auto& verts = mesh->data().verts;
        for (size_t i = 0, numVerts = verts.size(); i < numVerts; ++i) {
            currVerts[i] = (vec3)(world * vec4(verts[i], 1));
        }
        break;
    }
}

void Collider::updateNormals(std::vector<vec3>& currNormals) {
    switch (_type) {
    case Type::SPHERE:
        break;
    case Type::BOX:
    case Type::MESH:
        const auto rot = _transform->getMats()->rotate;
        //auto& faceVerts = mesh->faces().verts;
        for (size_t i = 0, numNormals = faceNormals.size(); i < numNormals; ++i) {
            currNormals[i] = (vec3)(rot * vec4(faceNormals[i], 0));

            //const auto  a = getVert(faceVerts[i * 3])
            //          , b = getVert(faceVerts[i * 3 + 1])
            //          , c = getVert(faceVerts[i * 3 + 2]);
            //auto center = a + b + c;
            //center /= 3;
            //DrawDebug::get().drawDebugVector(center, center + currNormals[i]);
        }
        break;
    }
}

void Collider::updateEdges(std::vector<vec3>& currEdges) {
    switch (_type) {
    case Type::SPHERE:
        break;
    case Type::BOX:
    case Type::MESH:
        const auto world = _transform->getMats()->world;
        for (size_t i = 0, numEdges = edges.size(); i < numEdges; ++i) {
            // we only care about rotating and scaling the edge, translation is discarded
            currEdges[i] = (vec3)(world * vec4(edges[i], 0));
        }
        
        /*for (auto& pair : gauss.adjacencies) {
            for (auto adj : pair.second) {
                auto s = getVert(adj.edge[0]);
                auto edge = getEdge(adj.edge);
                DrawDebug::get().drawDebugVector(s, s + edge);
            }
        }*/
        break;
    }
}

const std::vector<vec3>& Collider::getCurrVerts() { return currVerts(); }
const std::vector<vec3>& Collider::getCurrNormals() { return currNormals(); }
const std::vector<vec3>& Collider::getCurrEdges() { return currEdges(); }

GLuint Collider::getFaceVert(GLuint index) const { return mesh->indices().verts[index]; }
vec3 Collider::getVert(GLuint index) { return currVerts()[index]; }
vec3 Collider::getNormal(GLuint index) { return currNormals()[index]; }
vec3 Collider::getEdge(Edge e) { return currEdges()[edgeMap.at(e)]; }
void Collider::setEdge(Edge e, const GLuint index) { edgeMap[e] = index; }

const GaussMap& Collider::getGaussMap() const { return gauss; }

const std::vector<Adj>& GaussMap::getAdjs(vec3 v) const { return adjacencies.at(v); }
void GaussMap::addAdj(vec3 v, Adj a) { adjacencies[v].push_back(a); }

bool AABB::intersects(const AABB& other) const {
    auto back  = center - halfDims, otherBack  = other.center - other.halfDims;
    auto front = center + halfDims, otherFront = other.center + other.halfDims;
    auto xSeparate = (back.x > otherFront.x) || (front.x < otherBack.x);
    auto ySeparate = (back.y > otherFront.y) || (front.y < otherBack.y);
    auto zSeparate = (back.z > otherFront.z) || (front.z < otherBack.z);
    return !(xSeparate || ySeparate || zSeparate);
}