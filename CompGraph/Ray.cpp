#include "Ray.h"
#include <iostream>
Ray Ray::ConstructRayFromPixel(glm::mat4 camera, int screenWidth, int screenHeight, float fov, glm::vec2 pixel)
{
    float alfa = glm::radians(fov);
    //NDC space
    float Px = (pixel[0] + 0.5f) / screenWidth;
    float Py = (pixel[1] + 0.5f) / screenHeight;
    //Screen space
    Px = 2 * Px - 1;
    Py = 1 - 2 * Py;
    //Account the aspect ratio
    Px = (Px * screenWidth) / screenHeight;

    //Account fov
    Px = Px * tan(alfa / 2);
    Py = Py * tan(alfa / 2);
    std::cout << Px << std::endl << Py << std::endl;
    glm::vec4 O = camera*glm::vec4(0.0, 0.0, 0.0, 1.0);
    glm::vec4 R = camera*glm::vec4(Px, Py, -1.0, 1.0);
    return Ray{glm::vec3(O.x, O.y, O.z), glm::vec3(R.x, R.y, R.z)- glm::vec3(O.x, O.y, O.z) };
}

bool Ray::RayTriangleIntersection(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, Ray ray, glm::vec3& point)
{

    glm::vec3 normal = glm::normalize(glm::cross(v0 - v2, v1 - v2));
    float D = glm::dot(normal, v0);//the distance to the plane from the origin
    float epsilion = 0.0001f;
    if (glm::dot(normal , ray.R) >= 0.0f && glm::dot(normal , ray.R) < epsilion)//if dot product is zero they don't intersect
    {
        return false;
    }


    float t = -(glm::dot(normal , ray.O) + D) / glm::dot(normal ,ray.R);
    if (t < 0)return false;
    point = ray.compute(t);
    return true;
}
