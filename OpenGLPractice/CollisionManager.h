#pragma once

//#include "OctTree.h"
#include "ColliderEntity.h"

typedef std::vector<std::pair<ColliderEntity*, ColliderEntity*>> collisionPairList;

class CollisionManager
{
public:
	static CollisionManager& getInstance() { static CollisionManager instance; return instance; }

	void addEntity(ColliderEntity* o);
	void update(float dt);
	void draw();
	void clear();

	collisionPairList broadPhase();
	uint32_t narrowPhase(float dt);

private:
	CollisionManager();
	//~CollisionManager();

	//OctTree* octTree;
	std::vector<ColliderEntity*> objects;
	collisionPairList collisionPairs;

};
