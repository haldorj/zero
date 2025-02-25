#pragma once
#include <vector>
#include <Physics/Collision.h>

namespace Zero {

	class Solver
	{
		public:
		virtual ~Solver() = default;
		virtual void Solve(
				std::vector<Collision>& collisions,
				float dt) = 0;
	};

	class ImpulseSolver : public Solver
	{
		public:
			void Solve(
				std::vector<Collision>& collisions,
				float dt) override;
	};
}

