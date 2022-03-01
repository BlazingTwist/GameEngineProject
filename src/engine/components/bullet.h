#ifndef ACAENGINE_BULLETCOMPONENT_H
#define ACAENGINE_BULLETCOMPONENT_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <engine/entity/entityregistry.h>

namespace components {
	struct BulletComponent
	{
	public:

		BulletComponent() {}
		BulletComponent(const bool isbullet) :_isBullet(_isBullet) {}
		
	
	


	bool _isBullet;
	};
}

#endif //ACAENGINE_BULLETCOMPONENT_H
