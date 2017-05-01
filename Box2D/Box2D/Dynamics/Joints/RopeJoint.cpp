/*
* Original work Copyright (c) 2007-2011 Erin Catto http://www.box2d.org
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

#include <Box2D/Dynamics/Joints/RopeJoint.hpp>
#include <Box2D/Dynamics/Body.hpp>
#include <Box2D/Dynamics/StepConf.hpp>
#include <Box2D/Dynamics/Contacts/ContactSolver.hpp>
#include <Box2D/Dynamics/Contacts/BodyConstraint.hpp>

using namespace box2d;

// Limit:
// C = norm(pB - pA) - L
// u = (pB - pA) / norm(pB - pA)
// Cdot = dot(u, vB + cross(wB, rB) - vA - cross(wA, rA))
// J = [-u -cross(rA, u) u cross(rB, u)]
// K = J * invM * JT
//   = invMassA + invIA * cross(rA, u)^2 + invMassB + invIB * cross(rB, u)^2

RopeJoint::RopeJoint(const RopeJointDef& def):
    Joint(def),
    m_localAnchorA(def.localAnchorA),
    m_localAnchorB(def.localAnchorB),
    m_maxLength(def.maxLength)
{
}

void RopeJoint::InitVelocityConstraints(BodyConstraints& bodies,
                                        const StepConf& step,
                                        const ConstraintSolverConf& conf)
{
    auto& bodiesA = bodies.at(GetBodyA());
    auto& bodiesB = bodies.at(GetBodyB());

    const auto invMassA = bodiesA.GetInvMass();
    const auto invRotInertiaA = bodiesA.GetInvRotInertia();
    const auto posA = bodiesA.GetPosition();
    auto velA = bodiesA.GetVelocity();

    const auto invMassB = bodiesB.GetInvMass();
    const auto invRotInertiaB = bodiesB.GetInvRotInertia();
    const auto posB = bodiesB.GetPosition();
    auto velB = bodiesB.GetVelocity();

    const auto qA = UnitVec2{posA.angular};
    const auto qB = UnitVec2{posB.angular};

    m_rA = Rotate(m_localAnchorA - bodiesA.GetLocalCenter(), qA);
    m_rB = Rotate(m_localAnchorB - bodiesB.GetLocalCenter(), qB);
    const auto posDelta = Length2D{(posB.linear + m_rB) - (posA.linear + m_rA)};
    
    const auto uv = GetUnitVector(posDelta, m_length);

    const auto C = m_length - m_maxLength;
    m_state = (C > Length{0})? e_atUpperLimit: e_inactiveLimit;

    if (m_length > conf.linearSlop)
    {
        m_u = uv;
    }
    else
    {
        m_u = UnitVec2::GetZero();
        m_mass = Mass{0};
        m_impulse = 0;
        return;
    }

    // Compute effective mass.
    const auto crA = Cross(m_rA, m_u);
    const auto crB = Cross(m_rB, m_u);
    const auto invRotMassA = InvMass{invRotInertiaA * Square(crA) / SquareRadian};
    const auto invRotMassB = InvMass{invRotInertiaB * Square(crB) / SquareRadian};
    const auto invMass = invMassA + invMassB + invRotMassA + invRotMassB;

    m_mass = (invMass != InvMass{0}) ? RealNum{1} / invMass : Mass{0};

    if (step.doWarmStart)
    {
        // Scale the impulse to support a variable time step.
        m_impulse *= step.dtRatio;

        const auto P = m_impulse * m_u;
        
        // L * M * L T^-1 / QP is: L^2 M T^-1 QP^-1 which is: AngularMomentum.
        const auto crossAP = AngularMomentum{Cross(m_rA, P) / Radian};
        const auto crossBP = AngularMomentum{Cross(m_rB, P) / Radian}; // L * M * L T^-1 is: L^2 M T^-1

        velA -= Velocity{bodiesA.GetInvMass() * P, invRotInertiaA * crossAP};
        velB += Velocity{bodiesB.GetInvMass() * P, invRotInertiaB * crossBP};
    }
    else
    {
        m_impulse = 0;
    }

    bodiesA.SetVelocity(velA);
    bodiesB.SetVelocity(velB);
}

RealNum RopeJoint::SolveVelocityConstraints(BodyConstraints& bodies, const StepConf& step)
{
    auto& bodiesA = bodies.at(GetBodyA());
    auto& bodiesB = bodies.at(GetBodyB());

    auto velA = bodiesA.GetVelocity();
    auto velB = bodiesB.GetVelocity();

    // Cdot = dot(u, v + cross(w, r))
    const auto vpA = velA.linear + GetRevPerpendicular(m_rA) * velA.angular / Radian;
    const auto vpB = velB.linear + GetRevPerpendicular(m_rB) * velB.angular / Radian;
    const auto C = m_length - m_maxLength;
    const auto vpDelta = LinearVelocity2D{vpB - vpA};

    // Predictive constraint.
    const auto Cdot = LinearVelocity{Dot(m_u, vpDelta)}
                    + ((C < Length{0})? LinearVelocity{step.GetInvTime() * C}: LinearVelocity{0});

    auto impulse = -m_mass * Cdot;
    const auto oldImpulse = m_impulse;
    m_impulse = Min(Momentum{0}, m_impulse + impulse);
    impulse = m_impulse - oldImpulse;

    const auto P = impulse * m_u;
    
    // L * M * L T^-1 / QP is: L^2 M T^-1 QP^-1 which is: AngularMomentum.
    const auto crossAP = AngularMomentum{Cross(m_rA, P) / Radian};
    const auto crossBP = AngularMomentum{Cross(m_rB, P) / Radian}; // L * M * L T^-1 is: L^2 M T^-1

    velA -= Velocity{bodiesA.GetInvMass() * P, bodiesA.GetInvRotInertia() * crossAP};
    velB += Velocity{bodiesB.GetInvMass() * P, bodiesB.GetInvRotInertia() * crossBP};

    bodiesA.SetVelocity(velA);
    bodiesB.SetVelocity(velB);
    
    return impulse / (Kilogram * MeterPerSecond);
}

bool RopeJoint::SolvePositionConstraints(BodyConstraints& bodies, const ConstraintSolverConf& conf) const
{
    auto& bodiesA = bodies.at(GetBodyA());
    auto& bodiesB = bodies.at(GetBodyB());

    auto posA = bodiesA.GetPosition();
    auto posB = bodiesB.GetPosition();

    const UnitVec2 qA(posA.angular), qB(posB.angular);

    const auto rA = Length2D{Rotate(m_localAnchorA - bodiesA.GetLocalCenter(), qA)};
    const auto rB = Length2D{Rotate(m_localAnchorB - bodiesB.GetLocalCenter(), qB)};
    const auto posDelta = (posB.linear + rB) - (posA.linear + rA);
    
    auto length = Length{0};
    const auto u = GetUnitVector(posDelta, length);
    
    const auto C = Clamp(length - m_maxLength, Length{0}, conf.maxLinearCorrection);

    const auto impulse = -m_mass * C;
    const auto linImpulse = impulse * u;
    
    const auto angImpulseA = Cross(rA, linImpulse) / Radian;
    const auto angImpulseB = Cross(rB, linImpulse) / Radian;

    posA -= Position{bodiesA.GetInvMass() * linImpulse, bodiesA.GetInvRotInertia() * angImpulseA};
    posB += Position{bodiesB.GetInvMass() * linImpulse, bodiesB.GetInvRotInertia() * angImpulseB};

    bodiesA.SetPosition(posA);
    bodiesB.SetPosition(posB);

    return (length - m_maxLength) < conf.linearSlop;
}

Length2D RopeJoint::GetAnchorA() const
{
    return GetWorldPoint(*GetBodyA(), GetLocalAnchorA());
}

Length2D RopeJoint::GetAnchorB() const
{
    return GetWorldPoint(*GetBodyB(), GetLocalAnchorB());
}

Force2D RopeJoint::GetReactionForce(Frequency inv_dt) const
{
    return (inv_dt * m_impulse) * m_u;
}

Torque RopeJoint::GetReactionTorque(Frequency inv_dt) const
{
    NOT_USED(inv_dt);
    return Torque{0};
}

Length RopeJoint::GetMaxLength() const
{
    return m_maxLength;
}

Joint::LimitState RopeJoint::GetLimitState() const
{
    return m_state;
}
