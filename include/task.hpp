#ifndef ZOTBINS_TASK_HPP
#define ZOTBINS_TASK_HPP

#include <FreeRTOS.h>
#include <cstdint>

namespace Zotbins
{
    /**
     * @brief Abstract class for tasks to inherit from
     *
     */
    class Task
    {
    public:
        /**
         * @brief Construct a new Task object. Parameters are needed for FreeRTOS.
         *
         * @param name Name of task
         * @param priority Priority number of task
         * @param stackSize Stack size of task
         */
        Task(const char *name, const int priority, const uint32_t stackSize)
            : mName(name), mPriority(priority), mStackSize(stackSize)
        {
        }

        /**
         * @brief Start execution of task
         *
         */
        virtual void start() = 0;

    protected:
        const char *mName;
        const int mPriority;
        const uint32_t mStackSize;
    };
}

#endif