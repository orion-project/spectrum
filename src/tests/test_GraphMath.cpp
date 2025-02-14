#include "core/GraphMath.h"

#include "testing/OriTestBase.h"

#include <QDebug>

using namespace GraphMath;

static bool compareDouble(double value, double expected)
{
    return QString::number(value) == QString::number(expected);
}

#define ASSERT_ARR_SAME(arr, expected) \
    ASSERT_EQ_LIST_EX(arr, expected, compareDouble);

#define ASSERT_ARR(arr, expected...) { \
    Values tmp({ expected }); \
    ASSERT_EQ_LIST_EX(arr, tmp, compareDouble);\
}

namespace Z {
namespace Tests {
namespace GraphMathTests {

namespace MovingAverageTests {

TEST_METHOD(simple_with_points)
{
    Values xs = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10};
    Values ys = {10, 15, 10, 30, 20, 45, 70, 50, 40, 60};
    MavgSimple m;
    m.points = 5;
    m.useStep = false;
    auto r = m.calc({xs, ys});
    ASSERT_ARR_SAME(r.xs, xs);
    ASSERT_ARR(r.ys, 17.0, 24.0, 35.0, 43.0, 45.0, 53.0);
}

TEST_METHOD(simple_with_step)
{
    Values xs = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10};
    Values ys = {10, 15, 10, 30, 20, 45, 70, 50, 40, 60};
    MavgSimple m;
    m.step = 5.5;
    m.useStep = true;
    {
        auto r = m.calc({xs, ys});
        ASSERT_ARR_SAME(r.xs, xs);
        ASSERT_ARR(r.ys,17.0, 24.0, 35.0, 43.0, 45.0, 53.0);
    }
    m.step = 6.5;
    {
        auto r = m.calc({xs, ys});
        ASSERT_ARR_SAME(r.xs, xs);
        ASSERT_ARR(r.ys, 21.666666666666664, 31.666666666666664, 37.5, 42.49999999999999, 47.5);
    }
}

TEST_METHOD(cumulative)
{
    Values xs = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10};
    Values ys = {10, 15, 10, 30, 20, 45, 70, 50, 40, 60};
    MavgCumul m;
    auto r = m.calc({xs, ys});
    ASSERT_ARR_SAME(r.xs, xs);
    ASSERT_ARR(r.ys,
        10.0, 12.5, 11.666666666666666, 16.25, 17.0, 21.666666666666668, 28.571428571428573, 31.25, 32.22222222222222, 35.0);
}

TEST_METHOD(exponential)
{
    Values xs = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10};
    Values ys = {10, 15, 10, 30, 20, 45, 70, 50, 40, 60};
    MavgExp m;
    m.alpha = 0.5;
    auto r = m.calc({xs, ys});
    ASSERT_ARR_SAME(r.xs, xs);
    ASSERT_ARR(r.ys,
        10.0, 12.5, 11.25, 20.625, 20.3125, 32.65625, 51.328125, 50.6640625, 45.33203125, 52.666015625);
}

TEST_GROUP("MovingAverage",
    ADD_TEST(simple_with_points),
    ADD_TEST(simple_with_step),
    ADD_TEST(cumulative),
    ADD_TEST(exponential),
)

} // MovingAverageTests

//------------------------------------------------------------------------------

namespace DerivativeTests {

TEST_METHOD(calc)
{
    Values xs = {0, 1, 2, 3, 4};
    Values ys = {0, 1, 4, 9, 16};
    Derivative d;
    auto r = d.calc({xs, ys});
    ASSERT_ARR_SAME(r.xs, xs);
    ASSERT_ARR(r.ys, 1.0, 2.0, 4.0, 6.0, 7.0);
}

TEST_GROUP("Derivative",
    ADD_TEST(calc),
)

} // DerivativeTests

//------------------------------------------------------------------------------

TEST_GROUP("Graph Math",
    ADD_GROUP(MovingAverageTests),
    ADD_GROUP(DerivativeTests),
)


} // GraphMathTests
} // Tests
} // Z
