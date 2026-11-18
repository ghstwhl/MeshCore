#include <gtest/gtest.h>
#include <climits>
#include <cstring>
#include "helpers/PositionData.h"

TEST(PositionData, EncodeDecodeRoundTrip) {
  PositionData in = {0x11223344u, POSITION_FLAG_FIX_VALID | POSITION_FLAG_ALT_VALID,
                     41234567, -71234567, 123456, 12, 1700000000u};
  uint8_t buf[POSITION_DATA_SIZE];
  ASSERT_EQ(encodePositionData(buf, in), POSITION_DATA_SIZE);

  PositionData out = {};
  ASSERT_TRUE(decodePositionData(buf, POSITION_DATA_SIZE, out));
  EXPECT_EQ(out.tag, in.tag);
  EXPECT_EQ(out.flags, in.flags);
  EXPECT_EQ(out.lat, in.lat);
  EXPECT_EQ(out.lon, in.lon);
  EXPECT_EQ(out.alt_mm, in.alt_mm);
  EXPECT_EQ(out.satellites, in.satellites);
  EXPECT_EQ(out.gps_timestamp, in.gps_timestamp);
}

TEST(PositionData, EncodeWritesLittleEndianLayout) {
  PositionData in = {0x11223344u, 0x03, 1, -2, -3, 4, 0x01020304u};
  uint8_t buf[POSITION_DATA_SIZE];
  ASSERT_EQ(encodePositionData(buf, in), POSITION_DATA_SIZE);

  const uint8_t expected[POSITION_DATA_SIZE] = {
      0x44, 0x33, 0x22, 0x11,  // tag
      0x03,                    // flags
      0x01, 0x00, 0x00, 0x00,  // lat
      0xFE, 0xFF, 0xFF, 0xFF,  // lon
      0xFD, 0xFF, 0xFF, 0xFF,  // alt_mm
      0x04,                    // satellites
      0x04, 0x03, 0x02, 0x01   // gps_timestamp
  };
  EXPECT_EQ(0, memcmp(buf, expected, sizeof(expected)));
}

TEST(PositionData, EncodeDecodeInt32Extremes) {
  PositionData in = {0xFFFFFFFFu, 0x03, INT32_MIN, INT32_MAX, INT32_MIN, 255, UINT32_MAX};
  uint8_t buf[POSITION_DATA_SIZE];
  ASSERT_EQ(encodePositionData(buf, in), POSITION_DATA_SIZE);

  PositionData out = {};
  ASSERT_TRUE(decodePositionData(buf, POSITION_DATA_SIZE, out));
  EXPECT_EQ(out.lat, INT32_MIN);
  EXPECT_EQ(out.lon, INT32_MAX);
  EXPECT_EQ(out.alt_mm, INT32_MIN);
  EXPECT_EQ(out.gps_timestamp, UINT32_MAX);
}

TEST(PositionData, DecodeRejectsShortBuffer) {
  uint8_t buf[POSITION_DATA_SIZE] = {};
  PositionData out = {};
  EXPECT_FALSE(decodePositionData(buf, POSITION_DATA_SIZE - 1, out));
}

TEST(PositionData, DecodeIgnoresTrailingBytes) {
  PositionData in = {0x01020304u, 0x03, 10, 20, 30, 5, 1700000000u};
  uint8_t buf[POSITION_DATA_SIZE + 1];
  ASSERT_EQ(encodePositionData(buf, in), POSITION_DATA_SIZE);
  buf[POSITION_DATA_SIZE] = 0xAA;

  PositionData out = {};
  ASSERT_TRUE(decodePositionData(buf, sizeof(buf), out));
  EXPECT_EQ(out.tag, in.tag);
  EXPECT_EQ(out.lat, in.lat);
  EXPECT_EQ(out.gps_timestamp, in.gps_timestamp);
}

TEST(PositionData, BuildPositionData_GpsInactive_ReturnsZeroPosition) {
  PositionData pd = buildPositionData(0x1234u, false, true, 10, 20, 30, 8, 1700000000);
  EXPECT_EQ(pd.tag, 0x1234u);
  EXPECT_EQ(pd.flags, 0);
  EXPECT_EQ(pd.lat, 0);
  EXPECT_EQ(pd.lon, 0);
  EXPECT_EQ(pd.alt_mm, 0);
  EXPECT_EQ(pd.satellites, 0);
  EXPECT_EQ(pd.gps_timestamp, 0u);
}

TEST(PositionData, BuildPositionData_NoFix_ReturnsZeroPosition) {
  PositionData pd = buildPositionData(0x1234u, true, false, 10, 20, 30, 8, 1700000000);
  EXPECT_EQ(pd.flags, 0);
  EXPECT_EQ(pd.lat, 0);
  EXPECT_EQ(pd.gps_timestamp, 0u);
}

TEST(PositionData, BuildPositionData_ValidFix_PopulatesFields) {
  PositionData pd = buildPositionData(0x1234u, true, true, 41234567, -71234567, 123456, 12,
                                      1700000000);
  EXPECT_EQ(pd.tag, 0x1234u);
  EXPECT_EQ(pd.flags, POSITION_FLAG_FIX_VALID | POSITION_FLAG_ALT_VALID);
  EXPECT_EQ(pd.lat, 41234567);
  EXPECT_EQ(pd.lon, -71234567);
  EXPECT_EQ(pd.alt_mm, 123456);
  EXPECT_EQ(pd.satellites, 12);
  EXPECT_EQ(pd.gps_timestamp, 1700000000u);
}

TEST(PositionData, BuildPositionData_TimestampSanity) {
  EXPECT_EQ(buildPositionData(1, true, true, 0, 0, 0, 1, 12345).gps_timestamp, 0u);
  EXPECT_EQ(buildPositionData(1, true, true, 0, 0, 0, 1, 1577836799).gps_timestamp, 0u);
  EXPECT_EQ(buildPositionData(1, true, true, 0, 0, 0, 1, 1577836800).gps_timestamp, 1577836800u);
  EXPECT_EQ(buildPositionData(1, true, true, 0, 0, 0, 1,
                              (int64_t)UINT32_MAX + 1).gps_timestamp, 0u);
}

TEST(PositionData, BuildPositionData_ClampsSatellites) {
  EXPECT_EQ(buildPositionData(1, true, true, 0, 0, 0, 300, 1700000000).satellites, 255);
  EXPECT_EQ(buildPositionData(1, true, true, 0, 0, 0, -1, 1700000000).satellites, 0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
