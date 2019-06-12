// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/id_sources/mass_storage_device_id_source.h"

#include <gtest/gtest.h>

#include "common/scoped_silent_log_handler.h"
#include "common/scoped_temp_file.h"

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        std::optional<IdSource::IdentifiedUser> parser_read_file(const std::string &file_content)
        {
            Common::ScopedTempFile file(file_content);
            Common::ScopedSilentLogHandler log_handler;

            return MassStorageDeviceIdSource::Parser::read_file(file.path());
        }
    }

    TEST(MassStorageDeviceIdSourceParser, ValidFile)
    {
        auto result = parser_read_file("ID 1234\n"
                                       "SEAT 0x5678");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ("MSD-1234", result->user_identification_id);
        EXPECT_EQ(0x5678, result->seat_id);
    }

    TEST(MassStorageDeviceIdSourceParser, ValidFileWithTrailingNewline)
    {
        auto result = parser_read_file("ID 4321\n"
                                       "SEAT 0x8765\n");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ("MSD-4321", result->user_identification_id);
        EXPECT_EQ(0x8765, result->seat_id);
    }

    TEST(MassStorageDeviceIdSourceParser, TrailingLinesIgnored)
    {
        auto result = parser_read_file("ID 1\n"
                                       "SEAT 0x2345\n"
                                       "Allows for e.g. trailing lines to contain\n"
                                       "other ids to copy paste to top");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ("MSD-1", result->user_identification_id);
        EXPECT_EQ(0x2345, result->seat_id);
    }

    TEST(MassStorageDeviceIdSourceParser, SeatIdZeroAllowed)
    {
        auto result = parser_read_file("ID 1234\n"
                                       "SEAT 0x0");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ("MSD-1234", result->user_identification_id);
        EXPECT_EQ(0x0, result->seat_id);
    }

    TEST(MassStorageDeviceIdSourceParser, SeatIdMaxAllowed)
    {
        auto result = parser_read_file("ID 1234\n"
                                       "SEAT 0xffff");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ("MSD-1234", result->user_identification_id);
        EXPECT_EQ(0xffff, result->seat_id);
    }

    TEST(MassStorageDeviceIdSourceParser, ToFewLines)
    {
        EXPECT_FALSE(parser_read_file("ID 1234 SEAT 0x5678").has_value());
    }

    TEST(MassStorageDeviceIdSourceParser, GarbageLinesAtStart)
    {
        EXPECT_FALSE(parser_read_file("\n"
                                      "ID 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("garbage\n"
                                      "ID 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("\n"
                                      "\n"
                                      "ID 1234\n"
                                      "SEAT 0x5678")
                         .has_value());
    }

    TEST(MassStorageDeviceIdSourceParser, InvalidIdPrefix)
    {
        EXPECT_FALSE(parser_read_file(" ID 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("aID 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("iD 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("id 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("IDb 1234\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID1234\n"
                                      "SEAT 0x5678")
                         .has_value());
    }

    TEST(MassStorageDeviceIdSourceParser, InvalidId)
    {
        EXPECT_FALSE(parser_read_file("ID \n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1a\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID a1\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1a2\n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 12 \n"
                                      "SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1 2\n"
                                      "SEAT 0x5678")
                         .has_value());
    }

    TEST(MassStorageDeviceIdSourceParser, InvalidSeatPrefix)
    {
        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      " SEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "aSEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "sEAT 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "seat 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEATa 0x5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT0x5678")
                         .has_value());
    }

    TEST(MassStorageDeviceIdSourceParser, InvalidSeat)
    {
        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 12")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 5678")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x10000")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x1ffff")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT -0x0")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT -0x1")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x56 78")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x5678 ")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x567z")
                         .has_value());

        EXPECT_FALSE(parser_read_file("ID 1234\n"
                                      "SEAT 0x567 z")
                         .has_value());
    }
}
