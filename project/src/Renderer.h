#pragma once

#include <cstdint>

// Forwarding structs
struct SDL_Window;
struct SDL_Surface;

struct Vector2
{
	float x;
	float y;
};


namespace dae
{
	struct Matrix;
	class Vector3;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		void RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Matrix cameraToWorld, const Vector3 cameraOrigin) const;
		bool SaveBufferToImage() const;

		void CycleLightingMode();
		void ToggleShadows() { m_ShadowsEnabled = !m_ShadowsEnabled; }

		// Anti Aliassing
		void IncreaseMSAA();
		void DecreaseMSAA();
		uint32_t GetSampleAmount() const { return m_SampleAmount; }


	private:

		void CalculateSamplePositions()
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

		enum class LightingMode
		{
			ObservedArea,  // Lambert Cosine Law
			Radiance,  // Incident Radiance
			BRDF,  // Scattering of the light
			Combined,  // ObservedArea * Radiance & BRDF
			TOTAL_MODES  // Used for cycling between different modes
		};

		LightingMode m_CurrentLightingMode{ LightingMode::Combined };
		bool m_ShadowsEnabled{ true };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};

		float m_AspectRatio;


		// Samples for anti-aliasing
		uint32_t m_SampleAmount = 1;
		std::vector<Vector2> m_SamplePositions;

		static const uint32_t m_MaxSampleAmount = 16; 
		static const uint32_t m_minSampleAmount = 1;
		


	};
}
