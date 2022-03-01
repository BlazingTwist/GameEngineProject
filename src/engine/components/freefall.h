#ifndef ACAENGINE_FREEFALL_H
#define ACAENGINE_FREEFALL_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace components {
	struct FreeFallComponent
	{
	public:
		FreeFallComponent() {}
		
		FreeFallComponent(const double &gravity, float velocity) : _gravity(gravity), velocity(velocity){}
	
		double _gravity = 9.81;
		float velocity;
	
	};
}

#endif //ACAENGINE_FREEFALL_H
