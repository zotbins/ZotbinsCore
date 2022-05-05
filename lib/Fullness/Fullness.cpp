/**
 * @file Fullness.cpp
 * @brief C file for Fullness class
 * @version 0.1
 * @date 2022-05-04
 *
 */

#include "Fullness.hpp"
#include "IDistance.hpp"
#include <Arduino.h>
#include <bits/stdc++.h>
#include <cstdint>

using namespace Zotbins;

#define SENSOR_CALLS 8 // made it a number divisible by 4 for IQM calculations

// using constructor
Fullness::Fullness(int32_t binHeight, IDistance::IDistance &distanceSensor)
{
    height = binHeight;
    sensor = &distanceSensor;
}

float Fullness::getFullness()
{
    // call getDistance on sensor SENSOR_CALLS times to get an array of distances
    int distances[SENSOR_CALLS] = {};
    for (int i = 0; i < SENSOR_CALLS; i++)
    {
        if (sensor->getDistance() >= 0) // checking getDistance to make sure function won't get stuck
        {
            distances[i] = sensor->getDistance();
        }
        else
        {
            return -1;
        }
    }

    // use IQM to get average distance
    int averageDistance = IQM(distances, SENSOR_CALLS);

    // fullness = (height - distance) / height
    int fullness = (height - averageDistance) / height;

    // return fullness
    return fullness;
}

bool Fullness::isValidFullness()
{
    if (height < sensor->getDistance())
    {
        return false;
    }
    else if (getFullness() < 0)
    {
        return false;
    }
    return true;
}

float IQM(int *a, int n)
{
    // return variable
    int averageDistance = 0;

    // sort the data
    shellSort(a, n);

    // find the average not including the first two and last two indexes
    for (int i = 2; i < SENSOR_CALLS - 2; i++)
    {
        averageDistance += a[i];
    }
    averageDistance /= (SENSOR_CALLS - 4);

    return averageDistance;
}

int shellSort(int arr[], int n)
{
    for (int gap = n / 2; gap > 0; gap /= 2)
    {
        for (int i = gap; i < n; i += 1)
        {
            int temp = arr[i];

            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap)
                arr[j] = arr[j - gap];

            arr[j] = temp;
        }
    }
    return 0;
}