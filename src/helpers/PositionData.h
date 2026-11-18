#pragma once

#include <stdint.h>

#define POSITION_FLAG_FIX_VALID   0x01
#define POSITION_FLAG_ALT_VALID   0x02
#define POSITION_DATA_SIZE        22

// Earliest plausible GPS epoch: 2020-01-01 00:00:00 UTC.
#define POSITION_MIN_VALID_EPOCH  1577836800u

struct PositionData {
  uint32_t tag;
  uint8_t  flags;
  int32_t  lat;          // degrees * 1e6
  int32_t  lon;          // degrees * 1e6
  int32_t  alt_mm;       // millimetres
  uint8_t  satellites;
  uint32_t gps_timestamp;
};

static inline void positionWriteLE32(uint8_t* dest, uint32_t value) {
  dest[0] = (uint8_t)(value & 0xFF);
  dest[1] = (uint8_t)((value >> 8) & 0xFF);
  dest[2] = (uint8_t)((value >> 16) & 0xFF);
  dest[3] = (uint8_t)((value >> 24) & 0xFF);
}

static inline uint32_t positionReadLE32(const uint8_t* src) {
  return (uint32_t)src[0]
         | ((uint32_t)src[1] << 8)
         | ((uint32_t)src[2] << 16)
         | ((uint32_t)src[3] << 24);
}

// Writes the 22-byte wire representation. Returns POSITION_DATA_SIZE.
static inline uint8_t encodePositionData(uint8_t* dest, const PositionData& pd) {
  uint8_t i = 0;
  positionWriteLE32(&dest[i], pd.tag); i += 4;
  dest[i++] = pd.flags;
  positionWriteLE32(&dest[i], (uint32_t)pd.lat); i += 4;
  positionWriteLE32(&dest[i], (uint32_t)pd.lon); i += 4;
  positionWriteLE32(&dest[i], (uint32_t)pd.alt_mm); i += 4;
  dest[i++] = pd.satellites;
  positionWriteLE32(&dest[i], pd.gps_timestamp); i += 4;
  return i;
}

// Returns false when len < POSITION_DATA_SIZE; trailing bytes are ignored.
static inline bool decodePositionData(const uint8_t* src, uint8_t len, PositionData& pd) {
  if (len < POSITION_DATA_SIZE) return false;

  uint8_t i = 0;
  pd.tag = positionReadLE32(&src[i]); i += 4;
  pd.flags = src[i++];
  pd.lat = (int32_t)positionReadLE32(&src[i]); i += 4;
  pd.lon = (int32_t)positionReadLE32(&src[i]); i += 4;
  pd.alt_mm = (int32_t)positionReadLE32(&src[i]); i += 4;
  pd.satellites = src[i++];
  pd.gps_timestamp = positionReadLE32(&src[i]); i += 4;
  return true;
}

// Apply the fix and privacy gates. The tag is always preserved so the
// requester can match the response even when there is no position.
static inline PositionData buildPositionData(uint32_t tag, bool location_active, bool fix_valid,
                                             int32_t lat, int32_t lon, int32_t alt_mm,
                                             int32_t satellites, int64_t gps_timestamp) {
  PositionData pd = {};
  pd.tag = tag;
  if (!location_active || !fix_valid) return pd;

  pd.flags = POSITION_FLAG_FIX_VALID | POSITION_FLAG_ALT_VALID;
  pd.lat = lat;
  pd.lon = lon;
  pd.alt_mm = alt_mm;

  if (satellites < 0) {
    pd.satellites = 0;
  } else if (satellites > 255) {
    pd.satellites = 255;
  } else {
    pd.satellites = (uint8_t)satellites;
  }

  if (gps_timestamp >= (int64_t)POSITION_MIN_VALID_EPOCH
      && gps_timestamp <= (int64_t)UINT32_MAX) {
    pd.gps_timestamp = (uint32_t)gps_timestamp;
  }

  return pd;
}
