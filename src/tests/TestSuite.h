#include "testing/OriTestBase.h"
#include "tests/orion_tests.h"

namespace Z {
namespace Tests {

USE_GROUP(DataReadersTests)                          // test_DataReaders.cpp
USE_GROUP(GraphMathTests)                            // test_GraphMath.cpp
USE_GROUP(LuaHelperTests)                            // test_LuaHelper.cpp
USE_GROUP(StringUtilsTests)                          // test_StringUtils.cpp

TEST_SUITE(
    ADD_GROUP(Ori::Tests::All),
    ADD_GROUP(DataReadersTests),
    ADD_GROUP(GraphMathTests),
    ADD_GROUP(LuaHelperTests),
    ADD_GROUP(StringUtilsTests),
)

} // namespace Tests
} // namespace Z
