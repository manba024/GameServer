#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <vector>
#include <string>
#include <memory>

namespace Leaderboard {

struct Entry {
    std::string userId;
    int score;
};

enum class LeaderboardType {
    Vector,
    Set,
    Heap,
    SkipList
};

class ILeaderboard {
public:
    virtual ~ILeaderboard() = default;
    virtual void addOrUpdate(const std::string& userId, int score) = 0;
    virtual void remove(const std::string& userId) = 0;
    virtual std::vector<Entry> topN(size_t n) const = 0;
    virtual int getRank(const std::string& userId) const = 0;
    virtual int getScore(const std::string& userId) const = 0;
};

std::unique_ptr<ILeaderboard> createLeaderboard(LeaderboardType type);

} // namespace Leaderboard

#endif // LEADERBOARD_H 