#include "CollisionManager.h"

#include <iostream>
#include "DebugBenchmark.h"

CollisionManager::CollisionManager() { 
	float d = 100.f; 
	//octTree = std::make_unique(new OctTree(vec3(), vec3(d, d, d))); 
}

void CollisionManager::addEntity(ColliderEntity* o) {
	//octTree->add(o);
	for (auto c : objects) {
		collisionPairs.push_back(std::pair<ColliderEntity*, ColliderEntity*>(o, c));
	}
	objects.push_back(o);
}

void CollisionManager::update(float dt) {
	//octTree->update();

	size_t numCollisions, maxIters = 8;
	do {
		collisionPairs = broadPhase();
		numCollisions = narrowPhase(dt);
	} while (numCollisions > 0 && --maxIters > 0);
}

void CollisionManager::draw() {
#if DEBUG
	//octTree->draw();
#endif
}

void CollisionManager::clear()
{
	//octTree->clear();
	objects = std::vector<ColliderEntity*>();
}

//returns a list of all pairs of colliders requiring narrow phase checks
collisionPairList CollisionManager::broadPhase() {
	//return octTree->checkCollisions();
	return collisionPairs;
}

//returns the number of collisions found and handled
size_t CollisionManager::narrowPhase(float dt) {
	size_t numCollisions = 0;
	for (auto& [a, b] : collisionPairs) {
		//DebugBenchmark::start();
		if (!(a->active && b->active) || !(a->rigidBody.solid() && b->rigidBody.solid()))
			continue;

		auto m = a->collider()->intersects(b->collider());
		if (m.originator) {
			if (m.originator == a->collider())
				a->handleCollision(b, m, dt, numCollisions);
			else
				b->handleCollision(a, m, dt, numCollisions);
			a->collider()->update();
			b->collider()->update();
			++numCollisions;

			std::cout << "collision! " << a->id << ", " << b->id << "; " << (m.originator == a->collider() ? a->id : b->id) << ", "
				<< m.pen << "; contact points: " << m.colPoints.size() << '\n';
		}
		//std::cout << "Collision Check Time: " << DebugBenchmark::end() << '\n';
	}
	return numCollisions;
}