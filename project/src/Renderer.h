#pragma once

#include <cstdint>

// Forwarding structs
struct SDL_Window;
struct SDL_Surface;




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

	private:
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
	};
}
