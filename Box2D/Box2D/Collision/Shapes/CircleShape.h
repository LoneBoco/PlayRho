/*
* Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
* Modified work Copyright (c) 2016 Louis Langholtz https://github.com/louis-langholtz/Box2D
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_CIRCLE_SHAPE_H
#define B2_CIRCLE_SHAPE_H

#include <Box2D/Collision/Shapes/Shape.h>

namespace box2d {

/// A circle shape.
class CircleShape : public Shape
{
public:
	constexpr explicit CircleShape(float_t radius = 0, Vec2 position = Vec2_zero) noexcept:
		Shape{e_circle, radius}, m_p(position) {}

	CircleShape(const CircleShape&) = default;

	CircleShape& operator=(const CircleShape& other) = default;

	/// @see Shape::GetChildCount
	child_count_t GetChildCount() const override;

	/// Implement Shape.
	bool TestPoint(const Transformation& transform, const Vec2& p) const override;

	/// Implement Shape.
	bool RayCast(RayCastOutput* output, const RayCastInput& input,
				const Transformation& transform, child_count_t childIndex) const override;

	/// @see Shape::ComputeAABB
	AABB ComputeAABB(const Transformation& transform, child_count_t childIndex) const override;

	/// @see Shape::ComputeMass
	MassData ComputeMass(float_t density) const override;

	/// Gets the position of the center of this circle shape.
	Vec2 GetPosition() const noexcept { return m_p; }

	void SetPosition(const Vec2& value) noexcept { m_p = value; }

private:
	/// Linear position of the shape as initialized on construction or as assigned using the SetPosition method.
	Vec2 m_p = Vec2_zero;
};

} // namespace box2d

#endif
