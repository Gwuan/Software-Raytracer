#include <cassert>
#include <stdexcept>
#include <cmath>

#include "Matrix.h"

#include "MathHelpers.h"

namespace dae {
	Matrix::Matrix(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, const Vector3& t) :
		Matrix({ xAxis, 0 }, { yAxis, 0 }, { zAxis, 0 }, { t, 1 })
	{
	}

	Matrix::Matrix(const Vector4& xAxis, const Vector4& yAxis, const Vector4& zAxis, const Vector4& t)
	{
		data[0] = xAxis;
		data[1] = yAxis;
		data[2] = zAxis;
		data[3] = t;
	}

	Matrix::Matrix(const Matrix& m)
	{
		data[0] = m[0];
		data[1] = m[1];
		data[2] = m[2];
		data[3] = m[3];
	}

	Vector3 Matrix::TransformVector(const Vector3& v) const
	{
		return TransformVector(v[0], v[1], v[2]);
	}

	Vector3 Matrix::TransformVector(float x, float y, float z) const
	{
		return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z,
			data[0].y * x + data[1].y * y + data[2].y * z,
			data[0].z * x + data[1].z * y + data[2].z * z
		};
	}

	Vector3 Matrix::TransformPoint(const Vector3& p) const
	{
		return TransformPoint(p[0], p[1], p[2]);
	}

	Vector3 Matrix::TransformPoint(float x, float y, float z) const
	{
		return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
			data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
			data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
		};
	}

	const Matrix& Matrix::Transpose()
	{
		Matrix result{};
		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = data[c][r];
			}
		}

		data[0] = result[0];
		data[1] = result[1];
		data[2] = result[2];
		data[3] = result[3];

		return *this;
	}

	Matrix Matrix::Transpose(const Matrix& m)
	{
		Matrix out{ m };
		out.Transpose();

		return out;
	}

	Vector3 Matrix::GetAxisX() const
	{
		return data[0];
	}

	Vector3 Matrix::GetAxisY() const
	{
		return data[1];
	}

	Vector3 Matrix::GetAxisZ() const
	{
		return data[2];
	}

	Vector3 Matrix::GetTranslation() const
	{
		return data[3];
	}

	Matrix Matrix::CreateTranslation(float x, float y, float z)
	{
		Matrix result;

		result.data[0] = { 1, 0, 0, 0 };
		result.data[1] = { 0, 1, 0, 0 };
		result.data[2] = { 0, 0, 1, 0 };
		result.data[3] = { x, y, z, 1 };

		return result;
	}

	Matrix Matrix::CreateTranslation(const Vector3& t)
	{
		return { Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, t };
	}

	Matrix Matrix::CreateRotationX(float pitch)
	{
		Matrix result;
		result.data[0] = { 1, 0, 0, 0 };
		result.data[1] = { 0, cos(pitch), -sin(pitch), 0 };
		result.data[2] = { 0, sin(pitch), cos(pitch), 0 };
		result.data[3] = { 0, 0, 0, 1 };
		
		return result;
	}

	Matrix Matrix::CreateRotationY(float yaw)
	{
		Matrix result;

		result.data[0] = { cos(yaw), 0, -sin(yaw), 0 };
		result.data[1] = { 0, 1, 0, 0 };
		result.data[2] = { sin(yaw), 0, cos(yaw), 0 };
		result.data[3] = { 0, 0, 0, 1 };

		return result;
	}

	Matrix Matrix::CreateRotationZ(float roll)
	{
		Matrix result;

		result.data[0] = { cos(roll), sin(roll), 0, 0 };
		result.data[1] = { -sin(roll), cos(roll), 0, 0 };
		result.data[2] = { 0, 0, 1, 0 };
		result.data[3] = { 0, 0, 0, 1 };

		return result;
	}

	Matrix Matrix::CreateRotation(const Vector3& r)
	{
		return CreateRotationX(r[0]) * CreateRotationY(r[1]) * CreateRotationZ(r[2]);
	}

	Matrix Matrix::CreateRotation(float pitch, float yaw, float roll)
	{
		return CreateRotation({ pitch, yaw, roll });
	}

	Matrix Matrix::CreateScale(float sx, float sy, float sz)
	{
		Matrix result;

		result.data[0] = { sx, 0, 0, 0 };
		result.data[1] = { 0, sy, 0, 0 };
		result.data[2] = { 0, 0, sz, 0 };
		result.data[3] = { 0, 0, 0, 1 };

		return result;
	}

	Matrix Matrix::CreateScale(const Vector3& s)
	{
		return CreateScale(s[0], s[1], s[2]);
	}

#pragma region Operator Overloads
	Vector4& Matrix::operator[](int index)
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Vector4 Matrix::operator[](int index) const
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Matrix Matrix::operator*(const Matrix& m) const
	{
		Matrix result{};
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = Vector4::Dot(data[r], m_transposed[c]);
			}
		}

		return result;
	}

	const Matrix& Matrix::operator*=(const Matrix& m)
	{
		Matrix copy{ *this };
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				data[r][c] = Vector4::Dot(copy[r], m_transposed[c]);
			}
		}

		return *this;
	}

	bool Matrix::operator==(const Matrix& m) const
	{
		return data[0] == m.data[0]
			&& data[1] == m.data[1]
			&& data[2] == m.data[2]
			&& data[3] == m.data[3];
	}
#pragma endregion
}
