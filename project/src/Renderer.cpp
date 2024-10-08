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

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	// Converting the m_Width & m_Height to float instead inside the loop gives 2 fps more
	// TODO: Benchmark this
	const float widthF = static_cast<float>(m_Width);
	const float heightF = static_cast<float>(m_Height);

	for (unsigned int px{}; px < m_Width; ++px)
	{
		float xCdn = ((((2 * (static_cast<float>(px) + 0.5f)) / widthF) - 1) * m_AspectRatio) * camera.GetFovValue();
		for (unsigned int py{}; py < m_Height; ++py)
		{
			// y Value for CDN
			float yCdn = (1 - 2 * ((static_cast<float>(py) + 0.5f) / heightF)) * camera.GetFovValue();
			const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

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
				// finalColor = materials[closestHit.materialIndex]->Shade();

				
				const std::vector<Light>& lights = pScene->GetLights();  // Pass by reference, no copy reduces stack and optimized

				for (size_t i{ 0 }; i < lights.size(); ++i)
				{
					const Vector3 lightRayOrigin = closestHit.origin + closestHit.normal * 0.0001f;
					const Vector3 lightRayDirection = LightUtils::GetDirectionToLight(lights[i], lightRayOrigin);

					float dotLight{ Vector3::Dot(closestHit.normal, lightRayDirection.Normalized()) };
					if(dotLight > 0)
						finalColor += LightUtils::GetRadiance(lights[i], closestHit.origin)  * materials[closestHit.materialIndex]->Shade() * dotLight;

					if (pScene->DoesHit({ lightRayOrigin, lightRayDirection.Normalized(), 0.0001f, lightRayDirection.Magnitude()}))
					{
						finalColor *= 0.5f;
					}
				}
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

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
