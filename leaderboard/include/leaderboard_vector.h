#ifndef LEADERBOARD_VECTOR_H
#define LEADERBOARD_VECTOR_H

#include "leaderboard.h"
#include <vector>
#include <algorithm>

namespace Leaderboard {

class LeaderboardVector : public ILeaderboard {
public:
    void addOrUpdate(const std::string& userId, int score) override;
    void remove(const std::string& userId) override;
    std::vector<Entry> topN(size_t n) const override;
    int getRank(const std::string& userId) const override;
    int getScore(const std::string& userId) const override;
private:
    std::vector<Entry> entries_;
};

} // namespace Leaderboard

#endif // LEADERBOARD_VECTOR_H 