#include "FullnessMetric.hpp"
#include <unity.h>
#include <vector>

// TODO: Add more tests
// CURRENTLY NOT WORKING BECAUSE BRANCH THAT HAS MOCK DISTANCE SENSORS IS NOT PUSHED

void test_always_pass()
{
    TEST_ASSERT_TRUE(true);
}

void test_can_construct_mock_distance_sensor()
{
    Fullness::MockDistance mockDistanceSensor({3, -6, 27, 114, 9});
}

void test_mock_distance_sensor_returns_correct_distances()
{
    std::vector<uint32_t> distances = {7, 8, 3, 210, 1};
    Fullness::MockDistance mockDistanceSensor(distances);
    for (int i = 0; i < distances.size(); i++)
    {
        TEST_ASSERT_EQUAL_INT(mockDistanceSensor.getDistance(), distances[i]);
    }
}

void test_mock_distance_sensor_returns_correct_distances_and_loops()
{
    std::vector<uint32_t> distances = {7, 8, 3, 210, 1};
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

// NOT SURE IF I'M DOING THIS RIGHT
void test_returns_correct_fullness()
{
    uint32_t binHeight = 10;
    int32_t calculatedAverageDistance = 5; // should be 5.5
    std::vector<uint32_t> distances = {7, 8, 3, 2, 10, 4, 20, 0};
    Fullness::MockDistance mockDistanceSensor(distances);

    Fullness::Fullness mockFullness(binHeight, mockDistanceSensor);
    TEST_ASSERT_EQUAL_INT(calculatedAverageDistance, mockFullness.getFullness(distances, mockDistanceSensor));

    for (int i = 0; i < distances.size(); i++)
    {
        TEST_ASSERT_TRUE(mockFullness.isValidDistance(distances[i], binHeight));
    }
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_always_pass);
    UNITY_END();
    return 0;
}