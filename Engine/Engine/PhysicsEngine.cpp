#include "PhysicsEngine.h"
/*
PhysicsEngine::~PhysicsEngine()
{
}


void PhysicsEngine::ApplyCollision(PhysicsObject* objectOne, PhysicsObject*  objectTwo)
{
	if (objectOne->isAtRest && objectTwo->isAtRest)
		return;
	//TODO this should never happen;


	if (objectOne->isConstant)
	{
		float restitution = (objectOne->restitution + objectTwo->restitution) / 2; //TODO this isn't right
		objectTwo->velocity = XMFLOAT3(-objectTwo->velocity.x*restitution, -objectTwo->velocity.y*restitution, -objectTwo->velocity.z*restitution);
	}
	else if (objectTwo->isConstant)
	{
		float restitution = (objectOne->restitution + objectTwo->restitution) / 2; //TODO this isn't right
		objectOne->velocity = XMFLOAT3(-objectOne->velocity.x*restitution, -objectOne->velocity.y*restitution, -objectOne->velocity.z*restitution);
	}
	else {
		XMFLOAT3 vel1b = objectOne->velocity;
		XMFLOAT3 vel2b = objectTwo->velocity;
		float mass1 = objectOne->mass;
		float mass2 = objectTwo->mass;
		float rest = (objectOne->restitution+objectTwo->restitution)/2; //TODOD not physically accurate
		
		XMFLOAT3 vel1a = XMFLOAT3(
			(vel1b.x*mass1 + vel2b.x*mass2 + (mass2*rest)*(vel2b.x - vel1b.x)) / (mass1 + mass2),
			(vel1b.y*mass1 + vel2b.y*mass2 + (mass2*rest)*(vel2b.y - vel1b.y)) / (mass1 + mass2),
			(vel1b.z*mass1 + vel2b.z*mass2 + (mass2*rest)*(vel2b.z - vel1b.z)) / (mass1 + mass2));
		XMFLOAT3 vel2a = XMFLOAT3(
			(vel1b.x*mass1 + vel2b.x*mass2 + (mass1*rest)*(vel1b.x - vel2b.x)) / (mass1 + mass2),
			(vel1b.y*mass1 + vel2b.y*mass2 + (mass1*rest)*(vel1b.y - vel2b.y)) / (mass1 + mass2),
			(vel1b.z*mass1 + vel2b.z*mass2 + (mass1*rest)*(vel1b.z - vel2b.z)) / (mass1 + mass2));
		if (!objectOne->applied)
		{
			objectOne->velocity = vel1a;
			//objectOne->applied = true; //TODO
		}
		if (!objectTwo->applied)
		{
			objectTwo->velocity = vel2a;
			//objectTwo->applied = true;
		}
	}
	CheckVelocity(objectOne);
	CheckVelocity(objectTwo);
}

PhysicsEngine::PhysicsEngine()
{
}

void PhysicsEngine::Update(Object* object, const std::vector<Object*>& objects, const GameTimer& gt)
{
	deltaTime = gt.DeltaTime();
	if (deltaTime > .005f)
	{
		deltaTime = .005f;
	}
	PhysicsObject* col = CheckCollision(object->po.get(), objects);
	if (object->po->isAtRest)
		return;

	if (col != nullptr && col != object->po.get())
	{
		ApplyCollision(object->po.get(), col);
		UINT temp = 0;
		while (CheckCollision(object->po.get(), col)) {
			ApplyVelocity(object);
			temp++;
			if (temp >= 500)
				break;
		}
	}
	ApplyGravity(object->po.get());
	ApplyVelocity(object);

	if(col!=nullptr)
		CheckVelocity(object->po.get());
}

void PhysicsEngine::Reset(const std::vector<Object*>& objects)
{
	for (auto& e : objects)
	{
		e->po->applied = false;
	}
}

void PhysicsEngine::ApplyGravity(PhysicsObject* object)
{
	float newY = object->velocity.y + gravity*std::pow(deltaTime, 2);
	object->velocity = XMFLOAT3(object->velocity.x, newY, object->velocity.z);
}

PhysicsObject* PhysicsEngine::CheckCollision(PhysicsObject* object, const std::vector<Object*>& objects)
{
	// TODO: insert return statement here
	for (size_t i = 0; i < objects.size(); ++i)
	{
		auto obj = objects[i];
		if (object->bounds.Contains(obj->po->bounds) != DirectX::DISJOINT)
		{
			if (obj->po.get() == object)
				continue;
			return obj->po.get();
		}
	}
	return nullptr;
}


void PhysicsEngine::AdjustPosition(Object* object, float x, float y, float z)
{
	if (!object->po->isConstant)
	{
		float xt = object->po->bounds.Extents.x + x;
		float yt = object->po->bounds.Extents.y + y;
		float zt = object->po->bounds.Extents.z + z;
		object->po->bounds.Center = XMFLOAT3(xt, yt, zt);
		//TODO only for scale one rotated
		object->ri->World = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			x, y, z, 1.0f);
		object->ri->NumFramesDirty++;
	}
}

void PhysicsEngine::ApplyVelocity(Object* object)
{
	if (!object->po->isConstant && !object->po->isAtRest)
	{
		float x = object->po->bounds.Center.x + object->po->velocity.x*deltaTime;
		float y = object->po->bounds.Center.y + object->po->velocity.y*deltaTime;
		float z = object->po->bounds.Center.z + object->po->velocity.z*deltaTime;

		object->po->bounds.Center = XMFLOAT3(x, y, z);
		float worldPosX = x;
		float worldPosY = y - object->po->bounds.Extents.y;
		float worldPosZ = z;
		object->ri->World = XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			worldPosX, worldPosY, worldPosZ, 1.0f);
		object->ri->NumFramesDirty++;
	}
}

void PhysicsEngine::CheckVelocity(PhysicsObject* object)
{
	float x = object->velocity.x;
	float y = object->velocity.y;
	float z = object->velocity.z;
	if (std::abs(x) <= .05f)
	{
		x = 0;
	}
	if (std::abs(y) <= .05f)
	{
		y = 0;
	}
	if (std::abs(z) <= .05f)
	{
		z = 0;
	}
	object->velocity = XMFLOAT3(x, y, z);
	if (x == 0 && y == 0 && z == 0)
	{
		object->isAtRest = true;
	}
	else {
		object->isAtRest = false;
	}
}

bool PhysicsEngine::CheckCollision(PhysicsObject* objectOne, PhysicsObject* objectTwo)
{
	if (objectOne->bounds.Contains(objectTwo->bounds))
		return true;
	return false;
}
*/