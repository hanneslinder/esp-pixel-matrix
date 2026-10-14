#pragma once

#include "../types/CommonTypes.h"
#include <Arduino.h>

class MatrixController;

/**
 * StateStore - persists what the matrix is currently showing.
 *
 * Settings already survive a reboot through ConfigManager (/config.json). What
 * never did is the *content*: the background drawing, the text layout and the
 * clock visibility flag all lived only in RAM, so a power cut left a black
 * panel with default text until a browser pushed everything again.
 *
 * This stores that content in SPIFFS. Two slots (/state.a and /state.b) are
 * written alternately, and a slot holding the newest valid state is never the
 * one being written, so losing power mid-write can only damage the inactive
 * slot - the other one still restores. A CRC32 and a sequence number decide
 * which slot wins at boot.
 *
 * Writes are debounced: drawing produces a message every few tens of
 * milliseconds, and rewriting 4 KB of flash that often would wear the
 * partition out for no benefit.
 */
class StateStore {
  public:
  static StateStore& getInstance();

  // Number of text items the layout holds. Must match the textContent array in
  // main.cpp and the hardcoded bound used by the setText handler.
  static const size_t MAX_TEXT_ITEMS = 5;

  // How long the state has to stay unchanged before it is flushed, and the
  // longest a pending change may be held back while edits keep arriving.
  static const uint32_t SAVE_DEBOUNCE_MS = 5000;
  static const uint32_t SAVE_MAX_INTERVAL_MS = 60000;

  // Wires up the live objects to read from and restore into. Call before
  // begin(). SPIFFS must already be mounted - this class never mounts it.
  void init(MatrixController* matrix, TextItem* textItems, size_t textItemCount, bool* showText);

  // Restores the newest valid slot, if one exists. Returns true when state was
  // restored, false when there was nothing usable (first boot, both slots
  // corrupt, or SPIFFS unavailable).
  bool begin();

  // Records that the live state changed. Cheap; the write happens later from
  // update().
  void markDirty();

  // Call from loop(). Flushes a pending change once edits have settled, and at
  // least once per SAVE_MAX_INTERVAL_MS while they keep coming.
  void update();

  // Writes immediately, ignoring the debounce. Returns false on I/O failure.
  bool saveNow();

  bool hasStoredState() const { return _restoredSeq > 0; }
  uint32_t currentSequence() const { return _seq; }

  private:
  StateStore();
  StateStore(const StateStore&) = delete;
  StateStore& operator=(const StateStore&) = delete;

  // Reads one slot. Validates the header and the payload CRC. When `apply` is
  // set and the payload checks out, the staged data is copied into the live
  // objects.
  bool readSlot(uint8_t slot, bool apply, uint32_t* outSeq);

  // Writes the live objects into one slot.
  bool writeSlot(uint8_t slot, uint32_t seq);

  MatrixController* _matrix;
  TextItem* _textItems;
  size_t _textItemCount;
  bool* _showText;

  uint8_t _activeSlot; // slot holding the newest valid state
  uint32_t _seq;       // sequence number stored in that slot
  uint32_t _restoredSeq;
  bool _dirty;
  uint32_t _lastChange;
  uint32_t _lastAttempt;
  bool _loggedFailure;
};
