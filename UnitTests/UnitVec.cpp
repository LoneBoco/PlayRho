/*
 * Copyright (c) 2023 Louis Langholtz https://github.com/louis-langholtz/PlayRho
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

#include "UnitTests.hpp"

#include <playrho/d2/UnitVec.hpp>
#include <playrho/Math.hpp>

#include <iostream>
#include <utility>

using namespace playrho;
using namespace playrho::d2;

TEST(UnitVec, ByteSize)
{
    // Check size at test runtime instead of compile-time via static_assert to avoid stopping
    // builds and to report actual size rather than just reporting that expected size is wrong.
    switch (sizeof(Real))
    {
        case  4: EXPECT_EQ(sizeof(UnitVec), std::size_t(8)); break;
        case  8: EXPECT_EQ(sizeof(UnitVec), std::size_t(16)); break;
        case 16: EXPECT_EQ(sizeof(UnitVec), std::size_t(32)); break;
        default: FAIL(); break;
    }
}

TEST(UnitVec, DefaultConstruction)
{
    constexpr auto ExpectedDimensions = 2u;
    EXPECT_EQ(UnitVec::size(), ExpectedDimensions);
    EXPECT_EQ(UnitVec().GetX(), UnitVec::value_type());
    EXPECT_EQ(UnitVec().GetY(), UnitVec::value_type());
    const auto uv = UnitVec();
    for (const auto& e: uv) {
        EXPECT_EQ(e, UnitVec::value_type());
    }
    EXPECT_EQ(UnitVec(), UnitVec::GetZero());
}

TEST(UnitVec, RightIsRevPerpOfBottom)
{
    EXPECT_EQ(UnitVec::GetRight(), UnitVec::GetDown().GetRevPerpendicular());
}

TEST(UnitVec, TopIsRevPerpOfRight)
{
    EXPECT_EQ(UnitVec::GetUp(), UnitVec::GetRight().GetRevPerpendicular());
}

TEST(UnitVec, LeftIsRevPerpOfTop)
{
    EXPECT_EQ(UnitVec::GetLeft(), UnitVec::GetUp().GetRevPerpendicular());
}

TEST(UnitVec, BottomIsRevPerpOfLeft)
{
    EXPECT_EQ(UnitVec::GetDown(), UnitVec::GetLeft().GetRevPerpendicular());
}


TEST(UnitVec, RightIsFwdPerpOfTop)
{
    EXPECT_EQ(UnitVec::GetRight(), UnitVec::GetUp().GetFwdPerpendicular());
}

TEST(UnitVec, TopIsFwdPerpOfLeft)
{
    EXPECT_EQ(UnitVec::GetUp(), UnitVec::GetLeft().GetFwdPerpendicular());
}

TEST(UnitVec, LeftIsFwdPerpOfBottom)
{
    EXPECT_EQ(UnitVec::GetLeft(), UnitVec::GetDown().GetFwdPerpendicular());
}

TEST(UnitVec, BottomIsFwdPerpOfRight)
{
    EXPECT_EQ(UnitVec::GetDown(), UnitVec::GetRight().GetFwdPerpendicular());
}

TEST(UnitVec, ByAngleInDegreesNearOriented)
{
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::GetRight())),
                static_cast<double>(GetX(UnitVec::GetRight())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::GetRight())),
                static_cast<double>(GetY(UnitVec::GetRight())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::GetUp())),
                static_cast<double>(GetX(UnitVec::GetUp())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::GetUp())),
                static_cast<double>(GetY(UnitVec::GetUp())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::GetLeft())),
                static_cast<double>(GetX(UnitVec::GetLeft())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::GetLeft())),
                static_cast<double>(GetY(UnitVec::GetLeft())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::GetDown())),
                static_cast<double>(GetX(UnitVec::GetDown())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::GetDown())),
                static_cast<double>(GetY(UnitVec::GetDown())), 0.0001);
}

TEST(UnitVec, ByAngleInRadiansNearOriented)
{
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::Get((Pi * Real(0) / Real(2)) * 1_rad))),
                static_cast<double>(GetX(UnitVec::GetRight())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::Get((Pi * Real(0) / Real(2)) * 1_rad))),
                static_cast<double>(GetY(UnitVec::GetRight())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::Get((Pi * Real(1) / Real(2)) * 1_rad))),
                static_cast<double>(GetX(UnitVec::GetUp())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::Get((Pi * Real(1) / Real(2)) * 1_rad))),
                static_cast<double>(GetY(UnitVec::GetUp())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::Get((Pi * Real(2) / Real(2)) * 1_rad))),
                static_cast<double>(GetX(UnitVec::GetLeft())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::Get((Pi * Real(2) / Real(2)) * 1_rad))),
                static_cast<double>(GetY(UnitVec::GetLeft())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetX(UnitVec::Get((Pi * Real(3) / Real(2)) * 1_rad))),
                static_cast<double>(GetX(UnitVec::GetDown())), 0.0001);
    EXPECT_NEAR(static_cast<double>(GetY(UnitVec::Get((Pi * Real(3) / Real(2)) * 1_rad))),
                static_cast<double>(GetY(UnitVec::GetDown())), 0.0001);
}

TEST(UnitVec, GetForInvalid)
{
    {
        const auto x = std::numeric_limits<Real>::signaling_NaN();
        const auto y = std::numeric_limits<Real>::quiet_NaN();
        EXPECT_FALSE(IsValid(UnitVec::Get(x, y).first));
    }
    {
        const auto x = std::numeric_limits<Real>::quiet_NaN();
        const auto y = Real(0);
        EXPECT_FALSE(IsValid(UnitVec::Get(x, y).first));
    }
    {
        const auto x = Real(0);
        const auto y = std::numeric_limits<Real>::quiet_NaN();
        EXPECT_FALSE(IsValid(UnitVec::Get(x, y).first));
    }
    {
        const auto x = Real(0);
        const auto y = Real(0);
        EXPECT_FALSE(IsValid(UnitVec::Get(x, y, UnitVec::GetDefaultFallback()).first));
        EXPECT_EQ(UnitVec::Get(x, y, UnitVec::GetDefaultFallback()).second, Real(0));
    }
    {
        const auto x = Real(0);
        const auto y = Real(0);
        EXPECT_EQ(UnitVec::Get(x, y, UnitVec::GetZero()).first, UnitVec::GetZero());
        EXPECT_EQ(UnitVec::Get(x, y, UnitVec::GetZero()).second, Real(0));
        EXPECT_EQ(GetX(UnitVec::Get(x, y, UnitVec::GetZero()).first), Real(0));
        EXPECT_EQ(GetY(UnitVec::Get(x, y, UnitVec::GetZero()).first), Real(0));
    }
}

TEST(UnitVec, Assumptions)
{
    const auto maxReal = std::numeric_limits<Real>::max();
    const auto maxRealSquared = maxReal * maxReal;
    EXPECT_FALSE(isnormal(maxRealSquared));
    const auto hypotMaxReal = hypot(maxReal, Real{0});
    EXPECT_TRUE(isnormal(hypotMaxReal));
    EXPECT_EQ(maxReal, hypotMaxReal);
    EXPECT_EQ(maxReal / hypotMaxReal, Real{1});
}

TEST(UnitVec, Get)
{
    EXPECT_EQ(UnitVec::Get(Real(+1), Real(0)).first, UnitVec::GetRight());
    EXPECT_EQ(UnitVec::Get(Real(-1), Real(0)).first, UnitVec::GetLeft());
    EXPECT_EQ(UnitVec::Get(Real(0), Real(+1)).first, UnitVec::GetUp());
    EXPECT_EQ(UnitVec::Get(Real(0), Real(-1)).first, UnitVec::GetDown());
    EXPECT_EQ(UnitVec::Get(+std::numeric_limits<Real>::max(), Real(0)).first, UnitVec::GetRight());
    EXPECT_EQ(UnitVec::Get(-std::numeric_limits<Real>::max(), Real(0)).first, UnitVec::GetLeft());
    EXPECT_EQ(UnitVec::Get(Real(0), +std::numeric_limits<Real>::max()).first, UnitVec::GetUp());
    EXPECT_EQ(UnitVec::Get(Real(0), -std::numeric_limits<Real>::max()).first, UnitVec::GetDown());
    EXPECT_EQ(UnitVec::Get(+std::numeric_limits<Real>::min(), Real(0)).first, UnitVec::GetRight());
    EXPECT_EQ(UnitVec::Get(-std::numeric_limits<Real>::min(), Real(0)).first, UnitVec::GetLeft());
    EXPECT_EQ(UnitVec::Get(Real(0), +std::numeric_limits<Real>::min()).first, UnitVec::GetUp());
    EXPECT_EQ(UnitVec::Get(Real(0), -std::numeric_limits<Real>::min()).first, UnitVec::GetDown());
    
    {
        const auto foo = std::get<0>(UnitVec::Get(Real(1), Real(1)));
        const auto boo = UnitVec::GetUpRight();
        EXPECT_NEAR(static_cast<double>(GetX(foo)), 0.70710676908493042, 0.000001);
        EXPECT_NEAR(static_cast<double>(GetY(foo)), 0.70710676908493042, 0.000001);
        EXPECT_NEAR(static_cast<double>(GetX(foo)), static_cast<double>(GetX(boo)), 0.000001);
        EXPECT_NEAR(static_cast<double>(GetY(foo)), static_cast<double>(GetY(boo)), 0.000001);
    }
    {
        const auto value = std::numeric_limits<float>::min();
        const auto valueSquared = Square(value);
        ASSERT_EQ(valueSquared, 0.0f);
        ASSERT_FALSE(isnormal(valueSquared));
        const auto magnitude = hypot(value, value);
        ASSERT_NE(magnitude, 0.0f);
        ASSERT_TRUE(isnormal(magnitude));
        const auto foo = UnitVec::Get(value, value).first;
        const auto boo = UnitVec::GetUpRight();
        EXPECT_NEAR(static_cast<double>(GetX(foo)), static_cast<double>(GetX(boo)), 0.000001);
        EXPECT_NEAR(static_cast<double>(GetY(foo)), static_cast<double>(GetY(boo)), 0.000001);
    }
    {
        const auto value = std::numeric_limits<float>::quiet_NaN();
        const auto valueSquared = Square(value);
        ASSERT_NE(valueSquared, 0.0f);
        const auto magnitude = hypot(value, value);
        ASSERT_NE(magnitude, 0.0f);
        ASSERT_FALSE(isnormal(magnitude));
        const auto foo = UnitVec::Get(value, value).first;
        EXPECT_EQ(foo, UnitVec());
    }
}

TEST(UnitVec, Absolute)
{
    EXPECT_EQ(UnitVec::GetZero().Absolute(), UnitVec::GetZero());
    EXPECT_EQ(UnitVec::GetDown().Absolute(), UnitVec::GetUp());
    EXPECT_EQ(UnitVec::GetUp().Absolute(), UnitVec::GetUp());
    EXPECT_EQ(UnitVec::GetLeft().Absolute(), UnitVec::GetRight());
    EXPECT_EQ(UnitVec::GetRight().Absolute(), UnitVec::GetRight());

    EXPECT_EQ(UnitVec::Get(Real(-1), Real(-1)).first.Absolute(),
              UnitVec::Get(Real(+1), Real(+1)).first);
}

TEST(UnitVec, RotateMethod)
{
    EXPECT_EQ(UnitVec::GetRight().Rotate(UnitVec::GetRight()), UnitVec::GetRight());
    EXPECT_EQ(UnitVec::GetUp().Rotate(UnitVec::GetRight()), UnitVec::GetUp());
    EXPECT_EQ(UnitVec::GetLeft().Rotate(UnitVec::GetRight()), UnitVec::GetLeft());
    EXPECT_EQ(UnitVec::GetDown().Rotate(UnitVec::GetRight()), UnitVec::GetDown());

    EXPECT_EQ(UnitVec::GetRight().Rotate(UnitVec::GetUp()), UnitVec::GetUp());
    EXPECT_EQ(UnitVec::GetUp().Rotate(UnitVec::GetUp()), UnitVec::GetLeft());
    EXPECT_EQ(UnitVec::GetLeft().Rotate(UnitVec::GetUp()), UnitVec::GetDown());
    EXPECT_EQ(UnitVec::GetDown().Rotate(UnitVec::GetUp()), UnitVec::GetRight());
    
    EXPECT_EQ(UnitVec::GetRight().Rotate(UnitVec::GetLeft()), UnitVec::GetLeft());
    EXPECT_EQ(UnitVec::GetUp().Rotate(UnitVec::GetLeft()), UnitVec::GetDown());
    EXPECT_EQ(UnitVec::GetLeft().Rotate(UnitVec::GetLeft()), UnitVec::GetRight());
    EXPECT_EQ(UnitVec::GetDown().Rotate(UnitVec::GetLeft()), UnitVec::GetUp());
}

TEST(UnitVec, RotateFunction)
{
    EXPECT_EQ(Rotate(UnitVec::GetRight(), UnitVec::GetRight()), UnitVec::GetRight());
    EXPECT_EQ(Rotate(UnitVec::GetUp(), UnitVec::GetRight()), UnitVec::GetUp());
    EXPECT_EQ(Rotate(UnitVec::GetLeft(), UnitVec::GetRight()), UnitVec::GetLeft());
    EXPECT_EQ(Rotate(UnitVec::GetDown(), UnitVec::GetRight()), UnitVec::GetDown());
    
    EXPECT_EQ(Rotate(UnitVec::GetRight(), UnitVec::GetUp()), UnitVec::GetUp());
    EXPECT_EQ(Rotate(UnitVec::GetUp(), UnitVec::GetUp()), UnitVec::GetLeft());
    EXPECT_EQ(Rotate(UnitVec::GetLeft(), UnitVec::GetUp()), UnitVec::GetDown());
    EXPECT_EQ(Rotate(UnitVec::GetDown(), UnitVec::GetUp()), UnitVec::GetRight());
    
    EXPECT_EQ(Rotate(UnitVec::GetRight(), UnitVec::GetLeft()), UnitVec::GetLeft());
    EXPECT_EQ(Rotate(UnitVec::GetUp(), UnitVec::GetLeft()), UnitVec::GetDown());
    EXPECT_EQ(Rotate(UnitVec::GetLeft(), UnitVec::GetLeft()), UnitVec::GetRight());
    EXPECT_EQ(Rotate(UnitVec::GetDown(), UnitVec::GetLeft()), UnitVec::GetUp());
}

TEST(UnitVec, Copy)
{
    const auto a = UnitVec{};
    auto b = a;
    auto c = UnitVec{};
    c = a;
    EXPECT_EQ(a, b);
}

TEST(UnitVec, StreamOut)
{
    {
        std::ostringstream os;
        os << UnitVec::GetLeft();
        EXPECT_STREQ(os.str().c_str(), "UnitVec(-1,0)");
    }
    {
        std::ostringstream os;
        os << UnitVec::GetUp();
        EXPECT_STREQ(os.str().c_str(), "UnitVec(0,1)");
    }
    {
        std::ostringstream os;
        os << UnitVec::GetRight();
        EXPECT_STREQ(os.str().c_str(), "UnitVec(1,0)");
    }
    {
        std::ostringstream os;
        os << UnitVec::GetDown();
        EXPECT_STREQ(os.str().c_str(), "UnitVec(0,-1)");
    }
}

TEST(UnitVec, BeginEnd)
{
    const auto uv = UnitVec::GetLeft();
    EXPECT_EQ(uv.begin(), uv.data());
    EXPECT_NE(uv.begin(), uv.end());
    EXPECT_LT(uv.begin(), uv.end());
    EXPECT_EQ(uv.begin(), uv.cbegin());
    EXPECT_EQ(uv.end(), uv.cend());
    EXPECT_EQ(uv.begin() + 2, uv.end());
}

TEST(UnitVec, MagSquaredSinCosWithinTwoUlps)
{
    auto ulps = 0;
    for (auto counter = 0; counter < 360000; ++counter) {
        SCOPED_TRACE(counter);
        const auto angle = (counter * Degree) / Real(1000);
        const auto x = cos(angle);
        const auto y = sin(angle);
        const auto m2 = x * x + y * y;
        while (!AlmostEqual(Real(m2), Real(1), ulps)) {
            ++ulps;
        }
    }
    EXPECT_EQ(ulps, 2);
}

TEST(UnitVec, ConstructorWithVec2)
{
    EXPECT_THROW(UnitVec(Vec2(4, 2)), InvalidArgument);
    EXPECT_NO_THROW(UnitVec(Vec2(0, 1)));
    {
        const auto value = Vec2(1, 0);
        ASSERT_NO_THROW((UnitVec{value}));
        EXPECT_EQ(static_cast<Vec2>(UnitVec(value)), value);
    }
}

TEST(UnitVec, CosSinConstructedReversibleWithinZeroUlps)
{
    auto ulps = 0;
    for (auto counter = 0; counter < 360000; ++counter) {
        SCOPED_TRACE(counter);
        const auto angle = (counter * Degree) / Real(1000);
        const auto x = cos(angle);
        const auto y = sin(angle);
        const auto pc = UnitVec(Vec2{x, y});
        while (!AlmostEqual(Real(pc.GetX()), Real(x), ulps)) {
            ++ulps;
        }
        while (!AlmostEqual(Real(pc.GetY()), Real(y), ulps)) {
            ++ulps;
        }
    }
    EXPECT_EQ(ulps, 0);
}

TEST(UnitVec, GetCosSinIsReversibleWithTwoUlps)
{
    auto ulps = 0;
    for (auto counter = 0; counter < 360000; ++counter) {
        SCOPED_TRACE(counter);
        const auto angle = (counter * Degree) / Real(1000);
        const auto x = cos(angle);
        const auto y = sin(angle);
        const auto pc = UnitVec::Get(x, y);
        while (!AlmostEqual(Real(pc.first.GetX()), Real(x), ulps)) {
            ++ulps;
        }
        while (!AlmostEqual(Real(pc.first.GetY()), Real(y), ulps)) {
            ++ulps;
        }
        while (!AlmostEqual(Real(pc.second), Real(1), ulps)) {
            ++ulps;
        }
    }
    EXPECT_EQ(ulps, 2);
}
