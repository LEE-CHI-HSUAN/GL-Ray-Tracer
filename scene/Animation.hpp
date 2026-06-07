#pragma once
#include <vector>
#include <algorithm>
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/quaternion.hpp"

/**
 * @struct Keyframe
 * @brief Represents a transformation state at a specific time in an animation.
 */
struct Keyframe
{
    float time;                       // Time in seconds
    glm::vec3 position;               // Position in world space
    glm::vec3 rotation;               // Euler angles in degrees (Pitch, Yaw, Roll)
    glm::vec3 scale = glm::vec3(1.0); // Scale factors
};

/**
 * @class Animation
 * @brief Manages a sequence of keyframes and provides interpolated transformation matrices.
 */
class Animation
{
public:
    std::vector<Keyframe> keyframes; // List of animation keyframes

    /**
     * @brief Adds a keyframe to the animation and sorts them by time.
     * @param kf The keyframe to add.
     */
    void addKeyframe(const Keyframe &kf)
    {
        keyframes.push_back(kf);
        std::sort(keyframes.begin(), keyframes.end(), [](const Keyframe &a, const Keyframe &b)
                  { return a.time < b.time; });
    }

    /**
     * @brief Interpolates the transformation matrix at the given time using SLERP for rotation.
     * @param time The time in seconds.
     * @return The interpolated transformation matrix.
     */
    glm::mat4 interpolate(float time) const
    {
        if (keyframes.empty())
            return glm::mat4(1.0f);
        if (time <= keyframes.front().time)
            return getTransform(keyframes.front());
        if (time >= keyframes.back().time)
            return getTransform(keyframes.back());

        for (size_t i = 0; i < keyframes.size() - 1; ++i)
        {
            if (time >= keyframes[i].time && time <= keyframes[i + 1].time)
            {
                float t = (time - keyframes[i].time) / (keyframes[i + 1].time - keyframes[i].time);
                return interpolateKeyframes(keyframes[i], keyframes[i + 1], t);
            }
        }
        return getTransform(keyframes.back());
    }

private:
    /**
     * @brief Calculates a transformation matrix from a single keyframe.
     * @param kf The keyframe to convert.
     * @return The resulting transformation matrix.
     */
    glm::mat4 getTransform(const Keyframe &kf) const
    {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), kf.position);
        // Euler to Quaternion (pitch, yaw, roll)
        glm::quat rot = glm::quat(glm::vec3(glm::radians(kf.rotation.x), glm::radians(kf.rotation.y), glm::radians(kf.rotation.z)));
        trans *= glm::mat4_cast(rot);
        trans = glm::scale(trans, kf.scale);
        return trans;
    }

    /**
     * @brief Interpolates between two keyframes at time t [0, 1].
     * @param a The start keyframe.
     * @param b The end keyframe.
     * @param t The interpolation factor.
     * @return The interpolated transformation matrix.
     */
    glm::mat4 interpolateKeyframes(const Keyframe &a, const Keyframe &b, float t) const
    {
        glm::vec3 pos = glm::mix(a.position, b.position, t);

        // Quaternion interpolation
        glm::quat rotA = glm::quat(glm::vec3(glm::radians(a.rotation.x), glm::radians(a.rotation.y), glm::radians(a.rotation.z)));
        glm::quat rotB = glm::quat(glm::vec3(glm::radians(b.rotation.x), glm::radians(b.rotation.y), glm::radians(b.rotation.z)));
        glm::quat rot = glm::slerp(rotA, rotB, t);

        glm::vec3 scl = glm::mix(a.scale, b.scale, t);

        glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
        trans *= glm::mat4_cast(rot);
        trans = glm::scale(trans, scl);
        return trans;
    }
};
