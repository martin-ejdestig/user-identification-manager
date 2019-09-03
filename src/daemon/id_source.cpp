// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/id_source.h"

#include <glib.h>
#include <glibmm.h>

#include <utility>

#include "config.h"
#if UIM_CONFIG_MASS_STORAGE_DEVICE_ID_SOURCE
#    include "daemon/id_sources/mass_storage_device_id_source.h"
#endif
#if UIM_CONFIG_SMART_CARD_ID_SOURCE
#    include "daemon/id_sources/smart_card_id_source.h"
#endif

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        IdSource::Group::Sources create_sources()
        {
            IdSource::Group::Sources sources;

#if UIM_CONFIG_MASS_STORAGE_DEVICE_ID_SOURCE
            sources.emplace_back(std::make_unique<MassStorageDeviceIdSource>());
#endif

#if UIM_CONFIG_SMART_CARD_ID_SOURCE
            sources.emplace_back(std::make_unique<SmartCardIdSource>());
#endif

            return sources;
        }
    }

    IdSource::IdSource(const std::string &name) : name_(name)
    {
    }

    IdSource::~IdSource() = default;

    void IdSource::user_identified(const IdentifiedUser &identified_user) const
    {
        if (listener_) {
            listener_->user_identified(identified_user);
        }
    }

    IdSource::Group::Group() : Group(create_sources())
    {
    }

    IdSource::Group::Group(Sources &&sources) : sources_(std::move(sources))
    {
        for (auto &source : sources_) {
            source->set_listener(this);
        }
    }

    std::vector<std::string> IdSource::Group::enabled_names() const
    {
        std::vector<std::string> names;

        for (auto &source : sources_) {
            if (source->enabled()) {
                names.emplace_back(source->name());
            }
        }

        return names;
    }

    std::vector<std::string> IdSource::Group::disabled_names() const
    {
        std::vector<std::string> names;

        for (auto &source : sources_) {
            if (!source->enabled()) {
                names.emplace_back(source->name());
            }
        }

        return names;
    }

    void IdSource::Group::enable_all() const
    {
        for (auto &source : sources_) {
            source->enable();
        }
    }

    void IdSource::Group::disable_all() const
    {
        for (auto &source : sources_) {
            source->disable();
        }
    }

    void IdSource::Group::enable(const std::vector<std::string> &names) const
    {
        if (names.empty()) {
            enable_all();
            return;
        }

        disable_all();

        for (const std::string &name : names) {
            IdSource *source = find_source(name);

            if (source) {
                source->enable();
            } else {
                g_warning("Can not enable \"%s\", unknown source", name.c_str());
            }
        }
    }

    std::vector<IdSource::IdentifiedUser> IdSource::Group::identified_users() const
    {
        return {identified_users_.cbegin(), identified_users_.cend()};
    }

    void IdSource::Group::user_identified(const IdentifiedUser &identified_user)
    {
        g_message("User identified, user identification id: %s, seat id: 0x%04x",
                  identified_user.user_identification_id.c_str(),
                  identified_user.seat_id);

        if (identified_users_.size() >= MAX_SAVED_IDENTIFIED_USERS) {
            identified_users_.pop_front();
        }

        identified_users_.push_back(identified_user);

        user_identified_signal_.emit(identified_user);
    }

    IdSource *IdSource::Group::find_source(const std::string &name) const
    {
        for (auto &source : sources_) {
            if (g_ascii_strcasecmp(source->name().c_str(), name.c_str()) == 0) {
                return source.get();
            }
        }
        return nullptr;
    }
}
