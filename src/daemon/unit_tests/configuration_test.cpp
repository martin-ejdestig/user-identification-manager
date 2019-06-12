// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/configuration.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "common/scoped_silent_log_handler.h"
#include "common/scoped_temp_file.h"

namespace UserIdentificationManager::Daemon
{
    TEST(Configuration, DefaultWithConfigFileSetIfFileDoesNotExist)
    {
        const std::string file_name = "/tmp/uim_config_does_not_exist.conf";
        Configuration config = Configuration::from_file(file_name);

        EXPECT_EQ(file_name, config.config_file);
        EXPECT_EQ(Configuration().sources_enable, config.sources_enable);
    }

    TEST(Configuration, DefaultWithConfigFileSetIfInvalidContent)
    {
        Common::ScopedSilentLogHandler log_handler;
        Common::ScopedTempFile file("invalid content");
        Configuration config = Configuration::from_file(file.path());

        EXPECT_EQ(file.path(), config.config_file);
        EXPECT_EQ(Configuration().sources_enable, config.sources_enable);
    }

    TEST(Configuration, SourcesEnableParsedCorrectly)
    {
        Common::ScopedTempFile file("[sources]\n"
                                    "enable=TEST1,TEST2");
        Configuration config = Configuration::from_file(file.path());

        EXPECT_EQ(std::vector<std::string>({"TEST1", "TEST2"}), config.sources_enable);
    }

    TEST(Configuration, SourcesEnableDefaultsToEmptyIfNotSet)
    {
        Common::ScopedTempFile file("[sources]\n"
                                    "something_else=TEST1,TEST2");
        Configuration config = Configuration::from_file(file.path());

        EXPECT_TRUE(config.sources_enable.empty());
    }

    TEST(Configuration, SourcesEnableEmptyAllowed)
    {
        Common::ScopedTempFile file("[sources]\n"
                                    "enable=");
        Configuration config = Configuration::from_file(file.path());

        EXPECT_TRUE(config.sources_enable.empty());
    }
}
