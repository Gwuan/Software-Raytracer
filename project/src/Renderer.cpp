//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	// Calculate AspectRatio for CDN
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
}

void applyToneMapping(ColorRGB& color) {
	// Calculate the luminance
	float luminance = 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;

	// Reinhard tone mapping
	float mappedValue = luminance / (luminance + 1.0f);

	// Apply the tone mapping to each channel
	color.r = std::min(color.r / (luminance + 1.0f), 1.0f);
	color.g = std::min(color.g / (luminance + 1.0f), 1.0f);
	color.b = std::min(color.b / (luminance + 1.0f), 1.0f);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	// Converting the m_Width & m_Height to float instead inside the loop gives 2 fps more
	// TODO: Benchmark this
	const float widthF = static_cast<float>(m_Width);
	const float heightF = static_cast<float>(m_Height);

	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };
	const float fovValue = camera.GetFovValue();

	for (unsigned int px{}; px < m_Width; ++px)
	{
		float xCdn = ((((2 * (static_cast<float>(px) + 0.5f)) / widthF) - 1) * m_AspectRatio) * fovValue;
		for (unsigned int py{}; py < m_Height; ++py)
		{
			// y Value for CDN
			float yCdn = (1 - 2 * ((static_cast<float>(py) + 0.5f) / heightF)) * fovValue;

			// Creating the rayDirection
			Vector3 rayDirection{ xCdn, yCdn, 1 };
			rayDirection = cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();

			Ray viewRay{ camera.origin, rayDirection };

			ColorRGB finalColor{};

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);
			if (closestHit.didHit)
			{
				for (const Light& light : lights)
				{
					const Vector3 lightRayOrigin{ closestHit.origin + closestHit.normal * 0.0001f };
					const Vector3 lightRayDirection{ LightUtils::GetDirectionToLight(light, lightRayOrigin) };
					const Vector3 lightDirNormalized{lightRayDirection.Normalized()};

					const Ray lightRay{
						lightRayOrigin,
						lightDirNormalized,
						0.0001f,
						lightRayDirection.Magnitude()
					};

					// 1 for no shadow
					const float shadow = (pScene->DoesHit(lightRay) && m_ShadowsEnabled) ? 0.6f : 1.f; 

					const float ObserveredArea{ Vector3::Dot(closestHit.normal, lightDirNormalized) };  // Lambert cosine law
					const ColorRGB BRDF{ materials[closestHit.materialIndex]->Shade(closestHit, lightDirNormalized, -rayDirection.Normalized()) };


					switch (m_CurrentLightingMode)
					{
					case LightingMode::Combined:
						if (ObserveredArea > 0)
							finalColor += LightUtils::GetRadiance(light, closestHit.origin) * BRDF * ObserveredArea;
						break;
					case LightingMode::ObservedArea:
						if (ObserveredArea > 0)
							finalColor += ColorRGB(1,1,1) * ObserveredArea;	
						break;
					case LightingMode::Radiance:
						finalColor += LightUtils::GetRadiance(light, closestHit.origin);
						break;
					case LightingMode::BRDF:
						finalColor += BRDF;
						break;
					}


					finalColor *= shadow;
				}
			}

			//Update Color in Buffer
			finalColor.MaxToOne();
			//applyToneMapping(finalColor);

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));

		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}



bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	std::cout << static_cast<int>(m_CurrentLightingMode) << std::endl;
	m_CurrentLightingMode = LightingMode((static_cast<int>(m_CurrentLightingMode) + 1) % static_cast<int>(LightingMode::TOTAL_MODES));
}
