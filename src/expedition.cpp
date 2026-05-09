#include "expedition.h"

namespace guild {
    // ============================================================
    // ExpeditionClient
    // ============================================================

    std::future<std::optional<Attribute> > ExpeditionClient::async_query(const std::string &key) {
        // TODO: 实现异步查询
        auto future = pool_.dispatch([this,key] { return registry_.query(key); });

        return future;
    }

    std::future<void> ExpeditionClient::async_register(const std::string &key, Attribute value,
                                                       std::chrono::seconds ttl) {
        // TODO: 实现异步注册
        auto future = pool_.dispatch([this,key,value,ttl] { registry_.register_adventurer(key, value, ttl); });

        return future;
    }

    std::future<bool> ExpeditionClient::async_dismiss(const std::string &key) {
        // TODO: 实现异步删除
        auto future = pool_.dispatch([this,key] { return registry_.dismiss(key); });
        return future;
    }

    void ExpeditionClient::async_query_with_callback(
        const std::string &key,
        std::function<void(std::optional<Attribute>)> callback) {
        // TODO: 实现带回调的异步查询（加分项）
        (void) key;
        (void) callback;
    }

    // ============================================================
    // BatchDispatcher
    // ============================================================

    std::vector<QuestResult> BatchDispatcher::launch(const std::vector<Quest> &quests) {
        // TODO: 实现批量并行执行

        std::vector<QuestResult> result;
        result.reserve(quests.size());
        std::vector<std::variant<std::future<std::optional<Attribute> >, std::future<void>, std::future<bool> > >
                futures;

        for (auto &quest: quests) {
            switch (quest.type) {
                case QuestType::kQuery:
                    futures.emplace_back(pool_.dispatch([this,quest] {return registry_.query(quest.key); }));
                    break;
                case QuestType::kRegister:
                    futures.emplace_back(pool_.dispatch([this,quest] { registry_.register_adventurer(quest.key, quest.value.value(), quest.ttl); }));
                    break;
                case QuestType::kRemove:
                    futures.emplace_back(pool_.dispatch([this,quest] {return registry_.dismiss(quest.key); }));
                    break;
            }
        }

        for (auto &future : futures) {
            std::visit([&result](auto&& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T,std::future<std::optional<Attribute>> >) {
                    auto res = v.get();
                    if (res.has_value()) result.emplace_back(guild::QuestResult::ok(res.value()));
                    else result.emplace_back(guild::QuestResult::not_found());
                }
                else if constexpr (std::is_same_v<T,std::future<void> >) {
                    result.emplace_back(guild::QuestResult::ok());
                }
                else if constexpr (std::is_same_v<T,std::future<bool> >) {
                    auto res = v.get();
                    if (res) result.emplace_back(guild::QuestResult::ok());
                    else result.emplace_back(guild::QuestResult::not_found());
                }
            },future);
        }

        return result;
    }
} // namespace guild
