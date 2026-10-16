#pragma once

#ifndef __CONSTRUCTOR
    #define __CONSTRUCTOR __attribute__((constructor))
#endif

#ifndef __CONSTRUCTOR_PRIO
    #define __CONSTRUCTOR_PRIO(p) __attribute__((constructor(p)))
#endif