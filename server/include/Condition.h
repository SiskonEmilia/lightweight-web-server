#pragma once

#include <pthread.h>

#include "base/Uncopyable.h"
#include "MutexLock.h"

class Condition : private Uncopyable{

    MutexLock &raw_mutex;

public:
    
};