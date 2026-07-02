#include "core/StringUtils.h"

#include "testing/OriTestBase.h"

using namespace StringUtils;

namespace Z {
namespace Tests {
namespace StringUtilsTests {

//------------------------------------------------------------------------------

namespace DiceSimilarityTests {

TEST_CASE_METHOD(dice_similarity, const QString &s1, const QString &s2, double expectedSimilarity)
{
    double similarity = diceSimilarity(s1, s2);
    ASSERT_NEAR_DBL(similarity, expectedSimilarity, 1e-3);
}

TEST_CASE(too_short_1, dice_similarity, "d", "demo", 0)
TEST_CASE(too_short_2, dice_similarity, "demo", "d", 0)
TEST_CASE(not_equal, dice_similarity, "demo", "home", 0)
TEST_CASE(some_equal, dice_similarity, "demo", "domo", 1/3.0)
TEST_CASE(half_equal, dice_similarity, "demo", "de", 0.5)
TEST_CASE(more_equal, dice_similarity, "demo", "mode", 2/3.0)
TEST_CASE(almost_equal, dice_similarity, "demo1", "demo2", 0.75)
TEST_CASE(equal, dice_similarity, "demo", "demo", 1)

TEST_METHOD(compare_index)
{
    double s1 = diceSimilarity("file_#", "file_##"); TEST_LOG_DOUBLE(s1)
    double s2 = diceSimilarity("file_#", "file_###"); TEST_LOG_DOUBLE(s2)
    double s3 = diceSimilarity("file_##", "file_###"); TEST_LOG_DOUBLE(s3)
    ASSERT_IS_TRUE(s1 == s2);
    ASSERT_IS_TRUE(s3 > s2);
    ASSERT_IS_TRUE(s3 == 1);
}

TEST_METHOD(debug_threshold)
{
    TEST_LOG_DOUBLE(diceSimilarity("######_laser####.dat", "######_lasernoise####.dat"))
    TEST_LOG_DOUBLE(diceSimilarity("#_laser#.dat", "#_lasernoise#.dat"))
    TEST_LOG_DOUBLE(diceSimilarity("#-#-#_laser#.dat", "#-#-#_lasernoise#.dat"))
    TEST_LOG("---------")
    TEST_LOG_DOUBLE(diceSimilarity("mroi_measur_########.csv", "mroi_measur_#.csv"))
    TEST_LOG_DOUBLE(diceSimilarity("led_measur_########.csv", "mroi_measur_########.csv"))
    TEST_LOG_DOUBLE(diceSimilarity("led_measur_#.csv", "mroi_measur_#.csv"))
    TEST_LOG("---------")
    TEST_LOG_DOUBLE(diceSimilarity("noise_######.dat", "######_laser####.dat"))
    TEST_LOG_DOUBLE(diceSimilarity("noise_#.dat", "#_laser#.dat"))
}

TEST_GROUP("DiceSimilarity",
    ADD_TEST(too_short_1),
    ADD_TEST(too_short_2),
    ADD_TEST(not_equal),
    ADD_TEST(some_equal),
    ADD_TEST(half_equal),
    ADD_TEST(more_equal),
    ADD_TEST(almost_equal),
    ADD_TEST(equal),
    ADD_TEST(compare_index),
    ADD_TEST(debug_threshold),
)

} // DiceSimilarityTests

//------------------------------------------------------------------------------

namespace LevenshteinDistanceTests
{

TEST_CASE_METHOD(levenshtein, const QString &s1, const QString &s2, int expectedDistance)
{
    int distance = levenshteinDistance(s1, s2);
    ASSERT_EQ_INT(distance, expectedDistance);
}

TEST_CASE(empty1, levenshtein, "", "demo", 4)
TEST_CASE(empty2, levenshtein, "demo", "", 4)
TEST_CASE(test1, levenshtein, "file_#", "file_##", 1)
TEST_CASE(test2, levenshtein, "file_#", "file_###", 2)
TEST_CASE(test3, levenshtein, "demo_#", "file_#", 4)
TEST_CASE(test4, levenshtein, "file_#", "demo_####", 7)

TEST_GROUP("LevenshteinDistance",
    ADD_TEST(empty1),
    ADD_TEST(empty2),
    ADD_TEST(test1),
    ADD_TEST(test2),
    ADD_TEST(test3),
    ADD_TEST(test4),
)

} // LevenshteinDistanceTests

//------------------------------------------------------------------------------

namespace SelectSimilarFileNameTests
{

TEST_METHOD(various_names)
{
    QStringList fileNames = {
        "mroi_measur_1.csv", // 0
        "led_measur_1.csv",  // 1
        "led_measur_2.csv",  // 2
        "led_measur_2025-04-21.csv", // 3
        "260209_laser1550.dat", // 4
        "220404_1_Freq_A_1.allan.txt", // 5
        "220429_2_Freq_A_1.allan.txt", // 6
    };
    
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_1.csv", fileNames).index, 0);
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_2.csv", fileNames).index, 0);
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_20260703.csv", fileNames).index, 0);
    ASSERT_EQ_INT(selectDiceSimilar("led_measur_3.csv", fileNames).index, 2);
    ASSERT_EQ_INT(selectDiceSimilar("led_measur_2025-05-21.csv", fileNames).index, 3);
    ASSERT_EQ_INT(selectDiceSimilar("led_measur_2025-05-21.csv", fileNames).index, 3);
    ASSERT_EQ_INT(selectDiceSimilar("260219_laser1550.dat", fileNames).index, 4);
    ASSERT_EQ_INT(selectDiceSimilar("260219_laser1551.dat", fileNames).index, 4);
    ASSERT_EQ_INT(selectDiceSimilar("260219_lasernoise1551.dat", fileNames).index, -1);
    ASSERT_EQ_INT(selectDiceSimilar("noise_260219.dat", fileNames).index, -1);
    ASSERT_EQ_INT(selectDiceSimilar("220512_1_Freq_A_1.allan.txt", fileNames).index, 6);
}

TEST_METHOD(not_very_different_names)
{
    QStringList fileNames = {
        "led_measur_1.csv",
        "led_measur_2.csv",
        "led_measur_2025-04-21.csv",
    };

    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_1.csv", fileNames).index, -1);
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_2.csv", fileNames).index, -1);
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_20260703.csv", fileNames).index, -1);
}

TEST_METHOD(very_different_names)
{
    QStringList fileNames = {
        "220404_1_Freq_A_1.allan.txt",
        "220429_2_Freq_A_1.allan.txt",
    };

    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_1.csv", fileNames).index, -1);
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_2.csv", fileNames).index, -1);
    ASSERT_EQ_INT(selectDiceSimilar("mroi_measur_20260703.csv", fileNames).index, -1);
}

TEST_GROUP("SelectSimilarFileName",
    ADD_TEST(various_names),
    ADD_TEST(not_very_different_names),
    ADD_TEST(very_different_names),
)

} // SelectSimilarFileNameTests

//------------------------------------------------------------------------------

TEST_GROUP("String Utils",
    ADD_GROUP(DiceSimilarityTests),
    ADD_GROUP(LevenshteinDistanceTests),
    ADD_GROUP(SelectSimilarFileNameTests),
)

} // StringUtilsTests
} // Tests
} // Z
