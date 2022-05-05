#ifndef WEIGHT_HPP
#define WEIGHT_HPP
#include "IWeight.hpp"

/**
 * @brief Acts as main class, taking in weight interface to allow for mock data and real data
 *
 */
class Weight
{
public:
    /**
     * @brief Construct a new Weight object.
     *
     * @param weightSensor An object that is a weight interface object.
     */
    Weight(IWeight &weightSensor);

    /**
     * @brief Returns the weight from the weight interface in kilograms.
     *
     * @return int32_t
     */
    int32_t getWeight();

private:
    /**
     * @brief IWeight object, uses pointer because virtual function
     *
     */
    IWeight *m_weightSensor;
};

#endif