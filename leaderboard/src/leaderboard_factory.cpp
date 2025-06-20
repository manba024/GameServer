#include "leaderboard.h"
#include "leaderboard_vector.h"
#include "leaderboard_set.h"
#include "leaderboard_heap.h"
#include "leaderboard_skiplist.h"
#include <memory>

namespace Leaderboard {

std::unique_ptr<ILeaderboard> createLeaderboard(LeaderboardType type) {
    switch (type) {
        case LeaderboardType::Vector:
            return std::unique_ptr<ILeaderboard>(new LeaderboardVector());
        case LeaderboardType::Set:
            return std::unique_ptr<ILeaderboard>(new LeaderboardSet());
        case LeaderboardType::Heap:
            return std::unique_ptr<ILeaderboard>(new LeaderboardHeap());
        case LeaderboardType::SkipList:
            return std::unique_ptr<ILeaderboard>(new LeaderboardSkipList());
        default:
            return nullptr;
    }
}

} // namespace Leaderboard 