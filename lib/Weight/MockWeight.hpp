#include "IWeight.hpp"

class MockWeight : IWeight
{
public:
    MockWeight(int32_t mWeight);

    int32_t getWeight() override;

private:
    int32_t mWeight;
};