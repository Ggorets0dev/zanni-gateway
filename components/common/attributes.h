#pragma once

#ifndef __CONSTRUCTOR
    /**
     * @brief Attribute macro to mark a function as a constructor executed prior to main.
     */
    #define __CONSTRUCTOR __attribute__((constructor))
#endif

#ifndef __CONSTRUCTOR_PRIO
    /**
     * @brief Attribute macro to mark a function as a constructor executed with a given priority.
     * @param p Execution priority value (lower values execute earlier).
     */
    #define __CONSTRUCTOR_PRIO(p) __attribute__((constructor(p)))
#endif