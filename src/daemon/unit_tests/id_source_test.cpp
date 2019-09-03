// Copyright (C) 2019 Luxoft Sweden AB
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0

#include "daemon/id_source.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

#include "common/scoped_silent_log_handler.h"

namespace UserIdentificationManager::Daemon
{
    namespace
    {
        class TestSource : public IdSource
        {
        public:
            explicit TestSource(unsigned int i) : IdSource("TEST" + std::to_string(i))
            {
            }

            void enable() override
            {
                set_enabled(true);
            }

            void disable() override
            {
                set_enabled(false);
            }

            IdentifiedUser simulate_user_identified(const std::string &user_identification_id,
                                                    IdSource::SeatId seat_id) const
            {
                IdentifiedUser user;

                user.user_identification_id = name() + "-" + user_identification_id;
                user.seat_id = seat_id;

                user_identified(user);

                return user;
            }
        };

        class IdSourceGroupTest : public testing::Test
        {
        public:
            using Names = std::vector<std::string>;

            static constexpr unsigned int NUM_TEST_SOURCES = 3;

            IdSourceGroupTest() : group_(create_test_sources())
            {
            }

            TestSource &test_source(unsigned int num) const
            {
                return *sources_.at(num);
            }

            IdSource::Group &group()
            {
                return group_;
            }

        private:
            IdSource::Group::Sources create_test_sources()
            {
                IdSource::Group::Sources sources;

                for (unsigned int i = 0; i < NUM_TEST_SOURCES; i++) {
                    auto source = std::make_unique<TestSource>(i + 1);
                    sources_.emplace_back(source.get());
                    sources.emplace_back(std::move(source));
                }

                return sources;
            }

            Common::ScopedSilentLogHandler log_handler_;
            std::vector<TestSource *> sources_;
            IdSource::Group group_;
        };

        testing::AssertionResult expect_user(const char *user1_expr,
                                             const char *user2_expr,
                                             const IdSource::IdentifiedUser &user1,
                                             const IdSource::IdentifiedUser &user2)
        {
            if (user1.user_identification_id != user2.user_identification_id) {
                return testing::AssertionFailure()
                       << user1_expr << ".user_identification_id (\""
                       << user1.user_identification_id << "\") != " << user2_expr
                       << ".user_identification_id (\"" << user2.user_identification_id << "\")";
            }

            if (user1.seat_id != user2.seat_id) {
                return testing::AssertionFailure()
                       << user1_expr << ".seat_id (" << user1.seat_id << ") != " << user2_expr
                       << ".seat_id (" << user2.seat_id << ")";
            }

            return testing::AssertionSuccess();
        }

        testing::AssertionResult expect_users(const char *users1_expr,
                                              const char *users2_expr,
                                              const std::vector<IdSource::IdentifiedUser> &users1,
                                              const std::vector<IdSource::IdentifiedUser> &users2)
        {
            if (users1.size() != users2.size()) {
                return testing::AssertionFailure()
                       << users1_expr << ".size() (" << users1.size() << ") != " << users2_expr
                       << ".size() (" << users2.size() << ")";
            }

            for (std::size_t i = 0; i < users1.size(); i++) {
                testing::AssertionResult result =
                    expect_user((std::string(users1_expr) + "[" + std::to_string(i) + "]").c_str(),
                                (std::string(users2_expr) + "[" + std::to_string(i) + "]").c_str(),
                                users1[i],
                                users2[i]);

                if (!result) {
                    return result;
                }
            }

            return testing::AssertionSuccess();
        }
    }

    TEST_F(IdSourceGroupTest, NoneEnabledByDefault)
    {
        EXPECT_TRUE(group().enabled_names().empty());
        EXPECT_EQ(NUM_TEST_SOURCES, group().disabled_names().size());
    }

    TEST_F(IdSourceGroupTest, EnableAll)
    {
        group().enable_all();
        EXPECT_EQ(NUM_TEST_SOURCES, group().enabled_names().size());
        EXPECT_TRUE(group().disabled_names().empty());
    }

    TEST_F(IdSourceGroupTest, DisableAll)
    {
        group().enable_all();
        EXPECT_EQ(NUM_TEST_SOURCES, group().enabled_names().size());

        group().disable_all();
        EXPECT_TRUE(group().enabled_names().empty());
        EXPECT_EQ(NUM_TEST_SOURCES, group().disabled_names().size());
    }

    TEST_F(IdSourceGroupTest, EnableWithEmptyNamesArgumentEnablesAll)
    {
        group().enable({});
        EXPECT_EQ(NUM_TEST_SOURCES, group().enabled_names().size());
        EXPECT_TRUE(group().disabled_names().empty());
    }

    TEST_F(IdSourceGroupTest, EnableSpecificSources)
    {
        group().enable({"TEST1"});
        EXPECT_EQ(Names({"TEST1"}), group().enabled_names());
        EXPECT_EQ(Names({"TEST2", "TEST3"}), group().disabled_names());

        group().enable({"TEST1", "TEST3"});
        EXPECT_EQ(Names({"TEST1", "TEST3"}), group().enabled_names());
        EXPECT_EQ(Names({"TEST2"}), group().disabled_names());

        group().enable({"TEST2"});
        EXPECT_EQ(Names({"TEST2"}), group().enabled_names());
        EXPECT_EQ(Names({"TEST1", "TEST3"}), group().disabled_names());

        group().enable({"TEST3"});
        EXPECT_EQ(Names({"TEST3"}), group().enabled_names());
        EXPECT_EQ(Names({"TEST1", "TEST2"}), group().disabled_names());

        group().enable({"TEST1", "TEST2", "TEST3"});
        EXPECT_EQ(Names({"TEST1", "TEST2", "TEST3"}), group().enabled_names());
        EXPECT_TRUE(group().disabled_names().empty());
    }

    TEST_F(IdSourceGroupTest, EnableIsCaseInsensitive)
    {
        group().enable({"test1"});
        EXPECT_EQ(Names({"TEST1"}), group().enabled_names());

        group().enable({"TeSt2", "tEsT3"});
        EXPECT_EQ(Names({"TEST2", "TEST3"}), group().enabled_names());
        EXPECT_EQ(Names({"TEST1"}), group().disabled_names());
    }

    TEST_F(IdSourceGroupTest, EnableUnknownNamesAreIgnored)
    {
        group().enable({"UNKNOWN"});
        EXPECT_TRUE(group().enabled_names().empty());

        group().enable({"UNKNOWN1", "TEST1", "UNKNOWN2"});
        EXPECT_EQ(Names({"TEST1"}), group().enabled_names());

        group().enable({"UNKNOWN1", "TEST1", "UNKNOWN2", "TEST3"});
        EXPECT_EQ(Names({"TEST1", "TEST3"}), group().enabled_names());
    }

    TEST_F(IdSourceGroupTest, UserIdentifiedSignal)
    {
        IdSource::IdentifiedUser received_user;
        IdSource::IdentifiedUser expected_user;

        group().enable_all();
        group().user_identified_signal().connect([&](const auto &user) { received_user = user; });

        expected_user = test_source(0).simulate_user_identified("123", 0x0123);
        EXPECT_PRED_FORMAT2(expect_user, expected_user, received_user);

        expected_user = test_source(1).simulate_user_identified("456", 0x4567);
        EXPECT_PRED_FORMAT2(expect_user, expected_user, received_user);

        expected_user = test_source(2).simulate_user_identified("789", 0x89ab);
        EXPECT_PRED_FORMAT2(expect_user, expected_user, received_user);
    }

    TEST_F(IdSourceGroupTest, IdentifiedUsers)
    {
        std::vector<IdSource::IdentifiedUser> expected_users;

        group().enable_all();

        expected_users.emplace_back(test_source(0).simulate_user_identified("123", 0x0123));
        EXPECT_PRED_FORMAT2(expect_users, expected_users, group().identified_users());

        expected_users.emplace_back(test_source(1).simulate_user_identified("456", 0x4567));
        EXPECT_PRED_FORMAT2(expect_users, expected_users, group().identified_users());
    }

    TEST_F(IdSourceGroupTest, IdentifiedUsersMoreThanMaxSavedDiscardsOld)
    {
        std::vector<IdSource::IdentifiedUser> expected_users;

        group().enable_all();

        for (unsigned int i = 0; i < IdSource::Group::MAX_SAVED_IDENTIFIED_USERS; i++) {
            TestSource &source = test_source(i % NUM_TEST_SOURCES);

            expected_users.emplace_back(
                source.simulate_user_identified(std::to_string(i), IdSource::SeatId(i)));
        }

        EXPECT_EQ(IdSource::Group::MAX_SAVED_IDENTIFIED_USERS, group().identified_users().size());
        EXPECT_PRED_FORMAT2(expect_users, expected_users, group().identified_users());

        expected_users.emplace_back(test_source(0).simulate_user_identified("123", 0x0123));
        expected_users.erase(expected_users.begin());
        EXPECT_EQ(IdSource::Group::MAX_SAVED_IDENTIFIED_USERS, group().identified_users().size());
        EXPECT_PRED_FORMAT2(expect_users, expected_users, group().identified_users());

        expected_users.emplace_back(test_source(1).simulate_user_identified("456", 0x4567));
        expected_users.erase(expected_users.begin());
        EXPECT_EQ(IdSource::Group::MAX_SAVED_IDENTIFIED_USERS, group().identified_users().size());
        EXPECT_PRED_FORMAT2(expect_users, expected_users, group().identified_users());
    }
}
