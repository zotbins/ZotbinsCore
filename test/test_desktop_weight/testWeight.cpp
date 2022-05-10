#include <unity.h>
#include "WeightMetric.hpp"
#include "MockWeight.hpp"

// TODO: Add more tests

void test_always_pass()
{
    TEST_ASSERT_TRUE(true);
}

void test_can_construct_mock_weight_sensor()
{
    Weight::MockWeight mockWeightSensor(5);
}

void test_mock_weight_sensor_returns_correct_weight()
{
    int kg = 10;
    Weight::MockWeight mockWeightSensor(kg);
    TEST_ASSERT_EQUAL_INT(mockWeightSensor.getWeight(), 10);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_always_pass);
    UNITY_END();
    return 0;
}