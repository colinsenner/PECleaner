#pragma once

#define AddPtr(ptr, offset) (void*) (((SIZE_T)ptr) + ((SIZE_T)offset))
#define SubPtr(ptr, offset) (void*) (((SIZE_T)ptr) - ((SIZE_T)offset))
#define ValueAt(ptr, offset, type) (*(type*)AddPtr(ptr, offset))