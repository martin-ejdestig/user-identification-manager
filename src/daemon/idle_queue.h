// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_IDLE_QUEUE_H
#define UIM_DAEMON_IDLE_QUEUE_H

#include <glibmm.h>

#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace UserIdentificationManager::Daemon
{
    // Thread safe queue that notifies the main thread when it is idle.
    //
    // Meant to isolate thread synchronization details as much as possible from the rest of the
    // program.
    //
    // Any thread can push values to the queue. A callback will be invoked in the main
    // thread for each value popped from the queue. The main thread is expected to run a
    // Glib::MainLoop with the default context. See g_idle_add() for more details.
    template <typename T>
    class IdleQueue
    {
    public:
        using Callback = std::function<void(const T &)>;

        IdleQueue() = default;

        ~IdleQueue()
        {
            std::lock_guard<std::mutex> lock(shared_.mutex);

            if (shared_.idle_source_id != 0) {
                g_source_remove(shared_.idle_source_id);
            }
        }

        IdleQueue(const IdleQueue &other) = delete;
        IdleQueue(IdleQueue &&other) = delete;
        IdleQueue &operator=(const IdleQueue &other) = delete;
        IdleQueue &operator=(IdleQueue &&other) = delete;

        void push(T &&value)
        {
            std::lock_guard<std::mutex> lock(shared_.mutex);

            shared_.values.emplace_back(std::move(value));

            if (shared_.idle_source_id == 0) {
                shared_.idle_source_id = g_idle_add(&idle_function, this);
            }
        }

        void set_callback(Callback &&callback)
        {
            callback_ = std::move(callback);
        }

        void clear_callback()
        {
            set_callback(Callback());
        }

    private:
        static int idle_function(void *data)
        {
            static_cast<IdleQueue *>(data)->pop_and_invoke_callback();
            return G_SOURCE_REMOVE;
        }

        void pop_and_invoke_callback()
        {
            std::vector<T> popped_values;
            {
                std::lock_guard<std::mutex> lock(shared_.mutex);
                popped_values = std::move(shared_.values);
                shared_.idle_source_id = 0;
            }

            for (const T &value : popped_values) {
                if (callback_) {
                    callback_(value);
                }
            }
        }

        struct
        {
            std::mutex mutex;
            std::vector<T> values;
            guint idle_source_id = 0;
        } shared_;

        Callback callback_;
    };
}

#endif // UIM_DAEMON_IDLE_QUEUE_H
