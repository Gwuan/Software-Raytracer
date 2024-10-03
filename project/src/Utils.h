#pragma once
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const Vector3 SphereRayVec{ ray.origin - sphere.origin };

			const float a = Vector3::Dot(ray.direction, ray.direction);
			const float b = Vector3::Dot(ray.direction, SphereRayVec) * 2;
			const float c = Vector3::Dot(SphereRayVec, SphereRayVec) - (sphere.radius * sphere.radius);

			const float discriminant = (b * b) - (4.f * a * c);

			const bool anyHits = discriminant > 0;

			if (anyHits)
			{
				const float squareD = sqrt(discriminant);
				float t{};
				t = ((b * -1) - squareD) / (2 * a);

				if (t < ray.min || t > ray.max)
				{
					t = ((b * -1) + squareD) / (2 * a);
					if (t < ray.min || t > ray.max)
						return false;
				}

				if(!ignoreHitRecord)
				{
					hitRecord.didHit = true;
					hitRecord.origin = ray.origin;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.t = t;
					hitRecord.normal = Vector3{ray.direction * -1}.Normalized();
				}

				return true;
			}
			else
			{
				return false;
			}
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const float t = Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction, plane.normal);

			bool didHit{ t > ray.min && t < ray.max };

			if (!ignoreHitRecord && didHit)
			{
				hitRecord.t = t;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin;
				hitRecord.normal = Vector3{ray.direction * -1}.Normalized();
			}

			return didHit;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			throw std::runtime_error("Not Implemented Yet");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			throw std::runtime_error("Not Implemented Yet");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W2
			// Both light types: Point & directional lights (direction lights dont have an origin, magnitude of this direction is equal to FLT_MAX)
			// Return a unnormalized vector going from origin to lights origin
			// Because the returned vector in unnormalized, you can perform the normalization call inside your shadowing logic and automatically capture the magnitude distance between hit and light

			if (light.type == LightType::Point)
			{
				return { light.origin - origin };
			}
			else
			{
				throw std::runtime_error("Not Implemented Yet");
				return {};
			}
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			throw std::runtime_error("Not Implemented Yet");
			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (std::isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}