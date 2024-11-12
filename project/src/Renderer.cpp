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
}

void applyToneMapping(ColorRGB& color) {
	// Calculate the luminance
	float luminance = 0.2126f * color.r + 0.7152f * color.g + 0.0722f * color.b;

	// Reinhard tone mapping
	float mappedValue = luminance / (luminance + 1.0f);

	// Apply the tone mapping to each channel
	color.r = std::min(color.r / (mappedValue + 1.0f), 1.0f);
	color.g = std::min(color.g / (mappedValue + 1.0f), 1.0f);
	color.b = std::min(color.b / (mappedValue + 1.0f), 1.0f);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix& cameraToWorld = camera.CalculateCameraToWorld();

	const float aspectRatio = m_Width / static_cast<float>(m_Height);

	const float fov = camera.GetFovValue();

#if defined(PARALLEL_EXECUTION)
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};

	pixelIndices.reserve(amountOfPixels);
	for(uint32_t index{}; index < amountOfPixels; ++index) 
		pixelIndices.emplace_back(index);

	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i)
	{
		RenderPixel(pScene, i, fov, aspectRatio, cameraToWorld, camera.origin);
	});
#else
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	for(uint32_t pixelIndex{}; pixelIndex < amountOfPixels; pixelIndex++)
	{
		// std::cout << "current pixelIndex: " << pixelIndex << ", amount of pixels: " << amountOfPixels << " ";
		RenderPixel(pScene, pixelIndex, fov, aspectRatio, cameraToWorld, camera.origin);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

struct vector2
{
	float x;
	float y;
};

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const
{
	auto materials{pScene->GetMaterials()};

	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	const uint32_t sampleCount{ 4 };

	vector2 samples[sampleCount]{};
	uint32_t sqrtSample = sqrt(sampleCount);

	for (uint32_t y{}; y < sqrtSample; y++)
	{
		const float tempY = (y + .5f) / sqrtSample;
		for (uint32_t x{}; x < sqrtSample; x++)
		{
			const float tempX = (x + .5f) / sqrtSample;
			samples[x + (y * sqrtSample)] = { tempX, tempY };
		}
	}

	ColorRGB finalColor{};
	for(const auto& s : samples)
	{
		ColorRGB currentSampleColor{};

		float rx{px + s.x}, ry{ py + s.y };
		float cx{ (2 * (rx / float(m_Width)) - 1) * aspectRatio * fov };
		float cy{ (1 - (2 * (ry / float(m_Height)))) * fov };

		// inside the double loop
		Vector3 rayDirection{ cx, cy, 1 };
		rayDirection = cameraToWorld.TransformVector(rayDirection);
		rayDirection.Normalize();

		Ray viewRay{ cameraOrigin, rayDirection };


		HitRecord closestHit{};
		pScene->GetClosestHit(viewRay, closestHit);

		const auto& lights = pScene->GetLights();
		if (closestHit.didHit)
		{
			for (const Light& light : lights)
			{
				ColorRGB currentLightColor{};

				const Vector3 lightRayOrigin{ closestHit.origin + closestHit.normal * 0.0001f };
				const Vector3 lightRayDirection{ LightUtils::GetDirectionToLight(light, lightRayOrigin) };
				const Vector3 lightDirNormalized{ lightRayDirection.Normalized() };

				const Ray lightRay
				{
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
						currentLightColor += LightUtils::GetRadiance(light, closestHit.origin) * BRDF * ObserveredArea;
					break;
				case LightingMode::ObservedArea:
					if (ObserveredArea > 0)
						currentLightColor += ColorRGB(1, 1, 1) * ObserveredArea;
					break;
				case LightingMode::Radiance:
					currentLightColor += LightUtils::GetRadiance(light, closestHit.origin);
					break;
				case LightingMode::BRDF:
					currentLightColor += BRDF;
					break;
				}

				currentLightColor *= shadow;
				currentSampleColor += currentLightColor;
			}
		}

		finalColor += (currentSampleColor / sampleCount);
	}

	finalColor.MaxToOne();

	// std::cout << px << ", " << py << std::endl;
	// std::cout << "bufferPixelIndex: " << px + (py * m_Width) << std::endl;
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
	std::cout << static_cast<int>(m_CurrentLightingMode) << std::endl;
	m_CurrentLightingMode = LightingMode((static_cast<int>(m_CurrentLightingMode) + 1) % static_cast<int>(LightingMode::TOTAL_MODES));
}
