//External includes
#include "SDL.h"
#include "SDL_surface.h"


#include <execution>
//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

#define PARALLEL_EXECUTION

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	// Calculate AspectRatio for CDN
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

	// Calculate Samples positions, based on a grid in uniform distribution
	// https://en.wikipedia.org/wiki/Supersampling#Supersampling_patterns
	CalculateSamplePositions();

	// Calculates each samples color strength,
	// instead of static_casting each samples and each frame,
	// it's done here once and once IncreaseSamples() or DecreaseSamples() gets called
	CalculateSampleColorStrength();
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix& cameraToWorld = camera.CalculateCameraToWorld();
	const float fov = camera.GetFovValue();

#if defined(PARALLEL_EXECUTION)
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);
	for(uint32_t index{}; index < amountOfPixels; ++index) 
		pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i)
	{
		RenderPixel(pScene, i, fov, m_AspectRatio, cameraToWorld, camera.origin);
	});
#else
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	for(uint32_t pixelIndex{}; pixelIndex < amountOfPixels; pixelIndex++)
	{
		RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}


void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	// Initialize local variables once each 
	auto materials{pScene->GetMaterials()};
	const auto& lights = pScene->GetLights();
	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };
	ColorRGB finalColor{};

	for(const auto& s : m_SamplePositions)
	{
		// Calculate ray start pos in screen space based on samples positions
		// NOTE: The sample positions are grid based that is why only factor of 4 can be used
		const float rx{px + s.x}, ry{ py + s.y };

		// Convert screen space coordinates to NDC
		const float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
		const float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };

		Vector3 rayDirection{ cx, cy, 1 };
		rayDirection = cameraToWorld.TransformVector(rayDirection);
		rayDirection.Normalize();

		Ray viewRay{ cameraOrigin, rayDirection };
		HitRecord closestHit{};

		pScene->GetClosestHit(viewRay, closestHit);

		ColorRGB currentSampleColor{};
		if (closestHit.didHit)
		{
			for (const Light& light : lights)
			{
				ColorRGB currentLightColor{};

				// Add 0.0001 distance to prevents the model to cast shadows on itself
				const Vector3 lightRayOrigin{ closestHit.origin + closestHit.normal * 0.0001f };   
				const Vector3 lightRayDirection{ LightUtils::GetDirectionToLight(light, lightRayOrigin) };
				const Vector3 lightDirNormalized{ lightRayDirection.Normalized() };

				const Ray lightRay
				{
					lightRayOrigin,
					lightDirNormalized,
					0.0001f,
					light.type == LightType::Directional ? FLT_MAX : lightRayDirection.Magnitude()
				};

				// Check if shadow needs to be cast on current sample
				const bool shadowOnSample{ pScene->DoesHit(lightRay) && m_ShadowsEnabled };

				// Lambert cosine law
				const float ObservedArea{ Vector3::Dot(closestHit.normal, lightDirNormalized) };  

				// Different Render settings based on each mode
				switch (m_CurrentLightingMode)
				{
				case LightingMode::Combined:
					if (ObservedArea > 0)
					{
						const ColorRGB BRDF{
							materials[closestHit.materialIndex]->Shade(
								closestHit, 
								lightDirNormalized, 
								-rayDirection.Normalized()
							)
						};

						currentLightColor += LightUtils::GetRadiance(light, closestHit.origin) * BRDF * ObservedArea;
					}
					break;
				case LightingMode::ObservedArea:

					if (ObservedArea > 0)
						currentLightColor += ColorRGB(1, 1, 1) * ObservedArea;

					break;
				case LightingMode::Radiance:
					currentLightColor += LightUtils::GetRadiance(light, closestHit.origin);
					break;
				case LightingMode::BRDF:
					const ColorRGB BRDF{
						materials[closestHit.materialIndex]->Shade(
							closestHit, 
							lightDirNormalized, 
							-rayDirection.Normalized()
						)
					};

					currentLightColor += BRDF;
					break;
				}

				if(shadowOnSample)
					currentLightColor *= m_ShadowStrength;

				currentSampleColor += currentLightColor;
			}
		}

		finalColor += currentSampleColor * m_SampleColorStrength;
	}

	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	std::cout << "Current lighting mode: " << static_cast<int>(m_CurrentLightingMode) << std::endl;
	m_CurrentLightingMode = static_cast<LightingMode>((static_cast<int>(m_CurrentLightingMode) + 1) % static_cast<int>(LightingMode::TOTAL_MODES));
}

void Renderer::IncreaseMSAA()
{
	if(m_SampleAmount * 4 > m_MaxSampleAmount)
		return;

	m_SampleAmount *= 4;
	CalculateSamplePositions();

	CalculateSampleColorStrength();
}

void Renderer::DecreaseMSAA()
{
	if(m_SampleAmount / 4 < m_minSampleAmount)
		return;

	m_SampleAmount /= 4;
	CalculateSamplePositions();

	CalculateSampleColorStrength();
}

void Renderer::CalculateSampleColorStrength()
{
	m_SampleColorStrength = 1.f / static_cast<float>(m_SampleAmount);
}

void Renderer::CalculateSamplePositions()
{
	m_SamplePositions.clear();
	m_SamplePositions.reserve(m_SampleAmount);

	uint32_t sqrtSample = sqrt(m_SampleAmount);

	for (uint32_t y{}; y < sqrtSample; y++)
	{
		const float tempY = (y + .5f) / sqrtSample;
		for (uint32_t x{}; x < sqrtSample; x++)
		{
			const float tempX = (x + .5f) / sqrtSample;
			m_SamplePositions.emplace_back(tempX, tempY);
		}
	}

}
