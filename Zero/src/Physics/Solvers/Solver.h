#pragma once
#include <vector>

#include "Physics/Collision/Collider.h"

namespace Zero {

	class Solver
	{
		public:
		virtual ~Solver() = default;
		virtual void Solve(
				std::vector<Collision>& collisions,
				float dt) = 0;
	};
	
	class TestSolver : public Solver
	{
		public:
		void Solve(
				std::vector<Collision>& collisions,
				float dt) override;
	};
}

