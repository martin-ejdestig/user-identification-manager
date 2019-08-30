// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_ID_SOURCE_H
#define UIM_DAEMON_ID_SOURCE_H

#include <sigc++/sigc++.h>

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "config.h"

namespace UserIdentificationManager::Daemon
{
    // Identification source base class.
    //
    // The virtual methods enable() and disable() are called to enable/disable a source. Sources are
    // disabled by default.
    //
    // IdSource::Listener can be set for an IdSource to listen for users being identified. Note that
    // normally this interface should not be used directly. It is only exposed for making it easier
    // to write unit tests for sources. IdSource::Group is a listener and has a signal that is
    // emitted when any of its sources identifies a user. See below.
    //
    // IdSource::Group is the main interface through which the rest of the program should interact
    // with identification sources. It groups sources together and has methods for enabling and
    // disabling sources, getting names of enabled/disabled sources, a signal for listening for when
    // a user is identified by any of its sources and stores the most recent users identified.
    class IdSource
    {
    public:
        class Group;
        class Listener;
        struct IdentifiedUser;

        using SeatId = std::uint16_t;

        static constexpr SeatId SEAT_ID_MIN = 0;
        static constexpr SeatId SEAT_ID_MAX = 0xffff;
        static constexpr SeatId SEAT_ID_MAIN_USER = 0x0000;
        static constexpr SeatId SEAT_ID_UNDEFINED = 0xffff;

        explicit IdSource(const std::string &name);
        virtual ~IdSource();

        IdSource(const IdSource &other) = delete;
        IdSource(IdSource &&other) = delete;
        IdSource &operator=(const IdSource &other) = delete;
        IdSource &operator=(IdSource &&other) = delete;

        const std::string &name() const
        {
            return name_;
        }

        void set_listener(Listener *listener)
        {
            listener_ = listener;
        }

        bool enabled() const
        {
            return enabled_;
        }

        virtual void enable() = 0;
        virtual void disable() = 0;

    protected:
        void set_enabled(bool enabled)
        {
            enabled_ = enabled;
        }

        void user_identified(const IdentifiedUser &identified_user) const;

    private:
        std::string name_;
        Listener *listener_ = nullptr;
        bool enabled_ = false;
    };

    struct IdSource::IdentifiedUser
    {
        std::string user_identification_id;
        SeatId seat_id = SEAT_ID_UNDEFINED;
    };

    class IdSource::Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void user_identified(const IdentifiedUser &identified_user) = 0;
    };

    class IdSource::Group : public IdSource::Listener
    {
    public:
        using Sources = std::vector<std::unique_ptr<IdSource>>;

        static constexpr unsigned int MAX_SAVED_IDENTIFIED_USERS =
            UIM_CONFIG_DAEMON_MAX_SAVED_IDENTIFIED_USERS;

        Group();
        explicit Group(Sources &&sources);

        std::vector<std::string> enabled_names() const;
        std::vector<std::string> disabled_names() const;

        void enable_all() const;
        void disable_all() const;
        void enable(const std::vector<std::string> &names) const;

        sigc::signal<void, const IdentifiedUser &> &user_identified_signal()
        {
            return user_identified_signal_;
        }

        std::vector<IdentifiedUser> identified_users() const;

    private:
        void user_identified(const IdentifiedUser &identified_user) override;

        IdSource *find_source(const std::string &name) const;

        const Sources sources_;

        std::deque<IdentifiedUser> identified_users_;
        sigc::signal<void, const IdentifiedUser &> user_identified_signal_;
    };
}

#endif // UIM_DAEMON_ID_SOURCE_H
