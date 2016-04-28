/*
* Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
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

#include <Box2D/Collision/Shapes/b2ChainShape.h>
#include <Box2D/Collision/Shapes/b2EdgeShape.h>
#include <new>
#include <string.h>

b2ChainShape::~b2ChainShape()
{
	Clear();
}

void b2ChainShape::Clear()
{
	b2Free(m_vertices);
	m_vertices = nullptr;
	m_count = 0;
}

void b2ChainShape::CreateLoop(const b2Vec2* vertices, child_count_t count)
{
	b2Assert(m_vertices == nullptr && m_count == 0);
	b2Assert(count >= 3);
	for (auto i = decltype(count){1}; i < count; ++i)
	{
		// If the code crashes here, it means your vertices are too close together.
		b2Assert(b2DistanceSquared(vertices[i-1], vertices[i]) > b2Square(b2_linearSlop));
	}

	m_count = count + 1;
	m_vertices = static_cast<b2Vec2*>(b2Alloc(m_count * sizeof(b2Vec2)));
	memcpy(m_vertices, vertices, count * sizeof(b2Vec2));
	m_vertices[count] = m_vertices[0];
	m_prevVertex = m_vertices[m_count - 2];
	m_nextVertex = m_vertices[1];
	m_hasPrevVertex = true;
	m_hasNextVertex = true;
}

void b2ChainShape::CreateChain(const b2Vec2* vertices, child_count_t count)
{
	b2Assert((m_vertices == nullptr) && (m_count == 0));
	b2Assert(count >= 2);
	for (auto i = decltype(count){1}; i < count; ++i)
	{
		// If the code crashes here, it means your vertices are too close together.
		b2Assert(b2DistanceSquared(vertices[i-1], vertices[i]) > b2Square(b2_linearSlop));
	}

	m_count = count;
	m_vertices = static_cast<b2Vec2*>(b2Alloc(count * sizeof(b2Vec2)));
	memcpy(m_vertices, vertices, m_count * sizeof(b2Vec2));

	m_hasPrevVertex = false;
	m_hasNextVertex = false;

	m_prevVertex.SetZero();
	m_nextVertex.SetZero();
}

void b2ChainShape::SetPrevVertex(const b2Vec2& prevVertex) noexcept
{
	m_prevVertex = prevVertex;
	m_hasPrevVertex = true;
}

void b2ChainShape::SetNextVertex(const b2Vec2& nextVertex) noexcept
{
	m_nextVertex = nextVertex;
	m_hasNextVertex = true;
}

b2Shape* b2ChainShape::Clone(b2BlockAllocator* allocator) const
{
	void* mem = allocator->Allocate(sizeof(b2ChainShape));
	auto clone = new (mem) b2ChainShape;
	clone->CreateChain(m_vertices, m_count);
	clone->m_prevVertex = m_prevVertex;
	clone->m_nextVertex = m_nextVertex;
	clone->m_hasPrevVertex = m_hasPrevVertex;
	clone->m_hasNextVertex = m_hasNextVertex;
	return clone;
}

child_count_t b2ChainShape::GetChildCount() const
{
	// edge count = vertex count - 1
	b2Assert(m_count > 0);
	return m_count - 1;
}

void b2ChainShape::GetChildEdge(b2EdgeShape* edge, child_count_t index) const
{
	b2Assert((0 <= index) && (index < (m_count - 1)));
	edge->SetRadius(GetRadius());

	edge->Set(m_vertices[index + 0], m_vertices[index + 1]);

	if (index > 0)
	{
		edge->SetVertex0(m_vertices[index - 1]);
	}
	else if (m_hasPrevVertex)
	{
		edge->SetVertex0(m_prevVertex);
	}

	if (index < m_count - 2)
	{
		edge->SetVertex3(m_vertices[index + 2]);
	}
	else if (m_hasNextVertex)
	{
		edge->SetVertex3(m_nextVertex);
	}
}

bool b2ChainShape::TestPoint(const b2Transform& xf, const b2Vec2& p) const
{
	B2_NOT_USED(xf);
	B2_NOT_USED(p);
	return false;
}

bool b2ChainShape::RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
							const b2Transform& xf, child_count_t childIndex) const
{
	b2Assert(childIndex < m_count);

	const auto i1 = childIndex;
	auto i2 = childIndex + 1;
	if (i2 == m_count)
	{
		i2 = 0;
	}

	const auto edgeShape = b2EdgeShape(m_vertices[i1], m_vertices[i2]);
	return edgeShape.RayCast(output, input, xf, 0);
}

b2AABB b2ChainShape::ComputeAABB(const b2Transform& xf, child_count_t childIndex) const
{
	b2Assert(childIndex < m_count);

	const auto i1 = childIndex;
	auto i2 = childIndex + 1;
	if (i2 == m_count)
	{
		i2 = 0;
	}

	const auto v1 = b2Mul(xf, m_vertices[i1]);
	const auto v2 = b2Mul(xf, m_vertices[i2]);

	return {b2Min(v1, v2), b2Max(v1, v2)};
}

b2MassData b2ChainShape::ComputeMass(b2Float density) const
{
	B2_NOT_USED(density);

	return {b2Float{0}, b2Vec2_zero, b2Float{0}};
}
