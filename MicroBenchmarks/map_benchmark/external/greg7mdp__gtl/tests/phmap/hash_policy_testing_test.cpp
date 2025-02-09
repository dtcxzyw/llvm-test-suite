// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtl/phmap.hpp"
#include "hash_policy_testing.hpp"

#include "gtest/gtest.h"

namespace gtl {
namespace priv {
namespace {

TEST(_, Hash) {
  StatefulTestingHash h1;
  EXPECT_EQ(1u, h1.id());
  StatefulTestingHash h2;
  EXPECT_EQ(2u, h2.id());
  StatefulTestingHash h1c(h1);
  EXPECT_EQ(1u, h1c.id());
  StatefulTestingHash h2m(std::move(h2));
  EXPECT_EQ(2u, h2m.id());
  EXPECT_EQ(0u, h2.id());
  StatefulTestingHash h3;
  EXPECT_EQ(3u, h3.id());
  h3 = StatefulTestingHash();
  EXPECT_EQ(4u, h3.id());
  h3 = std::move(h1);
  EXPECT_EQ(1u, h3.id());
}

}  // namespace
}  // namespace priv
}  // namespace gtl
