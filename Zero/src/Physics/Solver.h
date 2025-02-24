#pragma once
#include <vector>
#include <Physics/Collision.h>

namespace Zero {

	class Solver
	{
		public:
			virtual void Solve(
				std::vector<Collision>& collisions,
				float dt) = 0;
	};

}

