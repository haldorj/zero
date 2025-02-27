#include "Solver.h"

#include "Core/Application.h"
#include "Scene/GameObject.h"

namespace Zero
{
    void ImpulseSolver::Solve(std::vector<Collision>& collisions, float dt)
    {
        for (auto& [ObjA, ObjB, Points] : collisions)
        {
            // Calculate the relative velocity
            glm::vec3 relativeVelocity = ObjB->GetDynamics().Velocity - ObjA->GetDynamics().Velocity;
            
            // Calculate the relative velocity in terms of the normal direction
            const float velocityAlongNormal = glm::dot(relativeVelocity, Points.Normal);
            
            if (ObjA->EnableGravity)
                ObjA->GetDynamics().AddImpulse(Points.Normal * velocityAlongNormal);
            if (ObjB->EnableGravity)
                ObjB->GetDynamics().AddImpulse(-Points.Normal * velocityAlongNormal);
        }
    }

    void PositionSolver::Solve(std::vector<Collision>& collisions, float dt)
    {
        for (auto& [ObjA, ObjB, Points] : collisions)
        {
            if (ObjA->EnableGravity)
            {
                ObjA->GetTransform().Location += Points.Normal * Points.Depth;
            }
            if (ObjB->EnableGravity)
            {
                ObjB->GetTransform().Location -= Points.Normal * Points.Depth;
            }
        }
    }
}
