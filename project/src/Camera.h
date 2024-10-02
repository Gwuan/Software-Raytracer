#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
			CalculateFov();
		}


		Vector3 origin{};

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		Vector3 worldUp{ Vector3::UnitY };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};


		float GetFovValue() const { return m_fovValue; }

		void SetFovAngle(float angle)  // Adjust angle of FOV in degrees
		{
			fovAngle = angle;
			CalculateFov();
		}

		void CalculateFov()
		{
			float fovAngleRad{ fovAngle / 180 * PI };
			m_fovValue = tan(fovAngleRad / 2);
		}

		Matrix CalculateCameraToWorld()  // This function should return the Camera ONB Matrix
		{
			this->right = Vector3::Cross(this->worldUp, this->forward).Normalized();
			this->up = Vector3::Cross(this->forward, this->right).Normalized();

			return { this->right, this->up, this->forward, this->origin };
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			HandleKeyboardInput(deltaTime);
			HandleMouseInput(deltaTime);

			const Matrix finalRotation = Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);

			this->forward = finalRotation.TransformVector(Vector3::UnitZ);
			this->forward.Normalize();
		}

		void HandleKeyboardInput(const float elapsedTime)
		{
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			// Forward & backward
			if (pKeyboardState[SDL_SCANCODE_W])
				this->origin += this->forward * (m_MoveSpeed * elapsedTime);
			if (pKeyboardState[SDL_SCANCODE_S])
				this->origin -= this->forward * (m_MoveSpeed * elapsedTime);

			// Right & Left
			if (pKeyboardState[SDL_SCANCODE_D])
				this->origin += this->right * (m_MoveSpeed * elapsedTime);
			if (pKeyboardState[SDL_SCANCODE_A])
				this->origin -= this->right * (m_MoveSpeed * elapsedTime);
		}

		void HandleMouseInput(const float elapsedTime)
		{
			// Get mouse input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState == SDL_BUTTON(1))  // Left Mouse Button
			{
				this->origin += this->forward * ((mouseY * -1) * m_MoveSpeed) * elapsedTime;
				this->totalYaw += mouseX * m_RotationSpeed * elapsedTime;
			}

			if (mouseState == SDL_BUTTON(3))  // Right Mouse Button
			{
				this->totalPitch += (mouseY * -1) * m_RotationSpeed * elapsedTime;
				this->totalYaw += mouseX * m_RotationSpeed * elapsedTime;
			}

			if (mouseState == SDL_BUTTON(1) + SDL_BUTTON(3))  // Left & Right button
			{
				this->origin += worldUp * ((mouseY * -1) * m_MoveSpeed * elapsedTime);
			}
		}


	private:
		float fovAngle{ 90.f };
		float m_fovValue{};  // TODO: Create a better identifier

		const float m_MoveSpeed = 5.f;
		const float m_RotationSpeed = 0.2f;
	};
}