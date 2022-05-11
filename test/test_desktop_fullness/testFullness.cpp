#include "FullnessMetric.hpp"
#include <unity.h>

// TODO: Add more tests

void test_always_pass()
{
    TEST_ASSERT_TRUE(true);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_always_pass);
    UNITY_END();
    return 0;
}