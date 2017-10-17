#include "Collider.h"

#include <iostream>

Collider::Collider(Transform* t, const vec3 d, const bool fudge) : Collider(Type::BOX, nullptr, t, d, fudge) { }
Collider::Collider(shared<Mesh> m, Transform* t) : Collider(Type::MESH, m, t, m->getPreciseDims()) { }
Collider::Collider(const Type type, shared<Mesh> m, Transform* t, const vec3 d, const bool fudge) : _type(type), mesh(m), _transform(t), fudgeAABB(fudge) 
{
	dims(d);
	base_aabb.center = _transform->getComputed()->position;
	transformed_aabb = base_aabb;
	updateDims();
	//the order is important;
	//edges depend on the gauss map, which depends on the normals
	genNormals();
	genGaussMap();
	genEdges();
	update();
}

//makes sure the radius is up to date
// IMPORTANT NOTE:
// using non-uniform scaling requires the use of the inverse-transpose of the transformation matrix to transform normals properly
// additionally, these normals then must be normalized. (the rotation is changed by the scale)
// the work involved is too much to be worth it; use non-uniform colliders if and only if you know they don't depend on rotation,
// e.g. a distended cube. Otherwise, your results will be inaccurate.
void Collider::updateDims() {
	const auto scale = _transform->getComputed()->scale();
	_radius = maxf(maxf(_dims.x * scale.x, _dims.y * scale.y), _dims.z * scale.z);
	
	const auto factor = fudgeAABB ? 1.2f : 1.f;
	transformed_aabb.halfDims = scale * factor * base_aabb.halfDims;
}

void Collider::update() {
	_framePos = _transform->getComputed()->position;
	base_aabb.center = _framePos;
	transformed_aabb.center = base_aabb.center;
	
	updateDims();
	updateNormals();
	updateEdges();
	
	//DrawDebug::getInstance().drawDebugSphere(_framePos, _radius);
	DrawDebug::getInstance().drawDebugBox(transformed_aabb.center, transformed_aabb.halfDims.x * 2.f, transformed_aabb.halfDims.y * 2.f, transformed_aabb.halfDims.z * 2.f);
}

//gets the vertex of the collider furthest in the direction of dir
SupportPoint Collider::getSupportPoint(const vec3 dir) const {
	auto& verts = mesh->data().verts;
	const auto world = _transform->getMats()->world;

	SupportPoint support{ (vec3)(world * vec4(verts[0], 1)), glm::dot(support.point, dir) };
	for (size_t i = 1, numVerts = verts.size(); i < numVerts; ++i) {
		const auto vert = (vec3)(world * vec4(verts[i], 1));
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

	auto& meshVerts = mesh->data().verts;
	auto& faceVerts = mesh->indices().verts;
	const auto world = _transform->getMats()->world;
	for (size_t i = 0, numAxes = currNormals.size(); axis.pen < COLLISION_PEN_TOLERANCE && i < numAxes; ++i) {
		const auto norm = currNormals[i];
		const auto support = other->getSupportPoint(-norm);
		const auto vert = (vec3)(world * vec4(meshVerts[faceVerts[i * FLOATS_PER_VERT]], 1));

		const auto pen = glm::dot(norm, support.point - vert);//point-plane signed distance, negative if penetrating, positive if not
		if (pen > axis.pen) {
			axis.norm = i;
			axis.pen = pen;
		}
	}

	if (axis.pen > -FLT_MAX) axis.axis = currNormals[axis.norm];
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
	//manifold.pen = -FLT_MAX;//initial value

	auto& othergauss = other->getGaussMap();
	auto& otherNormals = other->getCurrNormals();

	auto trans = _transform->getComputed();
	auto otherTrans = other->transform()->getComputed();

	for (const auto& pair : gauss.adjacencies) {
		for (const auto curr : pair.second) {

			const auto a = currNormals[curr.faces.first], b = currNormals[curr.faces.second];
			const auto bxa = glm::cross(b, a);

			for (const auto& otherPair : othergauss.adjacencies) {
				for (const auto otherCurr : otherPair.second) {

					//these must be negative to account for the Minkowski DIFFERENCE
					//however, that's more expensive than negating on demand, so that's what we'll do
					const auto c = otherNormals[otherCurr.faces.first], d = otherNormals[otherCurr.faces.second];

					//checks if the arcs between arc(a,b) and arc(c,d) intersect
					const bfloat cba{ -glm::dot(c, bxa) }
					           , dba{ -glm::dot(d, bxa) };// negate for MD

					//if c and d are on different sides of arc BA
					//test with whether the signs or different or either is 0
					//bitwise ops and int to bool ends up being faster than the multiplication
					//if (CBA * DBA < 0) {
					if (cba.i && dba.i && (cba.i ^ dba.i) < 0) {

						const auto dxc = glm::cross(d, c);// here the negative signs cancel out, so we're OK
						const bfloat adc{ glm::dot(a, dxc) }
						           , bdc{ glm::dot(b, dxc) };

						//if a and b are on different sides of arc DC &&
						//if a and d are on the same side of the plane formed by b and c (c . (b x a) * b . (d x c) > 0)
						//(this works because a . (b x c) == c . (b x a) and d . (b x c) == b . (d x c))(scalar triple product identity)
						//[scalar triple product of a,b,c == [ABC] = a . (b x c)]
						//same principle as previous
						//if (ADC * BDC < 0 && CBA * BDC > 0) {
						if (adc.i && bdc.i && (adc.i ^ bdc.i) < 0 && (cba.i ^ bdc.i) > 0) {
							const auto edge      = getEdge(curr.edge), 
								       otherEdge = other->getEdge(otherCurr.edge);

							//if edges are parallel, we don't care since they don't define a plane
							if (fuzzyParallel(edge, otherEdge)) continue;

							const auto v1 = trans->getTransformed(getVert(curr.edge.first)),
								       v2 = otherTrans->getTransformed(other->getVert(otherCurr.edge.first));

							//check distance from plane defined by edge normal and one vertex on this body's edge
							auto edgeNormal = glm::normalize(glm::cross(edge, otherEdge));
							edgeNormal *= signf(glm::dot(edgeNormal, v1 - trans->position()));//make sure the edge normal is facing outwards from the body

							const auto pen = glm::dot(edgeNormal, v2 - v1);//does this work regardless of the edges' points used?
							if (pen > manifold.pen) {
								manifold.edgePair.first  = curr;
								manifold.edgePair.second = otherCurr;
								manifold.pen = pen;
								manifold.axis = edgeNormal;
								//we found a separating axis boys
								if (manifold.pen > COLLISION_PEN_TOLERANCE) return manifold;
							}
						}

					}//end edge culling

				}
			}//end other gauss loop

		}
	}//end gauss loop
	return manifold;
}

// I should mention that the assumption that models are centered at the origin is what's breaking the algorithm at all right now
// it works perfectly for meshes centered at the origin
// we return a valid manifold, or if there was no collision one with a nullptr originator
Manifold Collider::intersects(Collider* other) {

	//quick sphere collision optimization
	const auto d = _framePos - other->framePos();//ignores displaced colliders for now (it's the one thing still breaking the algorithm)
	const auto distSq = d.x * d.x + d.y * d.y + d.z * d.z;
	const auto rad = _radius + other->radius();//probably want to use AABBs
	if (distSq > rad * rad)
		return Manifold();

	//separating axis theorem 2: electric boogaloo
	//axis of min pen on this collider
	auto minAxis = getAxisMinPen(other);
	if (minAxis.pen > COLLISION_PEN_TOLERANCE) {
		//std::cout << "This: " << minAxis.pen << "; " << minAxis.axis.x << ", " << minAxis.axis.y << ", " << minAxis.axis.z << std::endl;
		return Manifold();
	}

	//axis of min pen on other collider
	auto otherMinAxis = other->getAxisMinPen(this);
	if (otherMinAxis.pen > COLLISION_PEN_TOLERANCE) {
		//std::cout << "Other: " << otherMinAxis.pen << "; " << otherMinAxis.axis.x << ", " << otherMinAxis.axis.y << ", " << otherMinAxis.axis.z << std::endl;
		return Manifold();
	}

	//closest penetrating edges on both colliders
	auto minEdge = overlayGaussMaps(other);
	if (minEdge.pen > COLLISION_PEN_TOLERANCE) {
		//debug code
		const auto world = _transform->getMats()->world;
		const auto oworld = other->transform()->getMats()->world;
		const auto  v1  = (vec3)(world * vec4(getVert(minEdge.edgePair.first.edge.first), 1))
			      , v2  = (vec3)(world * vec4(getVert(minEdge.edgePair.first.edge.second), 1))
			      , ov1 = (vec3)(oworld * vec4(other->getVert(minEdge.edgePair.second.edge.first), 1))
			      , ov2 = (vec3)(oworld * vec4(other->getVert(minEdge.edgePair.second.edge.second), 1));
		DrawDebug::getInstance().drawDebugVector(v1, v2, vec3(1, 0, 0));
		DrawDebug::getInstance().drawDebugVector(ov1, ov2, vec3(1, 1, 0));
		//std::cout << "Edge: " << minEdge.pen << std::endl;
		return Manifold();
	}

	auto& minFace = (minAxis.pen > otherMinAxis.pen) ? minAxis : otherMinAxis;

	//edge-edge collision
	if (minEdge.pen > minFace.pen) {
		std::cout << "EDGE ";
		const auto world = _transform->getMats()->world;
		const auto oworld = other->transform()->getMats()->world;
		//get the points defining both edges in the collision in world space
		const auto p0 = (vec3)(world * vec4(getVert(minEdge.edgePair.first.edge.first), 1))
			     , p1 = (vec3)(world * vec4(getVert(minEdge.edgePair.first.edge.second), 1))
			     , q0 = (vec3)(oworld * vec4(other->getVert(minEdge.edgePair.second.edge.first), 1))
			     , q1 = (vec3)(oworld * vec4(other->getVert(minEdge.edgePair.second.edge.second), 1));

		//find the closest point between the two edges
		minEdge.colPoints.push_back(closestPointBtwnSegments(p0, p1, q0, q1));
		DrawDebug::getInstance().drawDebugSphere(minEdge.colPoints[0], 0.2f, vec3(0,1,0), 0.8f);
		return minEdge;
	}
	//face-* collision
	else {
		assert(minFace.axis.x || minFace.axis.y || minFace.axis.z);// Axis has not been set, collision probably got through with NaN errors
		
		//find the possible incident faces; the axis is the reference normal
		const auto incidents = minFace.other->getIncidentFaces(minFace.axis);
		//assert(incidents.size());// There were no incident faces! Somehow! // TODO FIX THIS!!!

		//clip the incident face(s) against the reference face
		minFace.originator->clipPolygons(minFace, incidents);

		for (const auto& colPoint : minFace.colPoints)
			DrawDebug::getInstance().drawDebugSphere(colPoint, 0.1f, vec3(1,0,0), 0.8f);

		return minFace;
	}
}

//Gets the indexes of the faces on this body that are most anti-parallel to the reference normal
std::vector<GLuint> Collider::getIncidentFaces(const vec3 refNormal) const {
	std::vector<GLuint> faces;
	auto antiProj = glm::dot(currNormals[0], refNormal);
	for (size_t i = 1, numFaces = currNormals.size(); i < numFaces; ++i) {
		//if + -, diff > 0; if - +, diff < 0; if same sign, magnitude of projection determines sign
		const auto proj = glm::dot(currNormals[i], refNormal);
		const auto diff = antiProj - proj;

		//if the face has close to 0 difference to the last anti-normal projection, then it's another incident face
		if (epsCheck(diff)) {
			faces.push_back(i);
		}
		//if the face is more anti-parallel than the previous, we replace our previous incident faces with this one
		else if (diff > 0) {
			faces = std::vector<GLuint>();
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
void Collider::clipPolygons(FaceManifold& reference, const std::vector<GLuint>& incidents) const {
	auto trans = _transform->getComputed();
	auto otherTrans = reference.other->transform()->getComputed();

	//get the transformed center point of the reference face
	auto& vertFaces = mesh->indices().verts;
	const auto index = reference.norm * 3;
	auto refCenter = getVert(vertFaces[index]) + getVert(vertFaces[index + 1]) + getVert(vertFaces[index + 2]);
	refCenter = trans->getTransformed(refCenter / 3.f);

	//DrawDebug::getInstance().drawDebugVector(refCenter, refCenter + refNorm);

	//get the side planes of the reference face
	//These are supposed to be the normals of the faces adjacent to the reference face, at least according to Bullet
	//I use the actual side planes of the face, 
	//i.e. normals from the edges perpendicular to the edge and face normal facing outward from the face's center
	std::vector<vec3> sidePlanes, sideVerts;
	for (auto i = 0; i < 3; ++i) {
		auto& v = vertFaces[reference.norm * 3 + i];
		const auto vert = trans->getTransformed(getVert(v))
			     , edge = getEdge({ v, vertFaces[reference.norm * 3 + (i + 1) % 3] });

		auto norm = glm::cross(reference.axis, edge);
		norm *= signf(glm::dot(norm, vert - refCenter));

		sidePlanes.push_back(norm);
		sideVerts.push_back(vert);

		//DrawDebug::getInstance().drawDebugVector(vert, vert + edge, vec3(1, 1, 0));
		//DrawDebug::getInstance().drawDebugVector(vert, vert + norm, vec3(1, 0, 1));
	}

	const auto oworld = otherTrans->getMats()->world;
	for (const auto incidentFace : incidents) {

		std::vector<vec3> incidentVerts = {
			(vec3)(oworld * vec4(reference.other->getVert(reference.other->getFaceVert(incidentFace * 3)), 1)),
			(vec3)(oworld * vec4(reference.other->getVert(reference.other->getFaceVert(incidentFace * 3 + 1)), 1)),
			(vec3)(oworld * vec4(reference.other->getVert(reference.other->getFaceVert(incidentFace * 3 + 2)), 1))
		};

		auto clipped = incidentVerts;
		for (auto s = 0; s < 3 && clipped.size(); ++s) {
			clipped = clipPolyAgainstEdge(clipped, sidePlanes[s], sideVerts[s], reference.axis, refCenter);
		}

		reference.colPoints.insert(reference.colPoints.end(), clipped.begin(), clipped.end());
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
std::vector<vec3> Collider::clipPolyAgainstEdge(std::vector<vec3>& input, const vec3 sidePlane, const vec3 sideVert, const vec3 refNorm, const vec3 refCenter) const {
	std::vector<vec3> output;

	//regular conditions protect against this, but just to be safe
	if (!input.size()) return output;

	vec3 startpt = input.back(), endpt;
	for (size_t i = 0, numInputs = input.size(); i < numInputs; ++i, startpt = endpt) {
		endpt = input[i];

		const auto clipStart = glm::dot(sidePlane, startpt - sideVert);
		const auto clipEnd   = glm::dot(sidePlane, endpt - sideVert);

		//the edge is "on the plane" (thick planes); keep end pt if it falls below reference face
		if (epsCheck(clipStart) || epsCheck(clipEnd)) {
			if (glm::dot(refNorm, endpt - refCenter) < 0) output.push_back(endpt);
			continue;
		}

		bool startInside = clipStart < 0, endInside = clipEnd < 0;//if they fall behind the side plane, they're inside the clip polygon

		//end pt is inside, keep it if it falls below the reference face
		if (endInside) {
			if (glm::dot(refNorm, endpt - refCenter) < 0) output.push_back(endpt);
		}

		//start pt and end pt fall on different sides of the plane, keep intersection if it falls below the reference face
		if (startInside != endInside) {
			//find intersection
			const auto e = glm::dot(sidePlane, startpt - endpt);
			const auto t = (e) ? clipStart / e : 0;
			//float t = clipStart * 1.f / (clipStart - clipEnd);
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
- You can also find a PDF copy in the proofs folder in the root of the repository
- As an alternative, my main source was http://geomalgorithms.com/a07-_distance.html

- The general thought process is that you start with the problem of finding an intersection between two lines
- The lines are defined by p = { p0, u = p1 - p0 } and q = { q0, v = q1 - q0 }
- This uses the parametric equations for the lines and the vector between them, w.
- The smallest w is defined as w(Sc, Tc) or W_c
- Sc = (b*e - c*d) / (a*c - b^2)
- Tc = (a*e - b*d) / (a*c - b^2)
- You can find the values for a,b,c,d,e in the code below
- If a*c - b^2 = 0, then the lines are parallel and Sc = 0, Tc = e / c
- The closest point to q on p is p0 + Sc * u
- The closest point to p on q is q0 + Tc * v
- To get the closest point in space we just compute p0 - W_c / 2 or q0 + W_c / 2

- The line problem isn't complex enough to account for the fact that these are segments
- The closest point between the two lines may not occur within the 0 to 1 range dictated by their segments
- We account for this ourselves by doing some range checks
- When s or t goes outside the 0 to 1 range, we have to change how we're solving for the other value
- if s < 0, t =  e / c. if s > 1, t = ( e + b) / c
- if t < 0, s = -d / a. if t > 1, s = (-d + b) / a
- After accounting for these, just do a basic clamp on these values to the 0 to 1 range
- We can save computations by just storing the numerators and denominators and doing the range checks with them
- That way there's only two divisions (Sc and Tc) at the end
------------------------------------------------------------------------------------------------------------------------------------
*/
vec3 Collider::closestPointBtwnSegments(const vec3 p0, const vec3 p1, const vec3 q0, const vec3 q1) const {
	auto u = p1 - p0, v = q1 - q0, w0 = p0 - q0;

	const auto a = glm::dot(u, u)
		     , b = glm::dot(u, v)
		     , c = glm::dot(v, v)
		     , d = glm::dot(u, w0)
		     , e = glm::dot(v, w0)
		     , D = a * c - b * b;

	float sNumer, sDenom = D;
	float tNumer, tDenom = D;

	//if D ~ 0, i.e. segments are parallel
	if (D < FLT_EPSILON) {
		sNumer = 0;	sDenom = 1;
		tNumer = e;	tDenom = c;
	}
	else {
		sNumer = b * e - c * d;
		tNumer = a * e - b * d;

		//if Sc < 0, Sc = 0 and Tc = e / c
		if (sNumer < 0) {
			sNumer = 0;
			tNumer = e; tDenom = c;
		}
		//if Sc > 1, Sc = 1 and Tc = (e + b) / c
		else if (sNumer > sDenom) {
			sNumer = sDenom;
			tNumer = e + b; tDenom = c;
		}
	}

	//if Tc < 0, Tc = 0 and Sc = -d / a
	if (tNumer < 0) {
		tNumer = 0;
		//if Sc < 0, Sc = 0; Tc = 0
		if (-d < 0)
			sNumer = 0;
		//if Sc > 1, Sc = 1; Tc = 0
		else if (-d > a)
			sNumer = sDenom;
		else {
			sNumer = -d; sDenom = a;
		}
	}
	//if Tc > 1, Tc = 1 and Sc = (-d + b) / a
	else if (tNumer > tDenom) {
		tNumer = tDenom;
		//if Sc < 0, Sc = 0; Tc = 1
		if (-d + b < 0)
			sNumer = 0;
		//if Sc > 1, Sc = 1; Tc = 1
		else if (-d + b > a)
			sNumer = sDenom;
		else {
			sNumer = -d + b; sDenom = a;
		}
	}

	//prevents possible divide by zero
	float sc = epsCheck(sNumer) ? 0 : sNumer / sDenom;
	float tc = epsCheck(tNumer) ? 0 : tNumer / tDenom;

	v *= tc;
	auto wc = w0; 
	wc += (sc * u);
	wc -= v; //the vector between the 2 closest points on the 2 segments (W_c = w0 + (Sc * u) - (Tc * v))
	wc *= 0.5f;
	wc += v;
	wc += q0;
	return wc;//the closest point between the 2 segments in the world (q0 + (Tc * v) + W_c * 0.5f)
}

void Collider::genVerts() {
	if (_type != Type::BOX)
		return;
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
			auto& v  = meshVerts[faceVerts[i]];

			const auto e1 = meshVerts[faceVerts[i + 1]] - v;
			const auto e2 = meshVerts[faceVerts[i + 2]] - v;
			
			const auto cross = glm::cross(e1, e2);			
			faceNormals.push_back(cross / glm::length(cross));
		}
		break;
	}
	currNormals = std::vector<vec3>(faceNormals.size());
}

void Collider::genEdges() {
	switch (_type) {
	case Type::SPHERE:
		break;
	case Type::BOX:
	case Type::MESH:
		auto& meshVerts = mesh->data().verts;
		for (const auto& pair : gauss.adjacencies) {
			for (size_t i = 0, numAdj = pair.second.size(); i < numAdj; ++i) {
				const auto& adj = pair.second[i];
				setEdge(adj.edge, edges.size());
				edges.push_back(meshVerts[adj.edge.second] - meshVerts[adj.edge.first]);
			}
		}
		break;
	}
	currEdges = std::vector<vec3>(edges.size());
}

void Collider::genGaussMap() {
	switch (_type) {
	case Type::SPHERE:
		break;
	case Type::BOX:
		//need to set up proper handling for box colliders for vertices
		//gauss.addAdj(faceNormals[0], Adj{  });
		//gauss.adjacencies[faceNormals[1]] = { Adj{ 2, {,} } };
		break;
	case Type::MESH:
		//set up the edge associations
		auto& faceVerts = mesh->indices().verts;
		//adjacencies = std::vector<std::vector<Adj>>(numFaces / 3, std::vector<Adj>());
		for (size_t i = 0, numFaces = faceVerts.size(); i < numFaces; i += 3) {
			for (auto j = i + 3; j < numFaces; j += 3) {
				// if (fuzzyParallel(faceNormals[i / 3], faceNormals[j / 3])) continue;
				
				auto src = -1;
				auto added = false;
				for (size_t p1 = 0; !added && p1 < 3; ++p1) {
					for (size_t p2 = 0; p2 < 3; ++p2) {
						// continue if there's no match
						if (faceVerts[i + p1] != faceVerts[j + p2]) continue;
						
						//if a match hasn't been found yet, just record it
						if (src < 0) src = faceVerts[i + p1];
						//otherwise, record it, set the normal, push it back, and end the loop
						else {
							const GLuint usrc = src;
							const auto dst = faceVerts[i + p1];
							auto adj = Adj{ { i/3, j/3 }, { usrc, dst } };
							if (usrc > dst) { adj.edge.first = dst; adj.edge.second = usrc; }
							gauss.addAdj(faceNormals[adj.faces.first], adj);
							added = true;
						}
						// none of the other vertices can be equal to this one now, so move to the next one
						break;
					}
				}//end edge loop

			}
		}//end face loop
		break;
	}
}

bool Collider::fuzzyParallel(const vec3 v1, const vec3 v2) const {
	if (v1 == v2)
		return true;
	const auto propx = (v1.x) ? v2.x / v1.x : 0;
	const auto propy = (v1.y) ? v2.y / v1.y : 0;

	// minimizes divisions
	// equivalent to: return EPS_CHECK(a) && EPS_CHECK(b);
	if (epsCheck((propy - propx) / propx)) {
		const auto propz = (v1.z) ? v2.z / v1.z : 0;
		return epsCheck((propz - propx) / propx);
	}
	return false;
}

void Collider::updateNormals() {
	switch (_type) {
	case Type::SPHERE:
		break;
	case Type::BOX:
	case Type::MESH:
		const auto rot = _transform->getMats()->rotate;
		//auto& faceVerts = mesh->faces().verts;
		for (size_t i = 0, numNormals = faceNormals.size(); i < numNormals; ++i) {
			currNormals[i] = (vec3)(rot * vec4(faceNormals[i], 0));

			//const auto  a = _transform->getTransformed(getVert(faceVerts[i * 3]))
			//	      , b = _transform->getTransformed(getVert(faceVerts[i * 3 + 1]))
			//	      , c = _transform->getTransformed(getVert(faceVerts[i * 3 + 2]));
			//auto center = a + b + c;
			//center /= 3;
			//DrawDebug::getInstance().drawDebugVector(center, center + currNormals[i]);
		}
		break;
	}
}

void Collider::updateEdges() {
	switch (_type) {
	case Type::SPHERE:
		break;
	case Type::BOX:
	case Type::MESH:
		const auto world = _transform->getMats()->world;
		for (size_t i = 0, numEdges = edges.size(); i < numEdges; ++i) {
			currEdges[i] = (vec3)(world * vec4(edges[i], 0));
		}
		
		/*for (auto& pair : gauss.adjacencies) {
			for (size_t i = 0, numAdj = pair.second.size(); i < numAdj; ++i) {
				auto a = pair.second[i];
				auto s = _transform->getTransformed(getVert(a.edge[0]));
				auto edge = getEdge(a.edge);
				DrawDebug::getInstance().drawDebugVector(s, s + edge);
			}
		}*/
		break;
	}
}

const std::vector<vec3>& Collider::getCurrNormals() const { return currNormals; }
const std::vector<vec3>& Collider::getEdges() const { return currEdges; }

GLuint Collider::getFaceVert(GLuint index) const { return mesh->indices().verts[index]; }
vec3 Collider::getVert(GLuint index) const { return mesh->data().verts[index]; }
vec3 Collider::getNormal(GLuint index) const { return currNormals[index]; }
vec3 Collider::getEdge(std::pair<GLuint, GLuint> e) const {
	std::string key;
	if (e.first < e.second) { key = std::to_string(e.first) + "," + std::to_string(e.second); }
	else                    { key = std::to_string(e.second) + "," + std::to_string(e.first); }
	return currEdges[edgeMap.at(key)];
}
void Collider::setEdge(std::pair<GLuint, GLuint> e, const GLuint index) {
	std::string key;
	if (e.first < e.second) { key = std::to_string(e.first) + "," + std::to_string(e.second); }
	else                    { key = std::to_string(e.second) + "," + std::to_string(e.first); }
	edgeMap[key] = index;
}

const GaussMap& Collider::getGaussMap() const { return gauss; }

const std::vector<Adj>& GaussMap::getAdjs(vec3 v) const { return adjacencies.at(to_string(v)); }
void GaussMap::addAdj(vec3 v, Adj a) { adjacencies[to_string(v)].push_back(a); }

bool AABB::intersects(const AABB& other) const {
	auto xSeparate = (center.x - halfDims.x > other.center.x + other.halfDims.x) || (center.x + halfDims.x < other.center.x - other.halfDims.x);
	auto ySeparate = (center.y - halfDims.y > other.center.y + other.halfDims.y) || (center.y + halfDims.y < other.center.y - other.halfDims.y);
	auto zSeparate = (center.z - halfDims.z > other.center.z + other.halfDims.z) || (center.z + halfDims.z < other.center.z - other.halfDims.z);
	return !(xSeparate || ySeparate || zSeparate);
}