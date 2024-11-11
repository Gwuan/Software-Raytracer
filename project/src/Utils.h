#pragma once
#include <complex.h>
#include <fstream>
#include "Maths.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		}
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			const Vector3 SphereRayVec{ sphere.origin - ray.origin };

			const float a = ray.direction.SqrMagnitude();
			const float b = Vector3::Dot(ray.direction, SphereRayVec);
			const float c = SphereRayVec.SqrMagnitude() - (sphere.radius * sphere.radius);

			const float discriminant = Square(b) - (a * c);

			if (discriminant <= 0)
				return false;

			const float squareD = sqrt(discriminant);
			float t = (b - squareD) / a;

			if (t < ray.min || t > ray.max)
			{
				t = (b + squareD) / a;
				if (t < ray.min || t > ray.max)
					return false;
			}

			if(!ignoreHitRecord)
			{
				const Vector3 hitLocation{ ray.origin + (ray.direction * t) };
				const Vector3 hitToCenter{ hitLocation - sphere.origin };

				hitRecord.didHit = true;
				hitRecord.origin = hitLocation;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.t = t;
				hitRecord.normal = hitToCenter.Magnitude() > 0 ? hitToCenter.Normalized() : Vector3(0,0,0);
			}

			return true;
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

			if (t < ray.min || t > ray.max)
				return false;

			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = plane.normal;
			}

			return true;
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
			const float normDotDirect = Vector3::Dot(triangle.normal, ray.direction);

			// Inverse if ignoreHitRecord is true,
			// ignoreHitRecord is mostly used for shadows
			if(ignoreHitRecord)
			{

				switch (triangle.cullMode)
				{
				case TriangleCullMode::BackFaceCulling:
					if (normDotDirect < 0.f)
						return false;
					break;
				case TriangleCullMode::FrontFaceCulling:
					if (normDotDirect > 0.f)
						return false;
					break;
				}
			}
			else
			{
				switch (triangle.cullMode)
				{
				case TriangleCullMode::BackFaceCulling:
					if (normDotDirect > 0.f)
						return false;
					break;
				case TriangleCullMode::FrontFaceCulling:
					if (normDotDirect < 0.f)
						return false;
					break;
				}
			}

			if(AreEqual(normDotDirect, 0.f))
				return false;

			const Vector3 L = triangle.v0 - ray.origin;

			float t = Vector3::Dot(L, triangle.normal) / 
				Vector3::Dot(ray.direction, triangle.normal);

			if (t < ray.min || t > ray.max)
				return false;

			const Vector3 point = ray.origin + ray.direction * t;

			const Vector3 vertices[]{triangle.v0, triangle.v1, triangle.v2};
			for (size_t i{0}; i < std::size(vertices); i++)
			{
				const Vector3 e = vertices[(i + 1) % std::size(vertices)] - vertices[i];
				const Vector3 pVector = point -  vertices[i];
				if (Vector3::Dot(Vector3::Cross(e, pVector), triangle.normal) < 0)
					return false;
			}

			if(!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.didHit = true;
				hitRecord.origin = point;
				hitRecord.normal = triangle.normal;
			}

			return true;
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
			for (size_t i{0}; i < mesh.indices.size(); i += 3)
			// Slabtest first

			{
				Triangle temp{
					mesh.transformedPositions[mesh.indices[i]],
					mesh.transformedPositions[mesh.indices[i + 1]],
					mesh.transformedPositions[mesh.indices[i + 2]],
					mesh.transformedNormals[i]
				};

				temp.cullMode = mesh.cullMode;
				temp.materialIndex = mesh.materialIndex;

				if (HitTest_Triangle(temp, ray, hitRecord))
					return true;
			}

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
			// Both light types: Point & directional lights
			// (direction lights dont have an origin, magnitude of this direction is equal to FLT_MAX)

			// Return a unnormalized vector going from origin to lights origin
			// Because the returned vector in unnormalized, you can perform the normalization call
			// inside the shadowing logic and automatically capture the magnitude distance between hit and light.

			if (light.type == LightType::Point)
			{
				return { light.origin - origin };
			}
			else
			{
				return light.direction.Normalized() * FLT_MAX;
			}
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			const Vector3 direction{ light.origin - target };
			const float distance{ direction.SqrMagnitude() };

			return light.color * (light.intensity / distance);
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