// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#ifndef UIM_DAEMON_ID_SOURCES_SMART_CARD_ID_SOURCE_H
#define UIM_DAEMON_ID_SOURCES_SMART_CARD_ID_SOURCE_H

#include "daemon/id_source.h"
#include "daemon/pcsc_context.h"

namespace UserIdentificationManager::Daemon
{
    // Identify user by reading information from smart card.
    //
    // Currently only extracts UID from contactless smart card and uses it as a user id.
    class SmartCardIdSource : public IdSource
    {
    public:
        SmartCardIdSource();

        void enable() override;
        void disable() override;

    private:
        void uid_extracted(const PCSCContext::ExtractedUID &extracted_uid) const;
    };
}

#endif // UIM_DAEMON_ID_SOURCES_SMART_CARD_ID_SOURCE_H
