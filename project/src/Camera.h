#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

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

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};


		float GetFovAmount() const { return fovAmount; }

		void SetFovAngle(float angle)  // Adjust angle of FOV in degrees
		{
			fovAngle = angle;
			CalculateFov();
		}

		void CalculateFov()
		{
			float fovAngleRad{ fovAngle / 180 * PI };
			fovAmount = fovAngleRad / 2;
		}

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//throw std::runtime_error("Not Implemented Yet");
		}

	private:
		float fovAngle{ 90.f };
		float fovAmount{};  // TODO: Create a better identifier

	};
}
