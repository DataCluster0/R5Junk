#include "includes.h"
#include "Vector3.h"

namespace Math
{
	Vector3::Vector3()
		: X(0), Y(0), Z(0)
	{
	}

	Vector3::Vector3(float X, float Y, float Z)
		: X(X), Y(Y), Z(Z)
	{
	}

	float& Vector3::operator[](int Index)
	{
		if (Index == 0)
			return X;
		else if (Index == 1)
			return Y;

		return Z;
	}

	Vector3 Vector3::operator+(const Vector3& Rhs) const
	{
		return Vector3(X + Rhs.X, Y + Rhs.Y, Z + Rhs.Z);
	}

	Vector3 Vector3::operator-(const Vector3& Rhs) const
	{
		return Vector3(X - Rhs.X, Y - Rhs.Y, Z - Rhs.Z);
	}

	Vector3 Vector3::operator*(const Vector3& Rhs) const
	{
		return Vector3(X * Rhs.X, Y * Rhs.Y, Z * Rhs.Z);
	}

	Vector3 Vector3::operator/(const Vector3& Rhs) const
	{
		return Vector3(X / Rhs.X, Y / Rhs.Y, Z / Rhs.Z);
	}

	Vector3& Vector3::operator+=(const Vector3& Rhs)
	{
		X += Rhs.X;
		Y += Rhs.Y;
		Z += Rhs.Z;

		return *this;
	}

	Vector3& Vector3::operator-=(const Vector3& Rhs)
	{
		X -= Rhs.X;
		Y -= Rhs.Y;
		Z -= Rhs.Z;

		return *this;
	}

	Vector3& Vector3::operator*=(const Vector3& Rhs)
	{
		X *= Rhs.X;
		Y *= Rhs.Y;
		Z *= Rhs.Z;

		return *this;
	}

	Vector3& Vector3::operator/=(const Vector3& Rhs)
	{
		X /= Rhs.X;
		Y /= Rhs.Y;
		Z /= Rhs.Z;

		return *this;
	}

	Vector3 Vector3::operator+(const float Rhs) const
	{
		return Vector3(X + Rhs, Y + Rhs, Z + Rhs);
	}

	Vector3 Vector3::operator-(const float Rhs) const
	{
		return Vector3(X - Rhs, Y - Rhs, Z - Rhs);
	}

	Vector3 Vector3::operator*(const float Rhs) const
	{
		return Vector3(X * Rhs, Y * Rhs, Z * Rhs);
	}

	Vector3 Vector3::operator/(const float Rhs) const
	{
		return Vector3(X / Rhs, Y / Rhs, Z / Rhs);
	}

	Vector3& Vector3::operator+=(const float Rhs)
	{
		X += Rhs;
		Y += Rhs;
		Z += Rhs;

		return *this;
	}

	Vector3& Vector3::operator-=(const float Rhs)
	{
		X -= Rhs;
		Y -= Rhs;
		Z -= Rhs;

		return *this;
	}

	Vector3& Vector3::operator*=(const float Rhs)
	{
		X *= Rhs;
		Y *= Rhs;
		Z *= Rhs;

		return *this;
	}

	Vector3& Vector3::operator/=(const float Rhs)
	{
		X /= Rhs;
		Y /= Rhs;
		Z /= Rhs;

		return *this;
	}

	bool Vector3::operator==(const Vector3& Rhs) const
	{
		return (std::abs(X - Rhs.X) < MathHelper::Epsilon) && (std::abs(Y - Rhs.Y) < MathHelper::Epsilon) && (std::abs(Z - Rhs.Z) < MathHelper::Epsilon);
	}

	bool Vector3::operator!=(const Vector3& Rhs) const
	{
		return !(*this == Rhs);
	}

	Vector3 Vector3::operator-() const
	{
		return Vector3(-X, -Y, -Z);
	}

	Vector3 Vector3::Min(const Vector3& a, const Vector3& b)
	{
		return {
		   Vector3::MinFloat(a.X, b.X),
		   Vector3::MinFloat(a.Y, b.Y),
		   Vector3::MinFloat(a.Z, b.Z)
		};
	}

	Vector3 Vector3::Max(const Vector3& a, const Vector3& b)
	{
		return {
		   Vector3::MaxFloat(a.X, b.X),
		   Vector3::MaxFloat(a.Y, b.Y),
		   Vector3::MaxFloat(a.Z, b.Z)
		};
	}

	float Vector3::MinFloat(const float& a, const float& b)
	{
		if (a > b)
			return a;
		else if (a < b)
			return b;

		return a;
	}

	float Vector3::MaxFloat(const float& a, const float& b)
	{
		if (a < b)
			return a;
		else if (a > b)
			return b;

		return a;
	}

	bool Vector3::IsBigger(const Vector3& input)
	{
		if (this->Length() > input.Length())
			return true;

		return false;
	}

	bool Vector3::IsSmaller(const Vector3& input)
	{
		if (this->Length() < input.Length())
			return true;

		return false;
	}

	void Vector3::Rotate(Vector3 axis, double theta)
	{
		double cosTheta = cos(theta);
		double sinTheta = sin(theta);
		*this = ((*this) * cosTheta) + (axis.Cross(*this) * sinTheta) + (axis * axis.Dot(*this)) * (1 - cosTheta);
	}

	float Vector3::Length() const
	{
		return (float)std::sqrt(X * X + Y * Y + Z * Z);
	}

	float Vector3::LengthSq() const
	{
		return X * X + Y * Y + Z * Z;
	}

	void Vector3::Normalize()
	{
		auto Length = this->Length();

		if (Length > 0.0f)
		{
			X /= Length;
			Y /= Length;
			Z /= Length;
		}
	}

	Vector3 Vector3::GetNormalized() const
	{
		auto Length = this->Length();

		return (Length > 0.0f) ? Vector3(X / Length, Y / Length, Z / Length) : Vector3(X, Y, Z);
	}

	Vector3 Vector3::Cross(const Vector3& Rhs) const
	{
		Vector3 Result;

		Result.X = (this->Y * Rhs.Z) - (this->Z * Rhs.Y);
		Result.Y = (this->Z * Rhs.X) - (this->X * Rhs.Z);
		Result.Z = (this->X * Rhs.Y) - (this->Y * Rhs.X);

		return Result;
	}

	float Vector3::Dot(const Vector3& Rhs)
	{
		return (this->X * Rhs.X) + (this->Y * Rhs.Y) + (this->Z * Rhs.Z);
	}

	Vector3 Vector3::Lerp(float Factor, const Vector3& Rhs) const
	{
		return (*this) + (Rhs - (*this)) * Factor;
	}

	Vector3 Vector3::Zero()
	{
		return Vector3();
	}

	Vector3 Vector3::One()
	{
		return Vector3(1, 1, 1);
	}
}