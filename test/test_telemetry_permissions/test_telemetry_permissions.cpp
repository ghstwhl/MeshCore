#include <gtest/gtest.h>
#include "helpers/TelemetryPermissions.h"

// Permission bits mirror TELEM_PERM_BASE (0x01), TELEM_PERM_LOCATION (0x02),
// TELEM_PERM_ENVIRONMENT (0x04) in src/helpers/SensorManager.h.
TEST(TelemetryPermissions, AllowAllGrantsRegardlessOfContactFlags) {
  EXPECT_EQ(resolveTelemetryPermission(true, false, 0x00, 0x02), 0x02);
  EXPECT_EQ(resolveTelemetryPermission(true, false, 0xFF, 0x04), 0x04);
}

TEST(TelemetryPermissions, AllowFlagsGrantsWhenContactBitSet) {
  EXPECT_EQ(resolveTelemetryPermission(false, true, 0x02, 0x02), 0x02);
  EXPECT_EQ(resolveTelemetryPermission(false, true, 0xFF, 0x04), 0x04);
}

TEST(TelemetryPermissions, AllowFlagsDeniesWhenContactBitClear) {
  EXPECT_EQ(resolveTelemetryPermission(false, true, 0x00, 0x02), 0);
  EXPECT_EQ(resolveTelemetryPermission(false, true, 0x01, 0x02), 0);
}

TEST(TelemetryPermissions, DenyReturnsZero) {
  EXPECT_EQ(resolveTelemetryPermission(false, false, 0xFF, 0x02), 0);
}

TEST(TelemetryPermissions, PermissionBitsAreIndependent) {
  EXPECT_EQ(resolveTelemetryPermission(false, true, 0x04, 0x02), 0);
  EXPECT_EQ(resolveTelemetryPermission(false, true, 0x04, 0x04), 0x04);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
