#include "Solver.h"

#include "Scene/GameObject.h"

namespace Zero
{
    void ImpulseSolver::Solve(std::vector<Collision>& collisions, float dt)
    {
        for (auto& [A, B, Points] : collisions)
        {
            if (!Points.HasCollision)
                continue;

            A->GetDynamics().AddImpulse(Points.Normal * Points.Depth);
            B->GetDynamics().AddImpulse(-Points.Normal * Points.Depth);
        }
    }
}
