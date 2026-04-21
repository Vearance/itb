#include "CommentsManager.hpp"
#include <algorithm>

void CommentsManager::kickSpammer(int threshold_score)
{
    chat_log_.erase(std::remove_if(chat_log_.begin(), chat_log_.end(), [this, threshold_score](const Comment& e) {
        auto it = reputation_.find(e.getUsername());
        int score = (it == reputation_.end()) ? 0 : it->second;
        return score > threshold_score;
    }), chat_log_.end());
}

void CommentsManager::printViolation()
{
    int ctr = 0;
    std::for_each(chat_log_.begin(), chat_log_.end(), [this, &ctr](const Comment& e) {
        std::for_each(e.getWords().begin(), e.getWords().end(), [this, &ctr, &e](const std::string& f) {
            if (blacklist_.find(f) != blacklist_.end()) {
                ctr++;
                std::cout << "kata \"" << f << "\" oleh akun \"" << e.getUsername() << "\"\n";
            }
        });
    });

    if (ctr == 0) {
        std::cout << "TIDAK ADA PELANGGARAN\n";
    }
}

std::string CommentsManager::quizWinner(const std::set<std::string>& passwords)
{
    auto it = std::find_if(chat_log_.begin(), chat_log_.end(), [&passwords](const Comment& e) {
        return std::find_if(e.getWords().begin(), e.getWords().end(), [&passwords](const std::string& f) {
            return passwords.find(f) != passwords.end();
        }) != e.getWords().end();
    });

    if (it != chat_log_.end()) {
        return it->getUsername();
    }

    return "BELUM ADA PEMENANG";
}

void CommentsManager::upVIPComment()
{
    std::stable_partition(chat_log_.begin(), chat_log_.end(), [](const Comment& e) {
        return e.isVip();
    });
}