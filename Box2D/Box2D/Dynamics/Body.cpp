/*
* Original work Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
* Modified work Copyright (c) 2017 Louis Langholtz https://github.com/louis-langholtz/Box2D
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

#include <Box2D/Dynamics/Body.hpp>
#include <Box2D/Dynamics/Fixture.hpp>
#include <Box2D/Dynamics/World.hpp>
#include <Box2D/Dynamics/Contacts/Contact.hpp>
#include <Box2D/Dynamics/Joints/Joint.hpp>

#include <iterator>

using namespace box2d;

const FixtureDef& box2d::GetDefaultFixtureDef() noexcept
{
	static const auto def = FixtureDef{};
	return def;
}

uint16 Body::GetFlags(const BodyDef& bd) noexcept
{
	uint16 flags = 0;
	if (bd.bullet)
	{
		flags |= e_impenetrableFlag;
	}
	if (bd.fixedRotation)
	{
		flags |= e_fixedRotationFlag;
	}
	if (bd.allowSleep)
	{
		flags |= e_autoSleepFlag;
	}
	if (bd.awake)
	{
		flags |= e_awakeFlag;
	}
	if (bd.active)
	{
		flags |= e_activeFlag;
	}
	switch (bd.type)
	{
		case BodyType::Dynamic:   flags |= (e_velocityFlag|e_accelerationFlag); break;
		case BodyType::Kinematic: flags |= (e_impenetrableFlag|e_velocityFlag); break;
		case BodyType::Static:    flags |= (e_impenetrableFlag); break;
	}
	return flags;
}

Body::Body(const BodyDef& bd, World* world):
	m_flags{GetFlags(bd)},
	m_xf{bd.position, UnitVec2{bd.angle}},
	m_world{world},
	m_sweep{Position{bd.position, bd.angle}},
	m_velocity{Velocity{bd.linearVelocity, bd.angularVelocity}},
	m_invMass{(bd.type == BodyType::Dynamic)? RealNum{1}: RealNum{0}},
	m_linearDamping{bd.linearDamping},
	m_angularDamping{bd.angularDamping},
	m_sleepTime{bd.sleepTime},
	m_userData{bd.userData}
{
	assert(IsValid(bd.position));
	assert(IsValid(bd.linearVelocity));
	assert(IsValid(bd.angle));
	assert(IsValid(bd.angularVelocity));
	assert(IsValid(bd.angularDamping) && (bd.angularDamping >= RealNum{0}));
	assert(IsValid(bd.linearDamping) && (bd.linearDamping >= RealNum{0}));
}

Body::~Body()
{
	// Assumes destructor is private which it should be given factory handling of Body.
	InternalDestroyJoints();
	InternalDestroyContacts();
	InternalDestroyFixtures();
}

void Body::InternalDestroyContacts()
{
	// Destroy the attached contacts.
	while (!m_contacts.empty())
	{
		auto iter = m_contacts.begin();
		const auto contact = *iter;
		m_contacts.erase(iter);
		m_world->m_contactMgr.Destroy(contact);
	}
}

void Body::InternalDestroyJoints()
{
	// Delete the attached joints.
	while (!m_joints.empty())
	{
		auto iter = m_joints.begin();
		const auto joint = *iter;
		m_joints.erase(iter);
		if (m_world->m_destructionListener)
		{
			m_world->m_destructionListener->SayGoodbye(*joint);
		}
		
		m_world->Destroy(joint);
	}
}

void Body::DestroyFixtures()
{
	assert(!m_world->IsLocked());
	if (!m_world->IsLocked())
	{
		InternalDestroyFixtures();
	}
}

void Body::InternalDestroyFixtures()
{
	// Delete the attached fixtures. This destroys broad-phase proxies.
	while (!m_fixtures.empty())
	{
		const auto fixture = m_fixtures.front();
		m_fixtures.pop_front();
		
		if (m_world->m_destructionListener)
		{
			m_world->m_destructionListener->SayGoodbye(*fixture);
		}
		
		fixture->DestroyProxies(m_world->m_blockAllocator, m_world->m_contactMgr.m_broadPhase);
		Delete(fixture, m_world->m_blockAllocator);
	}
	
	ResetMassData();
}

void Body::SetType(BodyType type)
{
	assert(!m_world->IsLocked());
	if (m_world->IsLocked())
	{
		return;
	}

	if (GetType() == type)
	{
		return;
	}

	m_flags &= ~(e_impenetrableFlag|e_velocityFlag|e_accelerationFlag);
	switch (type)
	{
		case BodyType::Dynamic:   m_flags |= (e_velocityFlag|e_accelerationFlag); break;
		case BodyType::Kinematic: m_flags |= (e_impenetrableFlag|e_velocityFlag); break;
		case BodyType::Static:    m_flags |= (e_impenetrableFlag); break;
	}

	ResetMassData();

	if (type == BodyType::Static)
	{
		m_velocity = Velocity{Vec2_zero, 0_rad};
		m_sweep.pos0 = m_sweep.pos1;
		SynchronizeFixtures();
	}

	SetAwake();

	m_linearAcceleration = Vec2_zero;
	m_angularAcceleration = 0_rad;
	if (IsAccelerable())
	{
		m_linearAcceleration += m_world->GetGravity();
	}

	InternalDestroyContacts();

	auto& broadPhase = m_world->m_contactMgr.m_broadPhase;
	for (auto&& fixture: GetFixtures())
	{
		fixture->TouchProxies(broadPhase);
	}
}

static inline bool IsValid(std::shared_ptr<const Shape> shape, const World* world)
{
	if (!shape)
	{
		return false;
	}
	const auto vr = GetVertexRadius(*shape);
	if (!(vr >= world->GetMinVertexRadius()))
	{
		return false;
	}
	if (!(vr <= world->GetMaxVertexRadius()))
	{
		return false;
	}
	return true;
}

static inline bool IsValid(const FixtureDef& def)
{
	if (!(def.density >= 0))
	{
		return false;
	}
	if (!(def.friction >= 0))
	{
		return false;
	}
	if (!(def.restitution < std::numeric_limits<decltype(def.restitution)>::infinity()))
	{
		return false;
	}
	if (!(def.restitution > -std::numeric_limits<decltype(def.restitution)>::infinity()))
	{
		return false;
	}
	return true;
}

Fixture* Body::CreateFixture(std::shared_ptr<const Shape> shape, const FixtureDef& def, bool resetMassData)
{
	if (!::IsValid(shape, m_world) || !::IsValid(def))
	{
		return nullptr;
	}

	assert(!m_world->IsLocked());
	if (m_world->IsLocked())
	{
		return nullptr;
	}

	auto& allocator = m_world->m_blockAllocator;

	const auto memory = allocator.Allocate(sizeof(Fixture));
	const auto fixture = new (memory) Fixture{this, def, shape};
	
	if (IsActive())
	{
		fixture->CreateProxies(allocator, m_world->m_contactMgr.m_broadPhase, GetTransformation());
	}

	m_fixtures.push_front(fixture);

	// Adjust mass properties if needed.
	if (fixture->GetDensity() > 0)
	{
		SetMassDataDirty();
		if (resetMassData)
		{
			ResetMassData();
		}
	}

	// Let the world know we have a new fixture. This will cause new contacts
	// to be created at the beginning of the next time step.
	m_world->SetNewFixtures();

	return fixture;
}

void Body::DestroyFixture(Fixture* fixture, bool resetMassData)
{
	assert(!m_world->IsLocked());
	if (m_world->IsLocked())
	{
		return;
	}

	assert(fixture->m_body == this);

	// Remove the fixture from this body's singly linked list.
	auto found = false;
	{
		auto prev = m_fixtures.before_begin();
		for (auto iter = m_fixtures.begin(); iter != m_fixtures.end(); ++iter)
		{
			if (*iter == fixture)
			{
				m_fixtures.erase_after(prev);
				found = true;
				break;
			}
			prev = iter;
		}
	}

	// You tried to remove a shape that is not attached to this body.
	assert(found);

	// Destroy any contacts associated with the fixture.
	for (auto&& contact: m_contacts)
	{
		const auto fixtureA = contact->GetFixtureA();
		const auto fixtureB = contact->GetFixtureB();
		if ((fixture == fixtureA) || (fixture == fixtureB))
		{
			// This destroys the contact and removes it from
			// this body's contact list.
			m_world->m_contactMgr.Destroy(contact);
		}
	}

	fixture->DestroyProxies(m_world->m_blockAllocator, m_world->m_contactMgr.m_broadPhase);
	
	Delete(fixture, m_world->m_blockAllocator);
	
	SetMassDataDirty();		
	if (resetMassData)
	{
		ResetMassData();
	}
}

void Body::ResetMassData()
{
	// Compute mass data from shapes. Each shape has its own density.

	// Non-dynamic bodies (Static and kinematic ones) have zero mass.
	if (!IsAccelerable())
	{
		m_invMass = 0;
		m_invI = 0;
		m_sweep = Sweep{Position{GetLocation(), GetAngle()}};
		UnsetMassDataDirty();
		return;
	}

	const auto massData = ComputeMassData(*this);

	// Force all dynamic bodies to have a positive mass.
	const auto mass = (massData.mass > 0)? massData.mass: RealNum{1};
	m_invMass = 1 / mass;
	
	// Compute center of mass.
	const auto localCenter = massData.center * m_invMass;
	
	if ((massData.I > 0) && (!IsFixedRotation()))
	{
		// Center the inertia about the center of mass.
		const auto lengthSquared = GetLengthSquared(localCenter);
		//assert((massData.I - mass * lengthSquared) > 0);
		m_invI = 1 / (massData.I - mass * lengthSquared);
	}
	else
	{
		m_invI = 0;
	}

	// Move center of mass.
	const auto oldCenter = GetWorldCenter();
	m_sweep = Sweep{Position{Transform(localCenter, GetTransformation()), GetAngle()}, localCenter};

	// Update center of mass velocity.
	m_velocity.linear += GetRevPerpendicular(GetWorldCenter() - oldCenter) * m_velocity.angular.ToRadians();
	
	UnsetMassDataDirty();
}

void Body::SetMassData(const MassData& massData)
{
	assert(!m_world->IsLocked());
	if (m_world->IsLocked())
	{
		return;
	}

	if (!IsAccelerable())
	{
		return;
	}

	const auto mass = (massData.mass > RealNum(0))? massData.mass: RealNum{1};
	m_invMass = RealNum{1} / mass;

	if ((massData.I > RealNum{0}) && (!IsFixedRotation()))
	{
		const auto lengthSquared = GetLengthSquared(massData.center);
		const auto I = massData.I - mass * lengthSquared;
		assert(I > RealNum{0});
		m_invI = RealNum{1} / I;
	}
	else
	{
		m_invI = RealNum{0};
	}

	// Move center of mass.
	const auto oldCenter = GetWorldCenter();

	m_sweep = Sweep{Position{Transform(massData.center, GetTransformation()), GetAngle()}, massData.center};

	// Update center of mass velocity.
	m_velocity.linear += GetRevPerpendicular(GetWorldCenter() - oldCenter) * m_velocity.angular.ToRadians();
	
	UnsetMassDataDirty();
}

void Body::SetVelocity(const Velocity& velocity) noexcept
{
	if ((velocity.linear != Vec2_zero) || (velocity.angular != 0_rad))
	{
		if (!IsSpeedable())
		{
			return;
		}
		SetAwake();
	}
	m_velocity = velocity;
}

void Body::SetAcceleration(const Vec2 linear, const Angle angular) noexcept
{
	assert(IsValid(linear));
	assert(IsValid(angular));

	if ((linear != Vec2_zero) || (angular != 0_rad))
	{
		if (!IsAccelerable())
		{
			return;
		}
	}
	m_linearAcceleration = linear;
	m_angularAcceleration = angular;
}

bool Body::ShouldCollide(const Body* other) const
{
	// At least one body should be accelerable/dynamic.
	if (!IsAccelerable() && !other->IsAccelerable())
	{
		return false;
	}

	// Does a joint prevent collision?
	for (auto&& joint: m_joints)
	{
		if (joint->GetBodyA() == other || joint->GetBodyB() == other)
		{
			if (!(joint->m_collideConnected))
			{
				return false;
			}
		}
	}

	return true;
}

void Body::SynchronizeFixtures(const Transformation& t1, const Transformation& t2)
{
	auto& broadPhase = m_world->m_contactMgr.m_broadPhase;
	for (auto&& fixture: GetFixtures())
	{
		fixture->Synchronize(broadPhase, t1, t2);
	}
}

void Body::SetTransform(const Vec2 position, Angle angle)
{
	assert(IsValid(position));
	assert(IsValid(angle));

	assert(!m_world->IsLocked());
	if (m_world->IsLocked())
	{
		return;
	}

	const auto xf = Transformation{position, UnitVec2{angle}};
	m_xf = xf;
	m_sweep = Sweep{Position{Transform(GetLocalCenter(), xf), angle}, GetLocalCenter()};
	SynchronizeFixtures(xf, xf);
}

void Body::SynchronizeFixtures()
{
	SynchronizeFixtures(GetTransform0(m_sweep), GetTransformation());
}

void Body::SetActive(bool flag)
{
	assert(!m_world->IsLocked());

	if (flag == IsActive())
	{
		return;
	}

	if (flag)
	{
		m_flags |= e_activeFlag;

		// Create all proxies.
		auto& broadPhase = m_world->m_contactMgr.m_broadPhase;
		auto& allocator = m_world->m_blockAllocator;
		const auto xf = GetTransformation();
		for (auto&& fixture: GetFixtures())
		{
			fixture->CreateProxies(allocator, broadPhase, xf);
		}

		// Contacts are created the next time step.
	}
	else
	{
		m_flags &= ~e_activeFlag;

		// Destroy all proxies.
		auto& broadPhase = m_world->m_contactMgr.m_broadPhase;
		auto& allocator = m_world->m_blockAllocator;
		for (auto&& fixture: GetFixtures())
		{
			fixture->DestroyProxies(allocator, broadPhase);
		}

		InternalDestroyContacts();
	}
}

void Body::SetFixedRotation(bool flag)
{
	const auto status = IsFixedRotation();
	if (status == flag)
	{
		return;
	}

	if (flag)
	{
		m_flags |= e_fixedRotationFlag;
	}
	else
	{
		m_flags &= ~e_fixedRotationFlag;
	}

	m_velocity.angular = 0_rad;

	ResetMassData();
}

box2d::size_t box2d::GetWorldIndex(const Body* body)
{
	if (body)
	{
		const auto world = body->GetWorld();
		
		auto i = size_t{0};
		for (auto&& b: world->GetBodies())
		{
			if (b == body)
			{
				return i;
			}
			++i;
		}
	}
	return size_t(-1);
}

Velocity box2d::GetVelocity(const Body& body, RealNum h) noexcept
{
	assert(IsValid(h));

	// Integrate velocity and apply damping.
	auto velocity = body.GetVelocity();
	if (body.IsAccelerable())
	{
		// Integrate velocities.
		velocity.linear += h * body.GetLinearAcceleration();
		velocity.angular += h * body.GetAngularAcceleration();
		
		// Apply damping.
		// ODE: dv/dt + c * v = 0
		// Solution: v(t) = v0 * exp(-c * t)
		// Time step: v(t + dt) = v0 * exp(-c * (t + dt)) = v0 * exp(-c * t) * exp(-c * dt) = v * exp(-c * dt)
		// v2 = exp(-c * dt) * v1
		// Pade approximation:
		// v2 = v1 * 1 / (1 + c * dt)
		velocity.linear *= RealNum{1} / (RealNum{1} + h * body.GetLinearDamping());
		velocity.angular *= RealNum{1} / (RealNum{1} + h * body.GetAngularDamping());
	}
	return velocity;
}

size_t box2d::GetFixtureCount(const Body& body)
{
	const auto& fixtures = body.GetFixtures();
	return static_cast<size_t>(std::distance(std::begin(fixtures), std::end(fixtures)));
}

MassData box2d::ComputeMassData(const Body& body) noexcept
{
	auto mass = RealNum{0};
	auto I = RealNum{0};
	auto center = Vec2_zero;
	for (auto&& fixture: body.GetFixtures())
	{
		if (fixture->GetDensity() > 0)
		{
			const auto massData = GetMassData(*fixture);
			mass += massData.mass;
			center += massData.mass * massData.center;
			I += massData.I;
		}
	}
	return MassData{mass, center, I};
}

void box2d::RotateAboutWorldPoint(Body& body, Angle amount, Vec2 worldPoint)
{
	const auto xfm = body.GetTransformation();
	const auto p = xfm.p - worldPoint;
	const auto c = Cos(amount);
	const auto s = Sin(amount);
	const auto x = p.x * c - p.y * s;
	const auto y = p.x * s + p.y * c;
	const auto pos = Vec2{x, y} + worldPoint;
	const auto angle = GetAngle(xfm.q) + amount;
	body.SetTransform(pos, angle);
}

void box2d::RotateAboutLocalPoint(Body& body, Angle amount, Vec2 localPoint)
{
	RotateAboutWorldPoint(body, amount, GetWorldPoint(body, localPoint));
}
