// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/id_sources/smart_card_id_source.h"

#include <string>

#include "daemon/pcsc_context.h"
#include "daemon/string.h"

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        constexpr char SMART_CARD_SOURCE_NAME[] = "SCARD";
    }

    SmartCardIdSource::SmartCardIdSource() : IdSource(SMART_CARD_SOURCE_NAME)
    {
    }

    void SmartCardIdSource::enable()
    {
        PCSCContext::instance().uid_extract_enable(
            [&](auto &extracted_uid) { uid_extracted(extracted_uid); });

        set_enabled(true);
    }

    void SmartCardIdSource::disable()
    {
        PCSCContext::instance().uid_extract_disable();

        set_enabled(false);
    }

    void SmartCardIdSource::uid_extracted(const PCSCContext::ExtractedUID &extracted_uid) const
    {
        IdentifiedUser identified_user;

        identified_user.user_identification_id =
            std::string(SMART_CARD_SOURCE_NAME) + "-" + string_hex_data(extracted_uid.uid);

        // TODO: Make mapping to seat id dependant on extracted_uid.reader_name? If mapping is
        //       stored in Configuration, it probably should be, then Daemon::apply_config() needs
        //       to propagate configuration to IdSources. Perhaps replace call to
        //       IdSource::Group::enable() in Daemon::apply_config() with
        //       IdSource::Group::apply_config() and then propagate configuration to all sources in
        //       that method. Want to minimize logic in Daemon. Should not know about different
        //       sources etc. at that level.
        identified_user.seat_id = SEAT_ID_MAIN_USER;

        user_identified(identified_user);
    }
}
