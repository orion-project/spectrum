#include "core/DataReaders.h"

#include "testing/OriTestBase.h"

#include <QDebug>

namespace Z {
namespace Tests {
namespace DataReadersTests {

namespace LineSplitterTests {

static bool comparePart(QStringView value, const QString &expected)
{
    return value.compare(expected) == 0;
}

#define ASSERT_SPLIT(line, ...) { \
    LineSplitter s; \
    QString line_str(line); \
    auto expected = QStringList({__VA_ARGS__}); \
    s.split(line_str); \
    ASSERT_EQ_LIST_EX(s.parts, expected, comparePart); \
}

#define ASSERT_SPLIT_EX(line, seps, ...) { \
    LineSplitter s(seps); \
    QString line_str(line); \
    auto expected = QStringList({__VA_ARGS__}); \
    s.split(line_str); \
    ASSERT_EQ_LIST_EX(s.parts, expected, comparePart); \
}

TEST_METHOD(detect_single_separator)
{
    ASSERT_SPLIT("1 2         3",           "1", "2", "3");
    ASSERT_SPLIT("4\t\t\t5\t6", "4",        "5", "6");
    ASSERT_SPLIT("7,8,,9",                  "7", "8", "", "9");
    ASSERT_SPLIT("10;;11;12",               "10", "", "11", "12");
    ASSERT_SPLIT("")
}

TEST_METHOD(empty_input)
{
    ASSERT_SPLIT("")
    ASSERT_SPLIT("      ")
    ASSERT_SPLIT("\t\t\t")
}

TEST_METHOD(use_first_detected_separator)
{
    ASSERT_SPLIT("1 2\t3",                  "1", "2\t3");
    ASSERT_SPLIT("4\t5 6",                  "4", "5 6");
    ASSERT_SPLIT("7,8 9",                   "7", "8 9");
    ASSERT_SPLIT("10;11,12",                "10", "11,12");
}

TEST_METHOD(take_more_tnan_one_parts)
{
    ASSERT_SPLIT("    \t    ",              "    ", "    ")
    ASSERT_SPLIT("\t\t  \t\t",              "\t\t", "\t\t")
}

TEST_METHOD(keep_empty_parts)
{
    ASSERT_SPLIT(",,,;;",                   "", "", "", ";;");
    ASSERT_SPLIT(";;;,,",                   "", "", "", ",,");
}

TEST_METHOD(use_several_separators)
{
    ASSERT_SPLIT_EX("1   20\t\t3,4;;5",   " \t,;",   "1", "20", "3", "", "4")
    ASSERT_SPLIT_EX("1 20\t3,4;;5",       " \t,",    "1", "20", "3;;4")
}

TEST_GROUP("LineSplitter",
    ADD_TEST(detect_single_separator),
    ADD_TEST(empty_input),
    ADD_TEST(use_first_detected_separator),
    ADD_TEST(take_more_tnan_one_parts),
    ADD_TEST(keep_empty_parts),
    ADD_TEST(use_several_separators),
)

} // namespace LineSplitterTests


//------------------------------------------------------------------------------

TEST_GROUP("Data Reades",
    ADD_GROUP(LineSplitterTests),
)

} // namespace DataReadersTests
} // namespace Tests
} // namespace Z
