#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <optional>

#include "Game/World/Entity.h"
#include "Core/JSON.h"

struct KeyFrame {
	Transform transform;
	float time = 0.0f;

	// Used to ease last key frame to the current one
	std::function<Transform(const Transform& t1, const Transform& t2, float time)> easing_function = Transform::LinearInterp;

	void LoadFromJSON(const JSON::Node& data);
};
struct Animation {
	std::vector<KeyFrame> key_frames;
	std::optional<std::function<Transform(float speed, float time)>> function; // Applied to the transform if set
	std::optional<KeyFrame> end_frame; // When animation stops it moves to this frame if set
	bool looping = false;

	void LoadFromJSON(const JSON::Node& data);
};

class Animator {
public:
	Animator(Transform& transform);
	~Animator() { }

	void AddAnimation(const std::string& name, const Animation& animation);

	void Play(const std::string& name, float speed = 1.0f, bool loop = false);
	void SetSpeed(float speed);
	void Stop();

	void Update(float delta_time);

	std::string GetCurrentAnimationName() const { return m_CurrentAnimation; }
	int GetCurrentFrameIndex() const { return m_CurrentFrameIndex; }
	float GetCurrentFrameTime() const { return m_FrameTime; }
	bool Playing() const { return m_Playing; }

	bool IsPlaying(const std::string& name) const;

private:
	void UpdateKeyFrameAnimation(Animation& animation, float delta_time);
	void UpdateFunctionAnimation(Animation& animation, float delta_time);

private:
	Transform m_LastTransform;
	Transform& m_Transform;
	std::unordered_map<std::string, Animation> m_Animations;

	std::string m_CurrentAnimation;
	KeyFrame m_CurrentKeyFrame;
	int m_CurrentFrameIndex;
	bool m_Playing;
	bool m_IsLooped;
	float m_FrameTime;
	float m_Speed;
};