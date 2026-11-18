#pragma once

#include <stdint.h>

// Resolve one telemetry permission bit for a contact. Returns perm_bit when
// granted, otherwise 0. The caller ORs the results for base, location, and
// environment together.
static inline uint8_t resolveTelemetryPermission(bool allow_all, bool allow_flags,
                                                 uint8_t contact_perms, uint8_t perm_bit) {
  if (allow_all) return perm_bit;
  if (allow_flags && (contact_perms & perm_bit) != 0) return perm_bit;
  return 0;
}
