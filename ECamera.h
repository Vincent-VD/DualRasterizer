/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert
/*=============================================================================*/
// ECamera.h: Base Camera Implementation with movement
/*=============================================================================*/

#pragma once
#include "EMath.h"

namespace Elite
{
	class Camera
	{
	public:

		static Camera* GetInstance();
		~Camera();
		static void CleanUp();

		//~Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		void Update(float elapsedSec);

		void SwitchCameraMode(RenderMode newRenderMode);

		void SetProjectionMatrix(RenderMode renderMode);
		void SetAspectRatio(uint32_t width, uint32_t height);

		const FMatrix4& GetWorldToView() const { return m_WorldToView; }
		const FMatrix4& GetViewToWorld() const { return m_ViewToWorld; }
		const FMatrix4& GetProjectionMatrix() const;

		FPoint3 GetPosition()
		{
			return m_Position;
		}

		const float GetFov() const { return m_Fov; }

	private:
		Camera(const FPoint3& position = { 0.f, 0.f, -10.f }, const FVector3& viewForward = { 0.f, 0.f, 1.f }, float fovAngle = 45.f);
		static Camera* m_Instance;

		void CalculateLookAt();

		float m_AspectRatio{};
		float m_Fov{};

		float m_KeyboardMoveSensitivity{ -10.f };
		const float m_KeyboardMoveMultiplier{ 10.f };
		const float m_MouseRotationSensitivity{ .1f };
		const float m_MouseMoveSensitivity{ 2.f };

		FPoint2 m_AbsoluteRotation{}; //Pitch(x) & Yaw(y) only
		FPoint3 m_RelativeTranslation{};

		FPoint3 m_Position{};
		FVector3 m_ViewForward{};
		FVector3 m_RightVector{};

		FMatrix4 m_WorldToView{};
		FMatrix4 m_ViewToWorld{};
		FMatrix4 m_ProjMatrix{};
	};
}
