#include "Solver.h"

#include "Core/Application.h"
#include "Scene/GameObject.h"

namespace Zero
{
    void ImpulseSolver::Solve(std::vector<Collision>& collisions, float dt)
    {
        for (auto& [A, B, Points] : collisions)
        {
            if (!Points.HasCollision)
                continue;

            Application::Get().DestroyGameObject(A->GetID());
        }
    }
}
