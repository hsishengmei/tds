#pragma once
#include <atomic>
struct GVC {
    int read() { return version.load(); }
    int addAndFetch() { return version.fetch_add(1)+1; }
    std::atomic<int> version;
};

static GVC gvc;
