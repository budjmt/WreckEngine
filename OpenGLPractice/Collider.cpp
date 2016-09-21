#include "Collider.h"

#include <iostream>

Collider::Collider() { }
Collider::Collider(Transform* t, vec3 d, bool fudge) : Collider(ColliderType::BOX, nullptr, t, d, fudge) { }
Collider::Collider(Mesh* m, Transform* t) : Collider(ColliderType::MESH, m, t, m->getDims()) { }
Collider::Collider(ColliderType type, Mesh* m, Transform* t, vec3 d, bool fudge) : _type(type), mesh(m), _transform(t), fudgeAABB(fudge) 
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

vec3 Collider::dims() const { return _dims; }
void Collider::dims(vec3 v) {
	_dims = v;
	_radius = maxf(maxf(_dims.x, _dims.y), _dims.z);
	base_aabb.halfDims = v;
}

//makes sure the radius is up to date
void Collider::updateDims() {
	auto& scale = _transform->getComputed()->scale();
	_radius = maxf(maxf(_dims.x * scale.x, _dims.y * scale.y), _dims.z * scale.z);
	
	auto factor = fudgeAABB ? 1.2f : 1.f;
	transformed_aabb.halfDims = base_aabb.halfDims * scale * factor;
}

void Collider::update() {
	_framePos = _transform->getComputed()->position;
	base_aabb.center = _framePos;
	transformed_aabb.center = base_aabb.center;
	
	updateDims();
	updateNormals();
	updateEdges();
	
	//DrawDebug::getInstance().drawDebugSphere(_framePos, _radius);
}

//gets the vertex of the collider furthest in the direction of dir
SupportPoint Collider::getSupportPoint(vec3 dir) {
	auto& verts = mesh->verts();
	auto trans = _transform->getComputed();

	SupportPoint support{ trans->getTransformed(verts[0]), glm::dot(support.point, dir) };
	for (size_t i = 1, numVerts = verts.size(); i < numVerts; i++) {
		auto vert = trans->getTransformed(verts[i]);
		auto proj = glm::dot(vert, dir);
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

	int numAxes = currNormals.size();
	auto meshVerts = mesh->verts();
	auto faceVerts = mesh->faces().verts;
	auto trans = _transform->getComputed();
	for (int i = 0; axis.pen < 0 && i < numAxes; i++) {
		auto& norm = currNormals[i];
		auto support = other->getSupportPoint(-norm);
		auto vert = trans->getTransformed(meshVerts[faceVerts[i * FLOATS_PER_VERT]]);

		auto pen = glm::dot(norm, support.point - vert);//point-plane signed distance, negative if penetrating, positive if not
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

	for (auto& pair : gauss.adjacencies) {
		for (size_t i = 0, numAdj = pair.second.size(); i < numAdj; i++) {

			auto& curr = pair.second[i];
			vec3 &a = currNormals[curr.faces.first], &b = currNormals[curr.faces.second];

			for (auto& otherPair : othergauss.adjacencies) {
				for (size_t j = 0, othernumAdj = otherPair.second.size(); j < othernumAdj; j++) {

					//we found a separating axis boys
					if (manifold.pen > 0)
						return manifold;

					auto& otherCurr = otherPair.second[j];
					vec3 &c = -otherNormals[otherCurr.faces.first], &d = -otherNormals[otherCurr.faces.second];//these must be negative to account for the minkowski DIFFERENCE

					//checks if the arcs between arc(a,b) and arc(c,d) intersect
					auto bxa = glm::cross(b, a);
					float cba = glm::dot(c, bxa)
						, dba = glm::dot(d, bxa);

					//if c and d are on different sides of arc ba
					if (cba * dba < 0) {

						auto dxc  = glm::cross(d, c);
						float adc = glm::dot(a, dxc)
							, bdc = glm::dot(b, dxc);

						//if a and b are on different sides of arc dc &&
						//if a and d are on the same side of the plane formed by b and c (c . (b x a) * b . (d x c) > 0)
						//(this works because a . (b x c) == c . (b x a) and d . (b x c) == b . (d x c))(scalar triple product identity)
						//[scalar triple product of a,b,c == [abc] = a . (b x c)]
						if (adc * bdc < 0 && cba * bdc > 0) {
							vec3 &edge = getEdge(curr.edge), 
								 &otherEdge = other->getEdge(otherCurr.edge);

							//if edges are parallel, we don't care since they don't define a plane
							if (fuzzyParallel(edge, otherEdge)) continue;

							vec3 v1 = trans->getTransformed(getVert(curr.edge.first)),
								 v2 = otherTrans->getTransformed(other->getVert(otherCurr.edge.first));

							//check distance from plane defined by edge normal and one vertex on this body's edge
							auto edgeNormal = glm::normalize(glm::cross(edge, otherEdge));
							edgeNormal *= signf(glm::dot(edgeNormal, v1 - trans->position()));//make sure the edge normal is facing outwards from the body

							auto pen = glm::dot(edgeNormal, v2 - v1);//does this work regardless of the edges' points used?
							if (pen > manifold.pen) {
								manifold.edgePair.first  = curr;
								manifold.edgePair.second = otherCurr;
								manifold.pen = pen;
								manifold.axis = edgeNormal;
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
	auto d = _framePos - other->framePos();//ignores displaced colliders for now (it's the one thing still breaking the algorithm)
	float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
	float rad = _radius + other->radius();//probably want to use aabbs
	if (distSq > rad * rad)
		return Manifold();

	//separating axis theorem 2: electric boogaloo
	//axis of min pen on this collider
	auto minAxis = getAxisMinPen(other);
	if (minAxis.pen > 0) {
		//std::cout << "This: " << minAxis.pen << "; " << minAxis.axis.x << ", " << minAxis.axis.y << ", " << minAxis.axis.z << std::endl;
		return Manifold();
	}

	//axis of min pen on other collider
	auto otherMinAxis = other->getAxisMinPen(this);
	if (otherMinAxis.pen > 0) {
		//std::cout << "Other: " << otherMinAxis.pen << "; " << otherMinAxis.axis.x << ", " << otherMinAxis.axis.y << ", " << otherMinAxis.axis.z << std::endl;
		return Manifold();
	}

	//closest penetrating edges on both colliders
	auto minEdge = overlayGaussMaps(other);
	if (minEdge.pen > 0) {
		//debug code
		auto t = _transform->getComputed();
		auto ot = other->transform()->getComputed();
		vec3 v1  = t->getTransformed(getVert(minEdge.edgePair.first.edge.first))
		   , v2  = t->getTransformed(getVert(minEdge.edgePair.first.edge.second))
		   , ov1 = ot->getTransformed(other->getVert(minEdge.edgePair.second.edge.first))
		   , ov2 = ot->getTransformed(other->getVert(minEdge.edgePair.second.edge.second));
		DrawDebug::getInstance().drawDebugVector(v1, v2, vec3(1, 0, 0));
		DrawDebug::getInstance().drawDebugVector(ov1, ov2, vec3(1, 1, 0));
		//std::cout << "Edge: " << minEdge.pen << std::endl;
		return Manifold();
	}

	auto& minFace = (minAxis.pen > otherMinAxis.pen) ? minAxis : otherMinAxis;

	//edge-edge collision
	if (minEdge.pen > minFace.pen) {
		std::cout << "EDGE ";
		auto t = _transform->getComputed();
		auto ot = other->transform()->getComputed();
		//get the points defining both edges in the collision in world space
		vec3  p0 = t->getTransformed(getVert(minEdge.edgePair.first.edge.first))
			, p1 = t->getTransformed(getVert(minEdge.edgePair.first.edge.second))
			, q0 = ot->getTransformed(other->getVert(minEdge.edgePair.second.edge.first))
			, q1 = ot->getTransformed(other->getVert(minEdge.edgePair.second.edge.second));

		//find the closest point between the two edges
		minEdge.colPoints.push_back(closestPointBtwnSegments(p0, p1, q0, q1));
		DrawDebug::getInstance().drawDebugSphere(minEdge.colPoints[0], 0.2f, vec3(0,1,0), 0.8f);
		return minEdge;
	}
	//face-* collision
	else {
		assert(minFace.axis.x || minFace.axis.y || minFace.axis.z);// Axis has not been set, collision probably got through with NaN errors
		
		//find the possible incident faces; the axis is the reference normal
		auto incidents = minFace.other->getIncidentFaces(minFace.axis);
		assert(incidents.size());// There were no incident faces! Somehow!

		//clip the incident face(s) against the reference face
		minFace.originator->clipPolygons(minFace, incidents);

		for (auto& colPoint : minFace.colPoints)
			DrawDebug::getInstance().drawDebugSphere(colPoint, 0.1f, vec3(1,0,0), 0.8f);

		return minFace;
	}
}

//Gets the indexes of the faces on this body that are most antiparallel to the reference normal
std::vector<GLuint> Collider::getIncidentFaces(vec3 refNormal) {
	std::vector<GLuint> faces;
	auto antiProj = glm::dot(currNormals[0], refNormal);
	for (size_t i = 1, numFaces = currNormals.size(); i < numFaces; ++i) {
		//if + -, diff > 0; if - +, diff < 0; if same sign, magnitude of projection determines sign
		auto proj = glm::dot(currNormals[i], refNormal);
		auto diff = antiProj - proj;

		//if the face has close to 0 difference with the last anti-normal projection, then it's another incident face
		if (EPS_CHECK(diff))
			faces.push_back(i);

		//if the face is more antiparallel than the previous, we replace our previous incident faces with this one
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
void Collider::clipPolygons(FaceManifold& reference, std::vector<GLuint>& incidents) {
	auto trans = _transform->getComputed();
	auto otherTrans = reference.other->transform()->getComputed();

	//get the transformed center point of the reference face
	auto& vertFaces = mesh->faces().verts;
	auto index = reference.norm * 3;
	auto refCenter = getVert(vertFaces[index]) + getVert(vertFaces[index + 1]) + getVert(vertFaces[index + 2]);
	refCenter = trans->getTransformed(refCenter / 3.f);

	//DrawDebug::getInstance().drawDebugVector(refCenter, refCenter + refNorm);

	//get the side planes of the reference face
	//These are supposed to be the normals of the faces adjacent to the reference face, at least according to Bullet
	//I use the actual side planes of the face, 
	//i.e. normals from the edges perpendicular to the edge and face normal facing outward from the face's center
	std::vector<vec3> sidePlanes, sideVerts;
	for (auto i = 0; i < 3; i++) {
		auto& v = vertFaces[reference.norm * 3 + i];
		vec3 vert = trans->getTransformed(getVert(v))
			, edge = getEdge({ v, vertFaces[reference.norm * 3 + (i + 1) % 3] });

		auto norm = glm::cross(reference.axis, edge);
		norm *= signf(glm::dot(norm, vert - refCenter));

		sidePlanes.push_back(norm);
		sideVerts.push_back(vert);

		//DrawDebug::getInstance().drawDebugVector(vert, vert + edge, vec3(1, 1, 0));
		//DrawDebug::getInstance().drawDebugVector(vert, vert + norm, vec3(1, 0, 1));
	}

	for (size_t i = 0, numIncidents = incidents.size(); i < numIncidents; ++i) {

		std::vector<vec3> incidentVerts = {
			otherTrans->getTransformed(reference.other->getVert(reference.other->getFaceVert(incidents[i] * 3))),
			otherTrans->getTransformed(reference.other->getVert(reference.other->getFaceVert(incidents[i] * 3 + 1))),
			otherTrans->getTransformed(reference.other->getVert(reference.other->getFaceVert(incidents[i] * 3 + 2)))
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
- Clipping has 4 cases to determine which vertices are kept, going through sets of start and end pts:
- Start and End both inside the reference face (here defined as behind it, since these are side planes facing outward): we keep only the end pt
- The reason we keep the end point and not the start point is because this algorithm ensures that all start pts will be end pts at one point, and vice versa
- This means enclosed start pts will be added at some point no matter what, so we only worry about end pts
- This case is more important for telling us there will be no intersection between the plane and the edge between start and end, so we don't need to add it

- Start and End both outside: we keep nothing, as nothing falls within the clip polygon

- Start inside and End outside: we keep the intersection between the edge and the side plane
- Because the points are on different side of the plane, there must be an intersection with the plane

- Start outside and End inside: we keep both the intersection and the end pt
- Because of how the first case actually works (telling us there is no intersection), this case and the first can be condensed together
- All we're saying is that since the start pt is NOT inside and the end pt IS, there must be an intersection, so we have to add that on top of the end pt
- Another way of describing these cases is to base it on the positioning of the end pt
- If the end pt is inside, we'll be keeping the end pt, otherwise we won't since it falls outside
- If the start pt is on the opposite side of the plane of the end pt, we also keep the intersection between the plane and the edge between the two pts

- These 2 simpler cases with possible overlap lead to the more complex explanation and the original 4 cases

- This is the version I use; it's less for optimization and more for reducing obfuscation, as the meaning here is easier to remember than the original 4 cases
- I should mention that it isn't really any less efficient than the original version; it adds a bitwise xor between startInside and endInside, but that's extremely negligible
- It's also less code, as the intersection code doesn't need to be duplicated

- There is one additional case that occurs for our purposes to account for floating point error
- Either the Start or End falls ON the side plane: we keep the end pt
- We define this as the dot product of the side plane normal and the vector from the plane to either vert being within -FLT_EPSILON and FLT_EPSILON
- We call this using thick planes, rather than planes of indeterminably small thickness which is what we normally use
------------------------------------------------------------------------------------------------------------------------------------
*/
std::vector<vec3> Collider::clipPolyAgainstEdge(std::vector<vec3>& input, vec3 sidePlane, vec3 sideVert, vec3 refNorm, vec3 refCenter) {
	std::vector<vec3> output;

	//regular conditions protect against this, but just to be safe
	if (!input.size()) return output;

	vec3 startpt = input[input.size() - 1], endpt;
	for (size_t i = 0, numInputs = input.size(); i < numInputs; ++i, startpt = endpt) {
		endpt = input[i];

		auto clipStart = glm::dot(sidePlane, startpt - sideVert);
		auto clipEnd   = glm::dot(sidePlane, endpt - sideVert);

		//the edge is "on the plane" (thick planes); keep end pt if it falls below reference face
		if (EPS_CHECK(clipStart) || EPS_CHECK(clipEnd)) {
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
			auto e = glm::dot(sidePlane, startpt - endpt);
			auto t = (e) ? clipStart / e : 0;
			//float t = clipStart * 1.f / (clipStart - clipEnd);
			auto intersect = glm::mix(startpt, endpt, t);
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
- You can also find a pdf copy in the proofs folder in the root of the repository
- As an alternative, my main source was http://geomalgorithms.com/a07-_distance.html

- The general thought process is that you start with the problem of finding an intersection between two lines
- The lines are defined by p = { p0, u = p1 - p0 } and q = { q0, v = q1 - q0 }
- This uses the parametric equations for the lines and the vector between them, w.
- The smallest w is defined as w(sc, tc) or wc
- sc = (be - cd) / (ac - b^2)
- tc = (ae - bd) / (ac - b^2)
- You can find the values for a,b,c,d,e in the code below
- If ac - b^2 = 0, then the lines are parallel and sc = 0, tc = e / c
- The closest point to q on p is p0 + sc * u
- The closest point to p on q is q0 + tc * v
- To get the closest point in space we just compute p0 - wc / 2 or q0 + wc / 2

- The line problem isn't complex enough to account for the fact that these are segments
- The closest point between the two lines may not occur within the 0 to 1 range dictated by their segments
- We account for this ourselves by doing some range checks
- When s or t goes outside the 0 to 1 range, we have to change how we're solving for the other value
- if s < 0, t =  e / c. if s > 1, t = ( e + b) / c
- if t < 0, s = -d / a. if t > 1, s = (-d + b) / a
- After accounting for these, just do a basic clamp on these values to the 0 to 1 range
- We can save computations by just storing the numerators and denominators and doing the range checks with them
- That way there's only two divisions (sc and tc) at the end
------------------------------------------------------------------------------------------------------------------------------------
*/
vec3 Collider::closestPointBtwnSegments(vec3 p0, vec3 p1, vec3 q0, vec3 q1) {
	vec3 u = p1 - p0, v = q1 - q0, w0 = p0 - q0;

	float a = glm::dot(u, u)
		, b = glm::dot(u, v)
		, c = glm::dot(v, v)
		, d = glm::dot(u, w0)
		, e = glm::dot(v, w0)
		, D = a * c - b * b;

	float sc, sNumer, sDenom = D;
	float tc, tNumer, tDenom = D;

	//if D ~ 0, i.e. segments are parallel
	if (D < FLT_EPSILON) {
		sNumer = 0;	sDenom = 1;
		tNumer = e;	tDenom = c;
	}
	else {
		sNumer = b * e - c * d;
		tNumer = a * e - b * d;

		//if sc < 0, sc = 0 and tc = e / c
		if (sNumer < 0) {
			sNumer = 0;
			tNumer = e; tDenom = c;
		}
		//if sc > 1, sc = 1 and tc = (e + b) / c
		else if (sNumer > sDenom) {
			sNumer = sDenom;
			tNumer = e + b; tDenom = c;
		}
	}

	//if tc < 0, tc = 0 and sc = -d / a
	if (tNumer < 0) {
		tNumer = 0;
		//if sc < 0, sc = 0; tc = 0
		if (-d < 0)
			sNumer = 0;
		//if sc > 1, sc = 1; tc = 0
		else if (-d > a)
			sNumer = sDenom;
		else {
			sNumer = -d; sDenom = a;
		}
	}
	//if tc > 1, tc = 1 and sc = (-d + b) / a
	else if (tNumer > tDenom) {
		tNumer = tDenom;
		//if sc < 0, sc = 0; tc = 1
		if (-d + b < 0)
			sNumer = 0;
		//if sc > 1, sc = 1; tc = 1
		else if (-d + b > a)
			sNumer = sDenom;
		else {
			sNumer = -d + b; sDenom = a;
		}
	}

	//prevents possible divide by zero
	sc = (EPS_CHECK(sNumer)) ? 0 : sNumer / sDenom;
	tc = (EPS_CHECK(tNumer)) ? 0 : tNumer / tDenom;

	vec3 wc = w0 + (sc * u) - (tc * v);//the vec between the 2 closest pts on the 2 segments
	return q0 + tc * v + wc * 0.5f;//the closest point between the 2 segments in the world
}

//the code below will need updating when the system is updated for 3D
/*void Collider::setCorners(std::vector<vec3> c) {
corners = c;
}*/

void Collider::genVerts() {
	if (_type != ColliderType::BOX)
		return;
	std::vector<vec3> verts, norms, uvs;//norms and uvs are empty
	Face faces;
	verts.push_back(vec3(_dims.x, _dims.y, _dims.z));
	verts.push_back(vec3(-_dims.x, _dims.y, _dims.z));
	verts.push_back(vec3(_dims.x, -_dims.y, _dims.z));
	verts.push_back(vec3(-_dims.x, -_dims.y, _dims.z));
	verts.push_back(vec3(_dims.x, _dims.y, -_dims.z));
	verts.push_back(vec3(-_dims.x, _dims.y, -_dims.z));
	verts.push_back(vec3(_dims.x, -_dims.y, -_dims.z));
	verts.push_back(vec3(-_dims.x, -_dims.y, -_dims.z));

	faces.verts.push_back(7); faces.verts.push_back(3); faces.verts.push_back(0);
	faces.verts.push_back(7); faces.verts.push_back(4); faces.verts.push_back(0);

	faces.verts.push_back(6); faces.verts.push_back(7); faces.verts.push_back(4);
	faces.verts.push_back(6); faces.verts.push_back(5); faces.verts.push_back(4);

	faces.verts.push_back(2); faces.verts.push_back(6); faces.verts.push_back(5);
	faces.verts.push_back(2); faces.verts.push_back(1); faces.verts.push_back(5);

	faces.verts.push_back(3); faces.verts.push_back(2); faces.verts.push_back(1);
	faces.verts.push_back(3); faces.verts.push_back(0); faces.verts.push_back(1);

	faces.verts.push_back(0); faces.verts.push_back(1); faces.verts.push_back(5);
	faces.verts.push_back(0); faces.verts.push_back(4); faces.verts.push_back(5);

	faces.verts.push_back(7); faces.verts.push_back(6); faces.verts.push_back(2);
	faces.verts.push_back(7); faces.verts.push_back(3); faces.verts.push_back(2);

	mesh = new Mesh(verts, norms, uvs, faces);
}

void Collider::genNormals() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		faceNormals.push_back(vec3(1, 0, 0));//need to define some kind of vertex array for box colliders
		faceNormals.push_back(vec3(0, 0, -1));
		faceNormals.push_back(vec3(-1, 0, 0));
		faceNormals.push_back(vec3(0, 0, 1));
		faceNormals.push_back(vec3(0, 1, 0));
		faceNormals.push_back(vec3(0, -1, 0));
		break;
	case ColliderType::MESH:
		// generate the face normals from the mesh's vertices
		// when iterating over normals, to retrieve the vertices of the face corresponding to the normal at index i,
		// the nth (0, 1, or 2) vertex in the face is meshVerts[faceVerts[i * 3 + n]]
		// alternatively, if you aren't getting meshVerts, use the function getVert(faceVerts[i * 3 + n])
		auto& faceVerts = mesh->faces().verts;
		auto& meshVerts = mesh->verts();
		for (size_t i = 0, numFaces = faceVerts.size(); i < numFaces; i += 3) {
			auto& v  = meshVerts[faceVerts[i]];

			auto e1 = meshVerts[faceVerts[i + 1]] - v;
			auto e2 = meshVerts[faceVerts[i + 2]] - v;
			
			auto cross = glm::cross(e1, e2);			
			faceNormals.push_back(cross / glm::length(cross));
		}
		break;
	}
	currNormals = std::vector<vec3>(faceNormals.size());
}

void Collider::genEdges() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
	case ColliderType::MESH:
		auto& meshVerts = mesh->verts();
		for (auto& pair : gauss.adjacencies) {
			for (size_t i = 0, numAdj = pair.second.size(); i < numAdj; ++i) {
				auto& adj = pair.second[i];
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
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		//need to set up proper handling for box colliders for vertices
		//gauss.addAdj(faceNormals[0], Adj{  });
		//gauss.adjacencies[faceNormals[1]] = { Adj{ 2, {,} } };
		break;
	case ColliderType::MESH:
		//set up the edge associations
		auto& faceVerts = mesh->faces().verts;
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
							GLuint usrc = src;
							auto dst = faceVerts[i + p1];
							auto adj = Adj{ { i/3, j/3 }, { usrc, dst } };
							if (usrc > dst) { adj.edge.first = dst; adj.edge.second = usrc; }
							gauss.addAdj(faceNormals[adj.faces.first], adj);

							//adjacencies[a.face1].push_back(a);
							//adjacencies[a.face2].push_back(a);

							added = true;
						}
						// none of the other verts can be equal to this one now, so move to the next one
						break;
					}
				}//end edge loop

			}
		}//end face loop
		break;
	}
}

bool Collider::fuzzyParallel(vec3 v1, vec3 v2) {
	if (v1 == v2)
		return true;
	auto propx = (v1.x) ? v2.x / v1.x : 0;
	auto propy = (v1.y) ? v2.y / v1.y : 0;
	auto propz = (v1.z) ? v2.z / v1.z : 0;

	// minimizes divisions
	auto a = (propy - propx) / propx;
	if (EPS_CHECK(a)) {
		auto b = (propz - propx) / propx;
		return EPS_CHECK(b);
	}
	return false;
}

void Collider::updateNormals() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
	case ColliderType::MESH:
		auto t = _transform->getComputed();
		auto rot = glm::rotate(t->rotAngle(), t->rotAxis());
		//auto faceVerts = mesh->faces().verts;
		for (size_t i = 0, numNormals = faceNormals.size(); i < numNormals; i++) {
			currNormals[i] = (vec3)(rot * vec4(faceNormals[i], 1));

			/*vec3 a = t.getTransformed(getVert(faceVerts[i * 3])), b = t.getTransformed(getVert(faceVerts[i * 3 + 1])), c = t.getTransformed(getVert(faceVerts[i * 3 + 2]));
			vec3 center = a + b + c;
			center /= 3;
			DrawDebug::getInstance().drawDebugVector(center, center + currNormals[i]);*/
		}
		break;
	}
}

void Collider::updateEdges() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
	case ColliderType::MESH:
		auto t = _transform->getComputed();
		auto rot = glm::rotate(t->rotAngle(), t->rotAxis());
		auto scale = glm::scale(t->scale());
		int numEdges = edges.size();
		for (int i = 0; i < numEdges; i++) {
			currEdges[i] = (vec3)(rot * (scale * vec4(edges[i], 1)));//this is probably slow
		}
		/*
		for (std::pair<std::string, std::vector<Adj>> pair : gauss.adjacencies) {
		for (int i = 0, numAdj = pair.second.size(); i < numAdj; i++) {
		Adj a = pair.second[i];
		vec3 s = t.getTransformed(getVert(a.edge[0]));
		vec3 edge = getEdge(a.edge);
		DrawDebug::getInstance().drawDebugVector(s,s + edge);
		}
		}*/
		break;
	}
}

const std::vector<vec3>& Collider::getCurrNormals() const { return currNormals; }
const std::vector<vec3>& Collider::getEdges() const { return currEdges; }

int Collider::getFaceVert(size_t index) const { return mesh->faces().verts[index]; }
vec3 Collider::getVert(size_t index) const { return mesh->verts()[index]; }
vec3 Collider::getNormal(size_t index) const { return currNormals[index]; }
vec3 Collider::getEdge(std::pair<GLuint, GLuint> e) {
	std::string key;
	if (e.first < e.second) { key = std::to_string(e.first) + "," + std::to_string(e.second); }
	else                    { key = std::to_string(e.second) + "," + std::to_string(e.first); }
	return currEdges[edgeMap[key]];
}
void Collider::setEdge(std::pair<GLuint, GLuint> e, size_t index) {
	std::string key;
	if (e.first < e.second) { key = std::to_string(e.first) + "," + std::to_string(e.second); }
	else                    { key = std::to_string(e.second) + "," + std::to_string(e.first); }
	edgeMap[key] = index;
}

const GaussMap& Collider::getGaussMap() const { return gauss; }

std::vector<Adj>& GaussMap::getAdjs(vec3 v) { return adjacencies[to_string(v)]; }
void GaussMap::addAdj(vec3 v, Adj a) { adjacencies[to_string(v)].push_back(a); }

bool AABB::intersects(const AABB& other) const {
	auto xSeparate = (center.x - halfDims.x > other.center.x + other.halfDims.x) || (center.x + halfDims.x < other.center.x - other.halfDims.x);
	auto ySeparate = (center.y - halfDims.y > other.center.y + other.halfDims.y) || (center.y + halfDims.y < other.center.y - other.halfDims.y);
	auto zSeparate = (center.z - halfDims.z > other.center.z + other.halfDims.z) || (center.z + halfDims.z < other.center.z - other.halfDims.z);
	return !(xSeparate || ySeparate || zSeparate);
}