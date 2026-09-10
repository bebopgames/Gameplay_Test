/**
* Copyright (c) 2022 Yigsoft. All Rights Reserved.
*/

#pragma once

class Camera
{

public:
	Camera();
	~Camera();
	void OnUpdate( float deltaTime, DirectX::Keyboard& keyboard, DirectX::Mouse& mouse, DirectX::GamePad& gamepad );

private:
	void RotationInput( DirectX::Mouse& mouse, DirectX::GamePad& gamepad );
	Vector3 MovementInput( DirectX::Keyboard& keyboard, DirectX::GamePad& gamepad );

	Vector3 m_cameraPos;
	float m_pitch;
	float m_yaw;

};

