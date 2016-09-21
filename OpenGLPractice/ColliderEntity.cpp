#include "ColliderEntity.h"

#include "CollisionManager.h"

//ColliderEntity::ColliderEntity(Mesh * mesh, Material * material)
//	: ColliderEntity(new DrawMesh(mesh, nullptr, DXInfo::getInstance().device))
//{
//	_shape->material(material);
//}

ColliderEntity::ColliderEntity(shared<DrawMesh> s)
	: Entity(s), _collider(make_unique<Collider>(s.get()->mesh(), &transform))
{
	CollisionManager::getInstance().addEntity(this);
}

ColliderEntity::ColliderEntity(vec3 p, vec3 dims, vec3 sc, vec3 rA, float r, shared<DrawMesh> s)
	: Entity(p, sc, rA, r, s), _collider(make_unique<Collider>(s.get()->mesh(), &transform))
{
	CollisionManager::getInstance().addEntity(this);
}

Collider* ColliderEntity::collider() const { return _collider.get(); }

void ColliderEntity::update(double delta) {
	auto dt = (float)delta;
	calcForces(delta);
	body.update(delta);
	transform.position += body.vel() * dt;
	transform.rotate(body.angVel() * dt);
	_collider->update();
	assert(!NaN_CHECK(transform.position.x));
}

void ColliderEntity::calcForces(double dt) {
	//body.applyGravity();
	
	//collision resolution stuff here
	body.netForce    += body.quadDrag(-0.5f, body.vel(), body.heading());// quadratic drag, no mass involved, it's all velocity dependent
	body.netAngAccel += body.quadDrag(-10.f, body.angVel(), body.angHeading());// for ang accel too

	//body.netForce *= body.invMass();
}

//override this (and preferably call it) to change on-collision behavior
//decrement the numCollisions counter if this collision is considered "resolved" without actually resolving the collision
void ColliderEntity::handleCollision(ColliderEntity* other, Manifold& m, double dt, size_t& numCollisions) {
	auto& oRB = other->rigidBody;

	//if the two bodies are travelling in the same direction along the axis
	auto velAlongAxis = glm::dot(oRB.vel() - body.vel(), m.axis);
	if (velAlongAxis > 0) return;

	//coefficient of restitution. we take the min of the two coeffs
	//when e = 0, it is a perfect inelastic/plastic collision, and the objects stick together
	//when 0 < e < 1, it is a regular inelastic collision, with some energy dissipated
	//when e = 1, it is an elastic collision, where all energy is put into the response
	float e = minf(body.restitution(), oRB.restitution());

	auto j = velAlongAxis;// magnitude of impulse
	j *= -(1 + e);
	j /= body.invMass() + oRB.invMass();

	//vec3 impulse = j * m.axis;
	//float massRatio = mass / (mass + other->mass);
	//_vel -= massRatio * impulse;
	//massRatio *= other->mass * invMass;
	//other->vel(massRatio * impulse);

	//F is the force applied by the collision; we use the definition F = dp / dt, where p = momentum and dp = impulse
	j /= (float)dt;
	auto F = j * m.axis;
	body.netForce +=  F;
	oRB.netForce  += -F;
	DrawDebug::getInstance().drawDebugVector(_transform.position, _transform.position() + F);
	DrawDebug::getInstance().drawDebugVector(other->transform.position, other->transform.position() - F);

	//they have the same collision points by definition, but vecs to those points change, meaning torque and covariance also change
	body.netAngAccel += calcAngularAccel(m, F);
	oRB.netAngAccel += other->calcAngularAccel(m, -F);

	//correct positions
	const auto percent = 1.2f, slop = 0.05f;
	auto correction = maxf(-m.pen - slop, 0.0f) * percent * (1 + body.fixed() + oRB.fixed()) / (body.invMass() + oRB.invMass()) * m.axis;

	transform.position        -= (body.invMass() + oRB.fixed()  * oRB.invMass())  * (1 - body.fixed()) * correction;
	other->transform.position += (oRB.invMass()  + body.fixed() * body.invMass()) * (1 - oRB.fixed())  * correction;

	assert(!NaN_CHECK(transform.position().x));
	assert(!NaN_CHECK(other->transform.position().x));

	--numCollisions;// YIKES
}

//Given a collision force F, calculates the change in angular acceleration it causes
vec3 ColliderEntity::calcAngularAccel(Manifold& m, vec3 F) {
	vec3 torque;
	if (!m.colPoints.size() || body.fixed())
		return torque;

	mat3 C;// mass-weighted covariance
	//assumes uniform mass distribution; we can account for non-uniform distributions with constraints
	auto m_n = body.mass() / m.colPoints.size();
	for (auto& colPoint : m.colPoints) {
		auto r = colPoint - _collider->framePos();//vector from the center of mass to the collision point
		torque += glm::cross(r, F);//torque = r x F = |r||F|sin(theta)
		C += m_n * mat3(r.x * r, r.y * r, r.z * r);//m_n * r * r_transpose
	}
	auto trace_C = C[0][0] + C[1][1] + C[2][2];

	auto iT = mat3() * trace_C - C;//inertia tensor = IdGameEntity_3x3 * trace(C) - C

	auto at_iT = vec3(m.axis.x * (iT[0][0] + iT[0][1] + iT[0][2])
		            , m.axis.y * (iT[1][0] + iT[1][1] + iT[1][2])
		            , m.axis.z * (iT[2][0] + iT[2][1] + iT[2][2]));//axis_transpose * inertia tensor (matrices are column major)

	auto inertia = glm::dot(at_iT, m.axis);//axis_transpose * iT * axis = (axis_transpose * inertia tensor) . axis
	return (inertia) ? torque / inertia : vec3();
}