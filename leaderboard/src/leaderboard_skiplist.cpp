#include "leaderboard_skiplist.h"
#include <climits>

namespace Leaderboard {

LeaderboardSkipList::LeaderboardSkipList() : level_(0), rng_(std::random_device{}()) {
    // 创建头节点，使用最小可能的值
    Entry headerEntry = {"", INT_MIN};
    header_ = new SkipListNode(headerEntry, MAX_LEVEL);
}

LeaderboardSkipList::~LeaderboardSkipList() {
    SkipListNode* current = header_->forward[0];
    while (current != nullptr) {
        SkipListNode* next = current->forward[0];
        delete current;
        current = next;
    }
    delete header_;
}

void LeaderboardSkipList::addOrUpdate(const std::string& userId, int score) {
    // 如果用户已存在，先删除
    auto nodeIt = userNodes_.find(userId);
    if (nodeIt != userNodes_.end()) {
        deleteNode(nodeIt->second);
        userNodes_.erase(nodeIt);
    }
    
    // 插入新节点
    std::vector<SkipListNode*> update(MAX_LEVEL + 1);
    SkipListNode* current = header_;
    
    // 从最高层开始查找插入位置
    for (int i = level_; i >= 0; i--) {
        while (current->forward[i] != nullptr && 
               shouldGoHigher(current->forward[i]->entry, {userId, score})) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    
    // 确定新节点的层数
    int newLevel = randomLevel();
    if (newLevel > level_) {
        for (int i = level_ + 1; i <= newLevel; i++) {
            update[i] = header_;
        }
        level_ = newLevel;
    }
    
    // 创建新节点
    Entry newEntry = {userId, score};
    SkipListNode* newNode = new SkipListNode(newEntry, newLevel);
    
    // 更新指针
    for (int i = 0; i <= newLevel; i++) {
        newNode->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = newNode;
    }
    
    userNodes_[userId] = newNode;
}

void LeaderboardSkipList::remove(const std::string& userId) {
    auto nodeIt = userNodes_.find(userId);
    if (nodeIt != userNodes_.end()) {
        deleteNode(nodeIt->second);
        userNodes_.erase(nodeIt);
    }
}

std::vector<Entry> LeaderboardSkipList::topN(size_t n) const {
    std::vector<Entry> result;
    result.reserve(n);
    
    SkipListNode* current = header_->forward[0];
    while (current != nullptr && result.size() < n) {
        result.push_back(current->entry);
        current = current->forward[0];
    }
    
    return result;
}

int LeaderboardSkipList::getRank(const std::string& userId) const {
    auto nodeIt = userNodes_.find(userId);
    if (nodeIt == userNodes_.end()) {
        return -1; // 用户不存在
    }
    
    int rank = 1;
    SkipListNode* current = header_->forward[0];
    
    while (current != nullptr && current->entry.userId != userId) {
        rank++;
        current = current->forward[0];
    }
    
    return current != nullptr ? rank : -1;
}

int LeaderboardSkipList::getScore(const std::string& userId) const {
    auto nodeIt = userNodes_.find(userId);
    if (nodeIt != userNodes_.end()) {
        return nodeIt->second->entry.score;
    }
    return -1; // 用户不存在
}

int LeaderboardSkipList::randomLevel() {
    int level = 0;
    while ((rng_() & 1) && level < MAX_LEVEL) {
        level++;
    }
    return level;
}

bool LeaderboardSkipList::shouldGoHigher(const Entry& a, const Entry& b) const {
    // 分数高的排在前面，分数相同时按 userId 字典序排列
    if (a.score != b.score) {
        return a.score > b.score;
    }
    return a.userId < b.userId;
}

LeaderboardSkipList::SkipListNode* LeaderboardSkipList::find(const std::string& userId) const {
    auto it = userNodes_.find(userId);
    return (it != userNodes_.end()) ? it->second : nullptr;
}

void LeaderboardSkipList::deleteNode(SkipListNode* nodeToDelete) {
    std::vector<SkipListNode*> update(MAX_LEVEL + 1);
    SkipListNode* current = header_;
    
    // 找到所有指向待删除节点的前驱节点
    for (int i = level_; i >= 0; i--) {
        while (current->forward[i] != nullptr && 
               current->forward[i] != nodeToDelete &&
               shouldGoHigher(current->forward[i]->entry, nodeToDelete->entry)) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    
    // 更新指针
    for (int i = 0; i <= level_; i++) {
        if (update[i]->forward[i] == nodeToDelete) {
            update[i]->forward[i] = nodeToDelete->forward[i];
        }
    }
    
    // 删除节点
    delete nodeToDelete;
    
    // 更新跳表的最高层数
    while (level_ > 0 && header_->forward[level_] == nullptr) {
        level_--;
    }
}

} // namespace Leaderboard 