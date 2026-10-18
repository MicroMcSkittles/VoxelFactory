#include "Game/Animation.h"
#include "Core/Utils.h"

Animator::Animator(Transform& transform) : m_Transform(transform), m_LastTransform(transform) {
	m_CurrentFrameIndex = 0;
	m_FrameTime = 0.0f;
	m_Speed = 1.0f;
	m_Playing = false;
	m_IsLooped = false;
}

void Animator::AddAnimation(const std::string& name, const Animation& animation) {
	ASSERT(!m_Animations.count(name));
	m_Animations.insert({ name, animation });
}

void Animator::Play(const std::string& name, float speed, bool loop) {
	ASSERT(m_Animations.count(name));
	m_CurrentAnimation = name;

	if (m_CurrentFrameIndex == -1) {
		m_LastTransform = m_CurrentKeyFrame.transform;
	}
	else m_LastTransform = m_Transform;
	
	m_CurrentFrameIndex = 0;
	m_FrameTime = 0.0f;
	m_Speed = speed;

	Animation& current_animation = m_Animations[m_CurrentAnimation];
	if (!current_animation.key_frames.empty()) m_CurrentKeyFrame = current_animation.key_frames[0];
	m_IsLooped = current_animation.looping || loop;

	m_Playing = true;
}
void Animator::SetSpeed(float speed) {
	m_Speed = speed;
}
void Animator::Stop() {
	m_Playing = false;
	Animation& current_animation = m_Animations[m_CurrentAnimation];
	if (current_animation.end_frame.has_value()) {
		m_LastTransform = m_Transform;
		m_CurrentFrameIndex = -1;
		m_CurrentKeyFrame = current_animation.end_frame.value();
		m_FrameTime = 0.0f;
		return;
	}

	if (current_animation.function.has_value()) {
		m_Transform = m_LastTransform;
	}
	m_CurrentAnimation = "";
}

void Animator::Update(float delta_time) {
	if (m_CurrentAnimation.empty()) return;

	Animation& current_animation = m_Animations[m_CurrentAnimation];
	if (!current_animation.function.has_value() || m_CurrentFrameIndex == -1) UpdateKeyFrameAnimation(current_animation, delta_time);
	else UpdateFunctionAnimation(current_animation, delta_time);
}

bool Animator::IsPlaying(const std::string& name) const {
	return m_Playing && (m_CurrentAnimation == name);
}

void Animator::UpdateKeyFrameAnimation(Animation& animation, float delta_time) {
	float target_time = m_CurrentKeyFrame.time / m_Speed;
	float time = m_FrameTime / target_time;
	m_Transform = m_CurrentKeyFrame.easing_function(m_LastTransform, m_CurrentKeyFrame.transform, time);

	m_FrameTime += delta_time;
	// Check if at the key frame
	if (m_FrameTime >= target_time) {

		// End frame after stop
		if (m_CurrentFrameIndex == -1) {
			m_CurrentAnimation = "";
			m_FrameTime = 0.0f;
			m_LastTransform = m_CurrentKeyFrame.transform;
			m_Transform = m_CurrentKeyFrame.transform;
			m_CurrentFrameIndex = 0;
			return;
		}

		// At the last frame
		if (m_CurrentFrameIndex == animation.key_frames.size() - 1) {
			m_CurrentFrameIndex = 0;
			if (!m_IsLooped) {
				Stop();
				return;
			}
		}
		else m_CurrentFrameIndex += 1;

		m_FrameTime = 0.0f;
		m_LastTransform = m_CurrentKeyFrame.transform;
		m_CurrentKeyFrame = m_Animations[m_CurrentAnimation].key_frames[m_CurrentFrameIndex];
	}
}

void Animator::UpdateFunctionAnimation(Animation& animation, float delta_time) {

	Transform offset = animation.function.value()(m_Speed, m_FrameTime);
	offset.position += m_LastTransform.position;
	offset.rotation += m_LastTransform.rotation;
	offset.scale    += m_LastTransform.scale;

	// This probably isn't the best way to do this... I don't want to find a better way rn though
	m_Transform.position += (offset.position - m_Transform.position) * 10.0f * delta_time;
	m_Transform.rotation += (offset.rotation - m_Transform.rotation) * 10.0f * delta_time;
	m_Transform.scale    += (offset.scale    - m_Transform.scale)    * 10.0f * delta_time;

	m_FrameTime += delta_time;
}

void KeyFrame::LoadFromJSON(const JSON::Node& data) {
	ASSERT(!data.IsNull());

	const JSON::Node& transform_data = data["Transform"];
	ASSERT(!transform_data.IsNull());
	transform.position = (glm::vec3)transform_data["Position"];
	transform.rotation = (glm::vec3)transform_data["Rotation"];
	transform.scale = (glm::vec3)transform_data["Scale"];

	const JSON::Node& time_data = data["Time"];
	ASSERT(!time_data.IsNull());
	time = (float)time_data;

	const JSON::Node& easing_function_data = data["EasingFunction"];
	if (easing_function_data.IsNull()) {
		easing_function = Transform::LinearInterp;
		return;
	}
	std::string easing_function_type = (std::string)easing_function_data;
	if (easing_function_type == "Linear") easing_function = Transform::LinearInterp;
	else if (easing_function_type == "Cubic") easing_function = Transform::CubicInterp;
	else easing_function = Transform::LinearInterp;
}
void Animation::LoadFromJSON(const JSON::Node& data) {
	ASSERT(!data.IsNull());

	const JSON::Node& key_frames_data = data["KeyFrames"];
	ASSERT(!key_frames_data.IsNull());
	for (const JSON::Node& key_frame_data : key_frames_data.children_array) {
		KeyFrame frame;
		frame.LoadFromJSON(key_frame_data);
		key_frames.push_back(frame);
	}

	const JSON::Node& end_frame_data = data["EndFrame"];
	if (!end_frame_data.IsNull()) {
		KeyFrame frame;
		frame.LoadFromJSON(end_frame_data);
		end_frame = frame;
	}

	const JSON::Node& looping_data = data["Looping"];
	if (!looping_data.IsNull()) looping = (bool)looping_data;
}
