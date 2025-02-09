#include "core/GraphMath.h"

#include "testing/OriTestBase.h"

#include <QDebug>

using namespace GraphMath;

static bool compareDouble(double value, double expected)
{
    return QString::number(value) == QString::number(expected);
}

namespace Z {
namespace Tests {
namespace GraphMathTests {

namespace MovingAverageTests {

TEST_METHOD(simple_with_points)
{
    QVector<double> xs({1,  2,  3,  4,  5,  6,  7,  8,  9,  10});
    QVector<double> ys({10, 15, 10, 30, 20, 45, 70, 50, 40, 60});
    MavgSimple m;
    m.points = 5;
    m.useStep = false;
    auto r = m.calc({xs, ys});
    ASSERT_EQ_LIST_EX(r.xs, xs, compareDouble);
    QVector<double> resY({17.0, 24.0, 35.0, 43.0, 45.0, 53.0});
    ASSERT_EQ_LIST_EX(r.ys, resY, compareDouble);
}

TEST_METHOD(simple_with_step)
{
    QVector<double> xs({1,  2,  3,  4,  5,  6,  7,  8,  9,  10});
    QVector<double> ys({10, 15, 10, 30, 20, 45, 70, 50, 40, 60});
    MavgSimple m;
    m.step = 5.5;
    m.useStep = true;
    {
        auto r = m.calc({xs, ys});
        ASSERT_EQ_LIST_EX(r.xs, xs, compareDouble);
        QVector<double> resY({17.0, 24.0, 35.0, 43.0, 45.0, 53.0});
        ASSERT_EQ_LIST_EX(r.ys, resY, compareDouble);
    }
    m.step = 6.5;
    {
        auto r = m.calc({xs, ys});
        ASSERT_EQ_LIST_EX(r.xs, xs, compareDouble);
        QVector<double> resY({21.666666666666664, 31.666666666666664, 37.5, 42.49999999999999, 47.5});
        ASSERT_EQ_LIST_EX(r.ys, resY, compareDouble);
    }
}

TEST_METHOD(cumulative)
{
    QVector<double> xs({1,  2,  3,  4,  5,  6,  7,  8,  9,  10});
    QVector<double> ys({10, 15, 10, 30, 20, 45, 70, 50, 40, 60});
    MavgCumul m;
    auto r = m.calc({xs, ys});
    ASSERT_EQ_LIST_EX(r.xs, xs, compareDouble);
    QVector<double> resY({10.0, 12.5, 11.666666666666666, 16.25, 17.0, 21.666666666666668, 28.571428571428573, 31.25, 32.22222222222222, 35.0});
    ASSERT_EQ_LIST_EX(r.ys, resY, compareDouble);
}

TEST_METHOD(exponential)
{
    QVector<double> xs({1,  2,  3,  4,  5,  6,  7,  8,  9,  10});
    QVector<double> ys({10, 15, 10, 30, 20, 45, 70, 50, 40, 60});
    MavgExp m;
    m.alpha = 0.5;
    auto r = m.calc({xs, ys});
    ASSERT_EQ_LIST_EX(r.xs, xs, compareDouble);
    QVector<double> resY({10.0, 12.5, 11.25, 20.625, 20.3125, 32.65625, 51.328125, 50.6640625, 45.33203125, 52.666015625});
    ASSERT_EQ_LIST_EX(r.ys, resY, compareDouble);
}

TEST_GROUP("MovingAverage",
    ADD_TEST(simple_with_points),
    ADD_TEST(simple_with_step),
    ADD_TEST(cumulative),
    ADD_TEST(exponential),
)

} // MovingAverageTests

//------------------------------------------------------------------------------

TEST_GROUP("Graph Math",
    ADD_GROUP(MovingAverageTests),
)


} // GraphMathTests
} // Tests
} // Z
