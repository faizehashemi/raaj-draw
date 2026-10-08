// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief Unit tests for the Preferences object
 *//*
 * Authors:
 * see git history
 *
 * Copyright (C) 2016-2024 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <gtest/gtest.h>

#include "preferences.h"

// test observer
class TestObserver : public Inkscape::Preferences::Observer {
public:
    TestObserver(Glib::ustring const &path) :
        Inkscape::Preferences::Observer(path),
        value(0) {}

    virtual void notify(Inkscape::Preferences::Entry const &val)
    {
        value = val.getInt();
    }
    int value;
};

class PreferencesTest : public ::testing::Test
{
protected:
    void SetUp() override {
        prefs = Inkscape::Preferences::get();
    }
    void TearDown() override {
        prefs = NULL;
        Inkscape::Preferences::unload();
    }
public:
    Inkscape::Preferences *prefs;
};

TEST_F(PreferencesTest, testStartingState)
{
    ASSERT_TRUE(prefs);
    ASSERT_EQ(prefs->isWritable(), true);
}

TEST_F(PreferencesTest, testOverwrite)
{
    prefs->setInt("/test/intvalue", 123);
    prefs->setInt("/test/intvalue", 321);
    ASSERT_EQ(prefs->getInt("/test/intvalue"), 321);
}

TEST_F(PreferencesTest, testDefaultReturn)
{
    ASSERT_EQ(prefs->getInt("/this/path/does/not/exist", 123), 123);
}

TEST_F(PreferencesTest, testLimitedReturn)
{
    prefs->setInt("/test/intvalue", 1000);

    // simple case
    ASSERT_EQ(prefs->getIntLimited("/test/intvalue", 123, 0, 500), 123);
    // the below may seem quirky but this behaviour is intended
    ASSERT_EQ(prefs->getIntLimited("/test/intvalue", 123, 1001, 5000), 123);
    // corner cases
    ASSERT_EQ(prefs->getIntLimited("/test/intvalue", 123, 0, 1000), 1000);
    ASSERT_EQ(prefs->getIntLimited("/test/intvalue", 123, 1000, 5000), 1000);
}

TEST_F(PreferencesTest, testKeyObserverNotification)
{
    Glib::ustring const path = "/some/random/path";
    TestObserver obs("/some/random");
    obs.value = 1;
    prefs->setInt(path, 5);
    ASSERT_EQ(obs.value, 1); // no notifications sent before adding

    prefs->addObserver(obs);
    prefs->setInt(path, 10);
    ASSERT_EQ(obs.value, 10);
    prefs->setInt("/some/other/random/path", 10);
    ASSERT_EQ(obs.value, 10); // value should not change

    prefs->removeObserver(obs);
    prefs->setInt(path, 15);
    ASSERT_EQ(obs.value, 10); // no notifications sent after removal
}

TEST_F(PreferencesTest, testEntryObserverNotification)
{
    Glib::ustring const path = "/some/random/path";
    TestObserver obs(path);
    obs.value = 1;
    prefs->setInt(path, 5);
    ASSERT_EQ(obs.value, 1); // no notifications sent before adding

    prefs->addObserver(obs);
    prefs->setInt(path, 10);
    ASSERT_EQ(obs.value, 10);

    // test that filtering works properly
    prefs->setInt("/some/random/value", 1234);
    ASSERT_EQ(obs.value, 10);
    prefs->setInt("/some/randomvalue", 1234);
    ASSERT_EQ(obs.value, 10);
    prefs->setInt("/some/random/path2", 1234);
    ASSERT_EQ(obs.value, 10);

    prefs->removeObserver(obs);
    prefs->setInt(path, 15);
    ASSERT_EQ(obs.value, 10); // no notifications sent after removal
}

TEST_F(PreferencesTest, testPreferencesEntryMethods)
{
    prefs->setInt("/test/prefentry", 100);
    Inkscape::Preferences::Entry val = prefs->getEntry("/test/prefentry");
    ASSERT_TRUE(val.isValid());
    ASSERT_EQ(val.getPath(), "/test/prefentry");
    ASSERT_EQ(val.getEntryName(), "prefentry");
    ASSERT_EQ(val.getInt(), 100);
}

TEST_F(PreferencesTest, testTemporaryPreferences)
{
    auto pref = "/test/prefentry";
    prefs->setInt(pref, 100);
    ASSERT_EQ(prefs->getInt(pref), 100);
    {
        auto transaction = prefs->temporaryPreferences();
        prefs->setInt(pref, 200);
        ASSERT_EQ(prefs->getInt(pref), 200);
        {
            auto sub_transaction = prefs->temporaryPreferences();
            prefs->setInt(pref, 300);
            ASSERT_EQ(prefs->getInt(pref), 300);
        }
        // This doesn't change because only one guard can exist in the stack at one time.
        ASSERT_EQ(prefs->getInt(pref), 300);
    }
    ASSERT_EQ(prefs->getInt(pref), 100);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
