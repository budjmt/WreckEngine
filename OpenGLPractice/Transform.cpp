#include "Transform.h"



Transform::Transform()
	: position(tposition), scale(tscale), rotAxis(trotAxis), rotation(trotation), forward(tforward), up(tup), right(tright)
{
}

Transform::Transform(const Transform& other) 
	: position(tposition), scale(tscale), rotAxis(trotAxis), rotation(trotation), forward(tforward), up(tup), right(tright)
{
	position = other.position;
	scale = other.scale;
	rotAxis = other.rotAxis;
	rotation = other.rotation;
	forward = other.forward;
	up = other.up;
	right = other.right;
}


Transform::~Transform()
{
}

Transform& Transform::operator=(const Transform& other) 
{
	position = other.position;
	scale = other.scale;
	rotAxis = other.rotAxis;
	rotation = other.rotation;
	forward = other.forward;
	up = other.up;
	right = other.right;
}

Transform Transform::computeTransform() {
	if (!parent)
		return *this;
	Transform t = Transform();
	t.position = parent->position + position;
	t.scale = parent->scale * scale;
	//I'm not including this until quaternions
	//t.rotation = parent->rotation + rotation;
	t.rotAxis = rotAxis;
	t.rotation = rotation;
	t.parent = parent->parent;
	return t.computeTransform();
}

void Transform::updateNormals() {

}