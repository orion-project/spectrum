#include "testing/OriTestBase.h"
#include "tests/orion_tests.h"

namespace Z {
namespace Tests {

USE_GROUP(DataReadersTests)                          // test_DataReadersTests.cpp
USE_GROUP(GraphMathTests)                            // test_GraphMathTests.cpp

TEST_SUITE(
    ADD_GROUP(Ori::Tests::All),
    ADD_GROUP(DataReadersTests),
    ADD_GROUP(GraphMathTests),
)

} // namespace Tests
} // namespace Z
