// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/idle_queue.h"

#include <glibmm.h>
#include <gtest/gtest.h>

#include <thread>
#include <vector>

namespace UserIdentificationManager::Daemon
{
    TEST(IdleQueue, CallbackCalledWithPushedValuesInOrder)
    {
        Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();
        IdleQueue<int> idle_queue;
        std::vector<int> popped_values;

        idle_queue.set_callback([&](int i) {
            popped_values.emplace_back(i);

            if (popped_values.size() == 3) {
                main_loop->quit();
            }
        });

        idle_queue.push(1);
        idle_queue.push(2);
        idle_queue.push(3);

        EXPECT_TRUE(popped_values.empty());

        main_loop->run();

        EXPECT_EQ(std::vector<int>({1, 2, 3}), popped_values);
    }

    TEST(IdleQueue, CallbackCalledInMainThreadWhenValuesPushedInOtherThread)
    {
        Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();
        IdleQueue<std::thread::id> idle_queue;
        std::thread::id callback_thread_id;
        std::thread::id push_thread_id;

        idle_queue.set_callback([&](std::thread::id id) {
            callback_thread_id = std::this_thread::get_id();
            push_thread_id = id;
            main_loop->quit();
        });

        std::thread thread([&] { idle_queue.push(std::this_thread::get_id()); });

        main_loop->run();

        thread.join();

        EXPECT_EQ(std::this_thread::get_id(), callback_thread_id);
        EXPECT_NE(std::this_thread::get_id(), push_thread_id);
    }
}
