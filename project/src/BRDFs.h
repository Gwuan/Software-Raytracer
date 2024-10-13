#pragma once
#include "Maths.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			//todo: W3
			return (cd * kd) / PI;
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			//todo: W3
			return (cd * kd) / PI;
 		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//todo: W3
			const Vector3 reflect = l - (2 *Vector3::Dot(n, l)) * n ;
			return ColorRGB( 1.f,1.f,1.f) * (ks * pow(Vector3::Dot(reflect, v), exp));
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			//todo: W3
			const ColorRGB f1{ 1.f, 1.f, 1.f };
			const float dot{Vector3::Dot(h, v)};


			ColorRGB result = (f0 + ((f1 - f0) * pow( 1- dot,5)));
			return result;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			//todo: W3
			const float a{roughness * roughness};
			const float a2{ a * a};
			const float nDothH{ Vector3::Dot(n, h) };

			const float dom{ ((nDothH * nDothH) * (a2 - 1)) + 1 };

			const float result{ a2 / (PI * (dom * dom)) };
			return result;

			//const float numerator{ roughness * roughness };

			//const float nDotH{ std::max(Vector3::Dot(n, h), 0.0000001f) };

			//float denominator = PI * pow(pow(nDotH, 2.f) * (numerator - 1.f) + 1.f, 2.f);

			//denominator = std::max(denominator, 0.0000001f);

			//return numerator / denominator;

			//const float cosTheta{ Vector3::Dot(n, h) };

			//if (cosTheta <= 0.f) // Prevent division by 0
			//	return 0.f;

			//const float tanTheta{ sqrt(1.f / (cosTheta * cosTheta) - 1.f) };

			//const float alphaPower2{ roughness * roughness };

			//return alphaPower2 / (PI * pow(cosTheta * (alphaPower2 + tanTheta * tanTheta), 2));
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			//todo: W3
			const float a{ roughness * roughness};
			const float k = pow(a + 1, 2) / 8.f;

			const float nDotv{ Vector3::Dot(n,v) };

			const float result = nDotv / (nDotv * (1.f - k) + k);

			if (result < 0)
				bool isFucked = true;

			return result;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			return GeometryFunction_SchlickGGX(n, v, roughness) * GeometryFunction_SchlickGGX(n, l, roughness);
		}

	}
}