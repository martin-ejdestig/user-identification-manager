// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_PCSC_CONTEXT_H
#define UIM_DAEMON_PCSC_CONTEXT_H

#include <glibmm.h>
#include <winscard.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include "daemon/idle_queue.h"

namespace UserIdentificationManager::Daemon
{
    // Helper class for pcsclite.
    //
    // Starts a separate thread where all SCard API calls are made since there is no way to
    // integrate nicely with a main loop. All public methods are meant to be called from the main
    // thread. Any callbacks invoked by PCSCContext are quaranteed to be invoked in the main thread.
    // The idea is to hide all syncronization with the thread performing SCard API calls and the
    // rest of the program in PCSCContext.
    class PCSCContext
    {
    public:
        struct ExtractedUID
        {
            std::vector<std::uint8_t> uid;
            std::string reader_name;
        };

        using UIDQueue = IdleQueue<ExtractedUID>;

        static PCSCContext &instance();

        PCSCContext(const PCSCContext &other) = delete;
        PCSCContext(PCSCContext &&other) = delete;
        PCSCContext &operator=(const PCSCContext &other) = delete;
        PCSCContext &operator=(PCSCContext &&other) = delete;

        void uid_extract_enable(UIDQueue::Callback &&callback);
        void uid_extract_disable();

    private:
        PCSCContext();
        ~PCSCContext();

        void thread();
        void thread_join();

        void check_states_after_get_status_change(SCARDCONTEXT context,
                                                  std::vector<SCARD_READERSTATE> &states);
        void card_present(SCARDCONTEXT context, const SCARD_READERSTATE &state);

        std::thread thread_;

        std::atomic<SCARDCONTEXT> context_{0};

        struct
        {
            std::mutex mutex;
            bool stop = false;
            bool cancellable = false;
            std::condition_variable cancelled_condition;
        } run_status_;

        std::atomic<bool> uid_extract_{false};
        UIDQueue uid_queue_;
    };
}

#endif // UIM_DAEMON_PCSC_CONTEXT_H
