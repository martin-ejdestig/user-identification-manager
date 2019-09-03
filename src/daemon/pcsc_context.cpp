// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/pcsc_context.h"

#include <glibmm.h>
#include <winscard.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        class Card
        {
        public:
            ~Card()
            {
                disconnect();
            }

            bool connect(SCARDCONTEXT context, const char *reader_name)
            {
                disconnect();

                SCARDHANDLE handle = 0;
                DWORD active_protocol = SCARD_PROTOCOL_UNDEFINED;
                LONG ret = SCardConnect(context,
                                        reader_name,
                                        SCARD_SHARE_SHARED,
                                        SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                                        &handle,
                                        &active_protocol);

                if (ret != SCARD_S_SUCCESS) {
                    g_warning("SCardConnect() failed for reader \"%s\": %s",
                              reader_name,
                              pcsc_stringify_error(ret));
                    return false;
                }

                handle_ = handle;

                switch (active_protocol) {
                case SCARD_PROTOCOL_T0:
                    send_pci_ = SCARD_PCI_T0;
                    break;
                case SCARD_PROTOCOL_T1:
                    send_pci_ = SCARD_PCI_T1;
                    break;
                default:
                    g_warning("SCardConnect() returned unknown protocol (%lu) for reader \"%s\"",
                              active_protocol,
                              reader_name);
                    disconnect();
                    return false;
                }

                return true;
            }

            void disconnect()
            {
                if (!handle_) {
                    return;
                }

                LONG ret = SCardDisconnect(*handle_, SCARD_LEAVE_CARD);

                handle_.reset();

                if (ret != SCARD_S_SUCCESS) {
                    g_warning("SCardDisconnect() failed: %s", pcsc_stringify_error(ret));
                }
            }

            std::size_t transmit(const std::vector<std::uint8_t> &send_buffer,
                                 std::vector<std::uint8_t> &recv_buffer)
            {
                DWORD recv_length = recv_buffer.size();

                LONG ret = SCardTransmit(*handle_,
                                         send_pci_,
                                         send_buffer.data(),
                                         send_buffer.size(),
                                         nullptr,
                                         recv_buffer.data(),
                                         &recv_length);

                if (ret != SCARD_S_SUCCESS) {
                    g_warning("SCardTransmit() failed: %s", pcsc_stringify_error(ret));
                    return 0;
                }

                return recv_length;
            }

        private:
            std::optional<SCARDHANDLE> handle_;
            const SCARD_IO_REQUEST *send_pci_ = nullptr;
        };

        constexpr char NOTIFICATION_READER_NAME[] = R"(\\?PnP?\Notification)";
        constexpr unsigned int NOTIFICATION_STATE_INDEX = 0;

        std::vector<std::string> multi_string_split(const char *multi_string)
        {
            std::vector<std::string> strings;
            const char *pos = multi_string;

            while (*pos) {
                strings.emplace_back(pos);
                pos += strings.back().size() + 1;
            }

            return strings;
        }

        std::vector<std::string> list_readers(SCARDCONTEXT context)
        {
            // Querying buffer size and then filling buffer with two calls to SCardListReaders() is
            // racey in pcsc-lite. Each call updates reader list from service. Will most likely fail
            // if first call reports X readers and next call reports X+1 readers due to buffer being
            // too small. The X+1:th reader will never be noticed. Have seen this happen so use
            // SCARD_AUTOALLOCATE even though it requires ugly casting of buffer argument and is not
            // exception safe (if multi_string_split() OOM:s we want to terminate anyway).
            char *readers_multi_string;
            auto readers_multi_string_size = static_cast<DWORD>(SCARD_AUTOALLOCATE);
            LONG ret;

            ret = SCardListReaders(context,
                                   nullptr,
                                   // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                                   reinterpret_cast<LPSTR>(&readers_multi_string),
                                   &readers_multi_string_size);

            if (ret != SCARD_S_SUCCESS) {
                if (ret != SCARD_E_NO_READERS_AVAILABLE) {
                    g_warning("Failed to list PC/SC readers: %s", pcsc_stringify_error(ret));
                }

                return {};
            }

            std::vector<std::string> readers = multi_string_split(readers_multi_string);

            SCardFreeMemory(context, readers_multi_string);

            return readers;
        }

        std::vector<SCARD_READERSTATE> initial_states(const std::vector<std::string> &reader_names)
        {
            std::vector<SCARD_READERSTATE> reader_states(1 + reader_names.size());

            reader_states[NOTIFICATION_STATE_INDEX].szReader = NOTIFICATION_READER_NAME;
            reader_states[NOTIFICATION_STATE_INDEX].dwCurrentState = SCARD_STATE_UNAWARE;
            reader_states[NOTIFICATION_STATE_INDEX].dwEventState = SCARD_STATE_UNAWARE;

            for (std::size_t i = 0; i < reader_names.size(); i++) {
                reader_states[1 + i].szReader = reader_names[i].c_str();
                reader_states[1 + i].dwCurrentState = SCARD_STATE_UNAWARE;
                reader_states[1 + i].dwEventState = SCARD_STATE_UNAWARE;
            }

            return reader_states;
        }

        bool get_data_uid_supported(const SCARD_READERSTATE &state)
        {
            // TODO: Better parsing of ATR and find out if the "get data" command can be used to
            //       extract UID for other cards than contactless storage cards that are identified
            //       with this ATR. See:
            //       - "3.1.3.2.3.2  Contactless Storage Cards" in
            //         https://muscle.apdu.fr/www.pcscworkgroup.com/PCSC/V2/pcsc3_v2.01.09.pdf .
            //       - https://en.wikipedia.org/wiki/Answer_to_reset
            if (state.cbAtr > 5) {
                return state.rgbAtr[5] == 0x4f;
            }
            return false;
        }

        std::optional<std::vector<std::uint8_t>> transmit_get_data_uid(SCARDCONTEXT context,
                                                                       const char *reader_name)
        {
            // See "3.2.2.1.3  Get Data Command" in
            // https://muscle.apdu.fr/www.pcscworkgroup.com/PCSC/V2/pcsc3_v2.01.09.pdf
            constexpr unsigned int UID_MIN_LENGTH = 4;
            constexpr unsigned int UID_MAX_LENGTH = 10;
            constexpr unsigned int ERROR_CODE_LENGTH = 2;
            constexpr std::uint16_t ERROR_CODE_SUCCESS = 0x9000;
            constexpr unsigned int RECV_MIN_LENGTH = UID_MIN_LENGTH + ERROR_CODE_LENGTH;
            constexpr unsigned int RECV_MAX_LENGTH = UID_MAX_LENGTH + ERROR_CODE_LENGTH;
            const std::vector<std::uint8_t> get_data_command = {0xff, 0xca, 0x00, 0x00, 0x00};
            std::vector<std::uint8_t> recv_buffer(RECV_MAX_LENGTH);

            Card card;

            if (!card.connect(context, reader_name)) {
                return {};
            }

            std::size_t recv_length = card.transmit(get_data_command, recv_buffer);

            if (recv_length < RECV_MIN_LENGTH) {
                g_warning("Too few bytes received from reader \"%s\" for Get Data command, "
                          "received %zu bytes but expected at least %u bytes",
                          reader_name,
                          recv_length,
                          RECV_MIN_LENGTH);
                return {};
            }

            std::uint16_t error_code =
                recv_buffer[recv_length - 2] << 8 | recv_buffer[recv_length - 1];

            if (error_code != ERROR_CODE_SUCCESS) {
                g_warning("Received error 0x%04x from reader \"%s\" for Get Data command",
                          error_code,
                          reader_name);
                return {};
            }

            recv_buffer.resize(recv_length - 2);

            return recv_buffer;
        }
    }

    PCSCContext::PCSCContext()
    {
        thread_ = std::thread(&PCSCContext::thread, this);
    }

    PCSCContext::~PCSCContext()
    {
        thread_join();
    }

    PCSCContext &PCSCContext::instance()
    {
        static PCSCContext context;
        return context;
    }

    void PCSCContext::uid_extract_enable(UIDQueue::Callback &&callback)
    {
        uid_extract_ = true;
        uid_queue_.set_callback(std::move(callback));
    }

    void PCSCContext::uid_extract_disable()
    {
        uid_queue_.clear_callback();
        uid_extract_ = false;
    }

    void PCSCContext::thread()
    {
        LONG ret;
        SCARDCONTEXT context = 0;

        ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &context);
        if (ret != SCARD_S_SUCCESS) {
            g_warning("Failed to establish PC/SC context: %s", pcsc_stringify_error(ret));
            return;
        }

        context_ = context;

        std::vector<std::string> reader_names = list_readers(context);
        std::vector<SCARD_READERSTATE> states = initial_states(reader_names);

        while (true) {
            {
                std::unique_lock<std::mutex> lock(run_status_.mutex);
                if (run_status_.stop) {
                    break;
                }
                run_status_.cancellable = true;
            }

            ret = SCardGetStatusChange(context, INFINITE, states.data(), states.size());

            {
                std::unique_lock<std::mutex> lock(run_status_.mutex);
                run_status_.cancellable = false;
                if (run_status_.stop) {
                    run_status_.cancelled_condition.notify_one();
                    break;
                }
            }

            if (ret != SCARD_S_SUCCESS) {
                if (ret != SCARD_E_CANCELLED) {
                    g_warning("Failed to get PC/SC status change: %s", pcsc_stringify_error(ret));
                }
                continue;
            }

            check_states_after_get_status_change(context, states);

            if (states[NOTIFICATION_STATE_INDEX].dwEventState & SCARD_STATE_CHANGED) {
                // Looks like \\?PnP?\Notification can miss readers in pcsc-lite. Number of readers
                // are counted at start of SCardGetStatusChange() and \\?PnP?\Notification is only
                // set to "changed" if the number of readers change. If a reader is added between
                // SCardListReaders() and SCardGetStatusChange(), it will not be noticed. Sleeping a
                // bit makes it less likely that a reader is not detected in next SCardListReaders()
                // call. Have observed this behavior with the ACR1252 that exposes two readers. It
                // is very often both of them are not detected without this sleep.
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                reader_names = list_readers(context);
                states = initial_states(reader_names);
            }
        }

        ret = SCardReleaseContext(context);
        if (ret != SCARD_S_SUCCESS) {
            g_warning("Failed to release PC/SC context: %s", pcsc_stringify_error(ret));
        }
    }

    void PCSCContext::thread_join()
    {
        {
            std::unique_lock<std::mutex> lock(run_status_.mutex);

            run_status_.stop = true;

            // SCardCancel() may not be noticed so loop until it is. Condition to wake up
            // as quickly as possible when it is done and to retry if it is not. See
            // https://salsa.debian.org/rousseau/PCSC/issues/16 for upstream bug.
            while (run_status_.cancellable) {
                SCardCancel(context_);
                run_status_.cancelled_condition.wait_for(lock, std::chrono::milliseconds(50));
            }
        }

        thread_.join();
    }

    void PCSCContext::check_states_after_get_status_change(SCARDCONTEXT context,
                                                           std::vector<SCARD_READERSTATE> &states)
    {
        for (std::size_t i = 0; i < states.size(); i++) {
            if (i == NOTIFICATION_STATE_INDEX) {
                continue;
            }

            SCARD_READERSTATE &state = states[i];

            if (state.dwEventState & SCARD_STATE_CHANGED) {
                if ((state.dwCurrentState & SCARD_STATE_EMPTY) &&
                    (state.dwEventState & SCARD_STATE_PRESENT)) {
                    card_present(context, state);
                }
            }

            state.dwCurrentState = state.dwEventState;
        }
    }

    void PCSCContext::card_present(SCARDCONTEXT context, const SCARD_READERSTATE &state)
    {
        if (uid_extract_) {
            if (get_data_uid_supported(state)) {
                auto uid = transmit_get_data_uid(context, state.szReader);

                if (uid) {
                    uid_queue_.push(ExtractedUID{std::move(*uid), state.szReader});
                }
            } else {
                g_warning("Can not extract UID from card present at \"%s\"", state.szReader);
            }
        }
    }
}
