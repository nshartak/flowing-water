#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Ray
{
public:
    Ray(glm::vec3 o, glm::vec3 r) :O{ o }, R{r} {}
    glm::vec3 compute(float t)const { return O + t * R; }

    
    static Ray ConstructRayFromPixel(glm::mat4 camera, int screenWidth, int screenHeight, float fov, glm::vec2 pixel);
    static bool RayTriangleIntersection(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Ray ray, glm::vec3& point);

    glm::vec3 O;//the origin
    glm::vec3 R;//the direction

    static bool isInside(glm::vec3 p, glm::vec3 normal, glm::vec3 a, glm::vec3 b, glm::vec3 c)//inside outside test
    {
        glm::vec3 e0 = a - c;
        glm::vec3 e1 = b - a;
        glm::vec3 e2 = c - b;
        glm::vec3 c0 = p - c;
        glm::vec3 c1 = p - a;
        glm::vec3 c2 = p - b;
        if (glm::dot(normal , glm::cross(e0, c0)) < 0)return false;
        if (glm::dot(normal , glm::cross(e1, c1)) < 0)return false;
        if (glm::dot(normal , cross(e2, c2)) < 0)return false;
        return true;
    }
};

