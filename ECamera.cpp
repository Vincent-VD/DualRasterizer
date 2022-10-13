#include "pch.h"
#include "ECamera.h"
#include <SDL.h>

namespace Elite
{

	Camera* Camera::m_Instance = nullptr;

	Camera* Camera::GetInstance()
	{
		if (m_Instance == nullptr)
		{
			m_Instance = new Camera();
		}
		return m_Instance;
	}

	void Camera::CleanUp()
	{
		delete m_Instance;
	}

	Camera::~Camera()
	{
		m_Instance = nullptr;
	}

	Camera::Camera(const FPoint3& position, const FVector3& viewForward, float fovAngle) :
		m_Fov(tanf((fovAngle* float(E_TO_RADIANS)) / 2.f)),
		m_Position{ position },
		m_ViewForward{ GetNormalized(viewForward) },
		m_RightVector{ FVector3{ 0.f,1.f,0.f } }

	{
		//Calculate initial matrices based on given parameters (position & target)
		CalculateLookAt();
	}

	void Camera::Update(float elapsedSec)
	{
		//Capture Input (absolute) Rotation & (relative) Movement
		//*************
		//Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(0);
		float keyboardSpeed = pKeyboardState[SDL_SCANCODE_LSHIFT] ? m_KeyboardMoveSensitivity * m_KeyboardMoveMultiplier : m_KeyboardMoveSensitivity;
		m_RelativeTranslation.x = (pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) * -keyboardSpeed * elapsedSec;
		m_RelativeTranslation.y = 0;
		m_RelativeTranslation.z = (pKeyboardState[SDL_SCANCODE_S] - pKeyboardState[SDL_SCANCODE_W]) * keyboardSpeed * elapsedSec;

		//Mouse Input
		int x, y = 0;
		uint32_t mouseState = SDL_GetRelativeMouseState(&x, &y);
		if (mouseState == SDL_BUTTON_LMASK)
		{
			m_RelativeTranslation.z += y * m_MouseMoveSensitivity * elapsedSec;
			m_AbsoluteRotation.y += x * m_MouseRotationSensitivity;
		}
		else if (mouseState == SDL_BUTTON_RMASK)
		{
			m_AbsoluteRotation.x -= y * m_MouseRotationSensitivity;
			m_AbsoluteRotation.y -= x * m_MouseRotationSensitivity;
		}
		else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
		{
			m_RelativeTranslation.y += y * m_MouseMoveSensitivity * elapsedSec;
		}

		//Update LookAt (view2world & world2view matrices)
		//*************
		CalculateLookAt();
	}

	void Camera::CalculateLookAt()
	{
		//FORWARD (zAxis) with YAW applied
		FMatrix3 yawRotation = MakeRotationY(m_AbsoluteRotation.y * float(E_TO_RADIANS));
		FVector3 zAxis = yawRotation * m_ViewForward;

		//Calculate RIGHT (xAxis) based on transformed FORWARD
		FVector3 xAxis = GetNormalized(Cross(m_RightVector, zAxis));

		//FORWARD with PITCH applied (based on xAxis)
		FMatrix3 pitchRotation = MakeRotation(m_AbsoluteRotation.x * float(E_TO_RADIANS), xAxis);
		zAxis = pitchRotation * zAxis;

		//Calculate UP (yAxis)
		FVector3 yAxis = Cross(zAxis, xAxis);
		if (m_RightVector.y < -0.1f)
		{
			yAxis = -yAxis;
		}

		//Translate based on transformed axis
		m_Position += m_RelativeTranslation.x * xAxis;
		m_Position += m_RelativeTranslation.y * yAxis;
		m_Position += m_RelativeTranslation.z * zAxis;

		//Construct View2World Matrix
		m_ViewToWorld =
		{
			FVector4{xAxis},
			FVector4{yAxis},
			FVector4{zAxis},
			FVector4{m_Position.x,m_Position.y,m_Position.z,1.f}
		};

		//Construct World2View Matrix
		m_WorldToView = Inverse(m_ViewToWorld);
	}

	void Camera::SetAspectRatio(uint32_t width, uint32_t height)
	{
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
	}

	void Camera::SetProjectionMatrix(RenderMode renderMode)
	{
		switch (renderMode)
		{
		case RenderMode::HARDWARE:
			m_ProjMatrix[0][0] = (1 / (m_AspectRatio * m_Fov));
			m_ProjMatrix[1][1] = (1 / m_Fov);
			m_ProjMatrix[2][2] = 100.f / (100.f - 0.1f);
			m_ProjMatrix[3][2] = -(100.f * 0.1f) / (100.f - 0.1f);
			m_ProjMatrix[2][3] = 1.f;
			break;
		case RenderMode::SOFTWARE:
			m_ProjMatrix[0][0] = (1 / (m_AspectRatio * m_Fov));
			m_ProjMatrix[1][1] = (1 / m_Fov);
			m_ProjMatrix[2][2] = 100.f / (0.1f - 100.f);
			m_ProjMatrix[3][2] = (100.f * 0.1f) / (0.1f - 100.f);
			m_ProjMatrix[2][3] = -1.f;
			break;
		}
	}

	const FMatrix4& Camera::GetProjectionMatrix() const
	{
		return m_ProjMatrix;
	}

	void Camera::SwitchCameraMode(RenderMode newRenderMode)
	{
		switch (newRenderMode)
		{
		case RenderMode::HARDWARE:
			m_ViewForward = FVector3{ 0.f, 0.f, 1.f };
			m_RightVector = FVector3{ 0.f, 1.f, 0.f };
			break;
		case RenderMode::SOFTWARE:
			m_ViewForward = FVector3{ 0.f, 0.f, -1.f };
			m_RightVector = FVector3{ 0.f, -1.f, 0.f };
			break;
		}
		m_KeyboardMoveSensitivity = -m_KeyboardMoveSensitivity;
		SetProjectionMatrix(newRenderMode);
	}

}