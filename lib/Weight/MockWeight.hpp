#include "IWeight.hpp"

/**
 * @brief Mock weight class that inherits from the IWeight interface class.
 * Used for unit tests so that actual hardware isn't require to test the ZotBins
 * system.
 */
class MockWeight final : public IWeight 
{
public:
    /**
     * @brief Constructs an instance of the MockWeight class.
     * @param mWeight Represents the current weight of the mock sensor.
     *  
     */
    MockWeight(int32_t weight);

    /** 
     * @brief Returns the mock weight in kilograms.
     * @return int32_t
     */
    int32_t getWeight() override;

private:
    /**
     * @brief Represents the current weight of the mock sensor.
     * 
     */
    int32_t mWeight;
};