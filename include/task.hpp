/**
 * @file task.hpp
 * @brief Header file for Task class
 * @version 0.1
 * @date 2022-04-27
 *
 */

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
        /**
         * @brief Name of task
         *
         */
        const char *mName;

        /**
         * @brief Priority number of task
         *
         */
        const int mPriority;
        
        /**
         * @brief Task size of task
         *
         */
        const uint32_t mStackSize;
    };
}

#endif