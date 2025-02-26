#include "Solver.h"

#include "Core/Application.h"
#include "Scene/GameObject.h"

namespace Zero
{
    void TestSolver::Solve(std::vector<Collision>& collisions, float dt)
    {
        for (auto& [ObjA, ObjB, Points] : collisions)
        {
            // Calculate the relative velocity
            glm::vec3 RelativeVelocity = ObjB->GetDynamics().Velocity - ObjA->GetDynamics().Velocity;
            
            // Calculate the relative velocity in terms of the normal direction
            float VelocityAlongNormal = glm::dot(RelativeVelocity, Points.Normal);

            ObjA->GetDynamics().AddImpulse(Points.Normal * VelocityAlongNormal);
            ObjB->GetDynamics().AddImpulse(-Points.Normal * VelocityAlongNormal);

            // Solve the position error
            if (Points.Depth > -0.01f)
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
            else if (Points.Depth < 0.01f)
            {
                if (ObjA->EnableGravity)
                {
                    ObjA->GetTransform().Location -= Points.Normal * Points.Depth;
                }
                if (ObjB->EnableGravity)
                {
                    ObjB->GetTransform().Location += Points.Normal * Points.Depth;
                }
            }
        }
    }
}
