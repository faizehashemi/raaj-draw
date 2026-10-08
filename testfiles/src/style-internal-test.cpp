// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Tests for Style internal classes
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2020 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include <gtest/gtest.h>
#include <src/style-internal.h>

TEST(StyleInternalTest, testSPIDashArrayInequality)
{
	SPIDashArray array;
	array.read("0 1 2 3");
	SPIDashArray subsetArray;
	subsetArray.read("0 1");
	
	ASSERT_FALSE(array == subsetArray);
	ASSERT_FALSE(subsetArray == array);
}

TEST(StyleInternalTest, testSPIDashArrayEquality)
{
	SPIDashArray anArray;
	anArray.read("0 1 2 3");
	SPIDashArray sameArray;
	sameArray.read("0 1 2 3");
	
	ASSERT_TRUE(anArray == sameArray);
	ASSERT_TRUE(sameArray == anArray);
}

TEST(StyleInternalTest, testSPIDashArrayValidity)
{
    // valid dash arrays
	SPIDashArray array10;
	array10.read("");

	SPIDashArray array11;
	array11.read("0");

	SPIDashArray array12;
	array12.read("0 1e3");

    // invalid dash arrayas
	SPIDashArray array20;
	array20.read("1-1");

	SPIDashArray array21;
	array21.read("10 10 -10");

	SPIDashArray array22;
	array22.read("-1");

	SPIDashArray array23;
	array23.read("0 -5e3");


    EXPECT_TRUE(array10.is_valid());
    EXPECT_TRUE(array11.is_valid());
    EXPECT_TRUE(array12.is_valid());

    // SPIDashArray::read is geared towards happy path, so it may reject negative entries:

    // EXPECT_FALSE(array20.is_valid()); // cannot read "1-1" as numbers, so 0
    EXPECT_FALSE(array21.is_valid());
    // EXPECT_FALSE(array22.is_valid()); // lone negative number is deemed invalid and removed by 'read'
    // EXPECT_FALSE(array23.is_valid()); // negative total: invalid and removed by 'read'
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
