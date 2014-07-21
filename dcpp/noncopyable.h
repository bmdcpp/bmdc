
#ifndef DCPLUSPLUS_NONCOPYABLE_H
#define DCPLUSPLUS_NONCOPYABLE_H

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable & operator=(const NonCopyable&) = delete;
};
#endif
