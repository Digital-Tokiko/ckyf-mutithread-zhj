#include "dispatch_pool.h"

namespace guild {
    DispatchPool::DispatchPool(size_t num_windows) {
        // TODO: 创建 num_windows 个 worker 线程，存入 workers_
        (void) num_windows;

        for (size_t i = 0; i < num_windows; i++) {
            std::thread worker([this] { worker_loop(); });
            workers_.push_back(std::move(worker));
        }
    }

    void DispatchPool::worker_loop() {
        // TODO: 循环从 board_ 取任务并执行，直到 board_ 关闭且为空时退出
        while (true) {
            auto task = board_.wait_and_take(); //哦对阿，拿了要干，不能扔掉。

            if (task.has_value()) {
                task.value()();
            }
            if (board_.is_closed() && pending_count() == 0) break;
        }
    }

    void DispatchPool::shutdown() {
        // TODO: 设置关闭标志，通知 board_ 停止，等待所有 worker 线程退出

        shutdown_.store(true);
        board_.close_board();

        for (auto &worker: workers_) {
            if (worker.joinable()) worker.join(); //判断必须得是joinable
        }
    }

    DispatchPool::~DispatchPool() {
        shutdown();
    }
} // namespace guild
