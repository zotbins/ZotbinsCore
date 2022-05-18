#include "FullnessMetric.hpp"
#include "MockDistance.hpp"
#include <unity.h>
#include <vector>

// to-do: add more unit tests

void test_mock_distance_sensor_returns_correct_distances()
{
    std::vector<int32_t> distances = {7, 8, 3, 210, 1};
    Fullness::MockDistance mockDistanceSensor(distances);
    for (int i = 0; i < distances.size(); i++)
    {
        TEST_ASSERT_EQUAL_INT(mockDistanceSensor.getDistance(), distances[i]);
    }
}

void test_mock_distance_sensor_returns_correct_distances_and_loops()
{
    std::vector<int32_t> distances = {7, 8, 3, 210, 1};
    Fullness::MockDistance mockDistanceSensor(distances);
    int tempCounter = 0;
    for (int i = 0; i < distances.size() * 2; i++)
    {
        TEST_ASSERT_EQUAL_INT(mockDistanceSensor.getDistance(), distances[i]);
        if (i == distances.size() - 1)
        {
            tempCounter = 0;
        }
        else
        {
            tempCounter++;
        }
    }
}

void test_returns_correct_fullness()
{
    const uint32_t binHeight = 10;
    const float calculatedAverageDistance = 5.5;
    std::vector<int32_t> distances = {7, 8, 3, 2, 10, 4, 20, 0};
    Fullness::MockDistance mockDistanceSensor(distances);

    Fullness::FullnessMetric mockFullness(binHeight, mockDistanceSensor);
    TEST_ASSERT_EQUAL_FLOAT(calculatedAverageDistance, (mockFullness.getFullness()));

    for (int i = 0; i < distances.size(); i++)
    {
        TEST_ASSERT_TRUE(mockFullness.isValidDistance(distances[i]));
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_mock_distance_sensor_returns_correct_distances);
    RUN_TEST(test_mock_distance_sensor_returns_correct_distances_and_loops);
    RUN_TEST(test_returns_correct_fullness);
    UNITY_END();
    return 0;
}