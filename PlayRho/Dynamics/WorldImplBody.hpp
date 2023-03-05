/*
 * Original work Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
 * Modified work Copyright (c) 2023 Louis Langholtz https://github.com/louis-langholtz/PlayRho
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef PLAYRHO_DYNAMICS_WORLDIMPLBODY_HPP
#define PLAYRHO_DYNAMICS_WORLDIMPLBODY_HPP

/// @file
/// Declarations of free functions of WorldImpl for bodies.

#include <PlayRho/Common/Math.hpp>
#include <PlayRho/Common/Units.hpp>
#include <PlayRho/Common/Transformation.hpp>
#include <PlayRho/Common/Range.hpp> // for SizedRange
#include <PlayRho/Common/Velocity.hpp>
#include <PlayRho/Common/Vector2.hpp> // for Length2, LinearAcceleration2

#include <PlayRho/Dynamics/BodyID.hpp>
#include <PlayRho/Dynamics/FixtureID.hpp>
#include <PlayRho/Dynamics/BodyType.hpp>
#include <PlayRho/Dynamics/BodyConf.hpp>
#include <PlayRho/Dynamics/Contacts/KeyedContactID.hpp>
#include <PlayRho/Dynamics/Joints/JointID.hpp>

#include <PlayRho/Collision/MassData.hpp>

#include <vector>

namespace playrho {
namespace d2 {

class WorldImpl;
class Body;

/// @brief Gets the extent of the currently valid body range.
/// @note This is one higher than the maxium BodyID that is in range for body related
///   functions.
/// @relatedalso WorldImpl
BodyCounter GetBodyRange(const WorldImpl& world) noexcept;

/// @brief Creates a body with the given configuration within the given world.
/// @relatedalso WorldImpl
BodyID CreateBody(WorldImpl& world, const BodyConf& def = GetDefaultBodyConf());

/// @brief Gets the body configuration for the identified body.
/// @throws std::out_of_range If given an invalid body identifier.
/// @relatedalso WorldImpl
const Body& GetBody(const WorldImpl& world, BodyID id);

/// @brief Sets the body state for the identified body.
/// @throws std::out_of_range If given an invalid body identifier.
/// @relatedalso WorldImpl
void SetBody(WorldImpl& world, BodyID id, const Body& value);

/// @brief Destroys the identified body.
/// @relatedalso WorldImpl
void Destroy(WorldImpl& world, BodyID id);

/// @brief Gets the range of all joints attached to this body.
/// @throws std::out_of_range If given an invalid body identifier.
/// @relatedalso WorldImpl
SizedRange<std::vector<std::pair<BodyID, JointID>>::const_iterator>
GetJoints(const WorldImpl& world, BodyID id);

/// @brief Gets the range of all constant fixtures attached to the given body.
/// @throws std::out_of_range If given an invalid body identifier.
/// @relatedalso WorldImpl
SizedRange<std::vector<FixtureID>::const_iterator>
GetFixtures(const WorldImpl& world, BodyID id);

/// @brief Gets the container of all contacts attached to this body.
/// @warning This collection changes during the time step and you may
///   miss some collisions if you don't use <code>ContactListener</code>.
/// @throws std::out_of_range If given an invalid body identifier.
/// @relatedalso WorldImpl
SizedRange<std::vector<KeyedContactPtr>::const_iterator>
GetContacts(const WorldImpl& world, BodyID id);

} // namespace d2
} // namespace playrho

#endif // PLAYRHO_DYNAMICS_WORLDIMPLBODY_HPP
