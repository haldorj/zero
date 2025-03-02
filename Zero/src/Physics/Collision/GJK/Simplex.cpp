#include "Simplex.h"

namespace Zero
{
    bool Line(Simplex& points, glm::vec3& direction)
    {
        const glm::vec3 a = points[0];
        const glm::vec3 b = points[1];

        const glm::vec3 ab = b - a;
        const glm::vec3 ao = -a;

        if (SameDirection(ab, ao))
        {
            direction = glm::cross(glm::cross(ab, ao), ab);
        }
        else
        {
            points = {a};
            direction = ao;
        }
        return false;
    }

    bool Triangle(Simplex& points, glm::vec3& direction)
    {
        const glm::vec3 a = points[0];
        const glm::vec3 b = points[1];
        const glm::vec3 c = points[2];

        const glm::vec3 ab = b - a;
        const glm::vec3 ac = c - a;
        const glm::vec3 ao = -a;

        const glm::vec3 abc = glm::cross(ab, ac);

        if (SameDirection(glm::cross(abc, ac), ao))
        {
            if (SameDirection(ac, ao))
            {
                points = {a, c};
                direction = glm::cross(glm::cross(ac, ao), ac);
            }
            else
            {
                return Line(points = {a, b}, direction);
            }
        }
        else
        {
            if (SameDirection(glm::cross(ab, abc), ao))
            {
                return Line(points = {a, b}, direction);
            }

            if (SameDirection(abc, ao))
            {
                direction = abc;
            }
            else
            {
                points = {c, b, a};
                direction = -abc;
            }
        }
        return false;
    }

    bool Tetrahedron(Simplex& points, glm::vec3& direction)
    {
        const glm::vec3 a = points[0];
        const glm::vec3 b = points[1];
        const glm::vec3 c = points[2];
        const glm::vec3 d = points[3];

        const glm::vec3 ab = b - a;
        const glm::vec3 ac = c - a;
        const glm::vec3 ad = d - a;
        const glm::vec3 ao = -a;

        const glm::vec3 abc = glm::cross(ab, ac);
        const glm::vec3 acd = glm::cross(ac, ad);
        const glm::vec3 adb = glm::cross(ad, ab);

        if (SameDirection(abc, ao))
        {
            return Triangle(points = {a, b, c}, direction);
        }
        if (SameDirection(acd, ao))
        {
            return Triangle(points = {a, c, d}, direction);
        }
        if (SameDirection(adb, ao))
        {
            return Triangle(points = {a, d, b}, direction);
        }

        return true;
    }
}