#include "StateStore.h"

#include "../config/pins.h"
#include "../matrix/MatrixController.h"
#include "SPIFFS.h"

namespace {

// Slots are written alternately; the one holding the newest valid state is
// never overwritten, so an interrupted write cannot lose the previous state.
const char* const SLOT_PATH[2] = { "/state.a", "/state.b" };

const uint32_t STATE_MAGIC = 0x54535850; // "PXST" little-endian
const uint16_t STATE_VERSION = 1;

const uint16_t FLAG_SHOW_TEXT = 0x0001;

const size_t STATE_PIXEL_WIDTH = PANEL_WIDTH * PANEL_CHAIN;
const size_t STATE_PIXEL_HEIGHT = PANEL_HEIGHT;
const size_t STATE_PIXEL_COUNT = STATE_PIXEL_WIDTH * STATE_PIXEL_HEIGHT;

// Written verbatim, so the fields are laid out explicitly instead of relying
// on the compiler's struct padding.
struct __attribute__((packed)) StateHeader {
  uint32_t magic;
  uint16_t version;
  uint16_t flags;
  uint32_t seq;
  uint32_t payloadLength;
  uint32_t crc;
  uint16_t textItemCount;
  uint16_t textItemSize;
  uint16_t pixelWidth;
  uint16_t pixelHeight;
};

// Staging area for a slot being read. The payload is validated (CRC) before
// anything is copied into the live objects, so a corrupt slot cannot leave the
// display half-restored when the other slot is then loaded.
TextItem s_textStage[StateStore::MAX_TEXT_ITEMS];
uint16_t s_pixelStage[STATE_PIXEL_COUNT];

uint32_t crc32Update(uint32_t crc, const uint8_t* data, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1)));
    }
  }
  return crc;
}

// Truncation into the panel's native 5-6-5 format. GFX_Layer expands this back
// with a scaling formula for which truncate(expand(v)) == v holds for every
// value, so saving and restoring is lossless at the precision the panel shows.
inline uint16_t rgbTo565(const CRGB& c)
{
  return (uint16_t)(((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3));
}

} // namespace

StateStore& StateStore::getInstance()
{
  static StateStore instance;
  return instance;
}

StateStore::StateStore()
    : _matrix(nullptr)
    , _textItems(nullptr)
    , _textItemCount(0)
    , _showText(nullptr)
    , _activeSlot(1) // first write targets slot 0
    , _seq(0)
    , _restoredSeq(0)
    , _dirty(false)
    , _lastChange(0)
    , _lastAttempt(0)
    , _loggedFailure(false)
{
}

void StateStore::init(
    MatrixController* matrix, TextItem* textItems, size_t textItemCount, bool* showText)
{
  _matrix = matrix;
  _textItems = textItems;
  _textItemCount = textItemCount;
  _showText = showText;
}

bool StateStore::begin()
{
  _activeSlot = 1;
  _seq = 0;
  _restoredSeq = 0;
  _dirty = false;
  _lastChange = millis();
  _lastAttempt = _lastChange;

  if (!_matrix || !_textItems || !_showText || _textItemCount == 0) {
    Serial.println("StateStore: not wired up, skipping restore");
    return false;
  }

  // Probe both slots for the newest valid sequence number. Only the winner is
  // applied, so a corrupt slot cannot partially repaint the display.
  uint32_t seq[2] = { 0, 0 };
  bool valid[2] = { false, false };

  for (uint8_t slot = 0; slot < 2; slot++) {
    valid[slot] = readSlot(slot, false, &seq[slot]);
  }

  int8_t winner = -1;
  if (valid[0] && valid[1]) {
    winner = seq[1] > seq[0] ? 1 : 0;
  } else if (valid[0]) {
    winner = 0;
  } else if (valid[1]) {
    winner = 1;
  }

  if (winner < 0) {
    Serial.println("StateStore: no stored state to restore");
    return false;
  }

  uint32_t restored = 0;
  if (!readSlot((uint8_t)winner, true, &restored)) {
    Serial.println("StateStore: restore failed while applying the slot");
    return false;
  }

  _activeSlot = (uint8_t)winner;
  _seq = restored;
  _restoredSeq = restored;

  Serial.printf("StateStore: restored slot %c (seq %u): %u text item(s), clock %s\n",
      winner == 0 ? 'a' : 'b', (unsigned)restored, (unsigned)_textItemCount,
      *_showText ? "visible" : "hidden");

  return true;
}

void StateStore::markDirty()
{
  _dirty = true;
  _lastChange = millis();
}

void StateStore::update()
{
  if (!_dirty || !_matrix) {
    return;
  }

  const uint32_t now = millis();

  // Wait for edits to settle, but do not hold a change back forever while
  // someone keeps drawing - that would mean never persisting at all.
  if (now - _lastChange < SAVE_DEBOUNCE_MS && now - _lastAttempt < SAVE_MAX_INTERVAL_MS) {
    return;
  }

  if (saveNow()) {
    _dirty = false;
  }
}

bool StateStore::saveNow()
{
  _lastAttempt = millis();

  if (!_matrix || !_textItems || !_showText || _textItemCount == 0) {
    return false;
  }

  const uint8_t target = _activeSlot ^ 1;
  const uint32_t seq = _seq + 1;

  if (!writeSlot(target, seq)) {
    if (!_loggedFailure) {
      Serial.println("StateStore: failed to write state (is SPIFFS mounted?)");
      _loggedFailure = true;
    }
    return false;
  }

  _loggedFailure = false;
  _activeSlot = target;
  _seq = seq;
  _dirty = false;

  Serial.printf("StateStore: saved to slot %c (seq %u)\n", target == 0 ? 'a' : 'b',
      (unsigned)seq);

  return true;
}

bool StateStore::writeSlot(uint8_t slot, uint32_t seq)
{
  if (slot > 1) {
    return false;
  }

  // Snapshot the background layer into the staging buffer and cover it with the
  // CRC in the same pass, so nothing is written to flash before it is known to
  // be consistent.
  layerPixels* pixels = _matrix->getBackgroundLayer().pixels;
  if (pixels == nullptr || pixels->data == nullptr) {
    return false;
  }

  uint32_t crc = 0xFFFFFFFFu;

  for (size_t y = 0; y < STATE_PIXEL_HEIGHT; y++) {
    for (size_t x = 0; x < STATE_PIXEL_WIDTH; x++) {
      s_pixelStage[y * STATE_PIXEL_WIDTH + x] = rgbTo565(pixels->data[y][x]);
    }
  }

  const size_t textBytes = sizeof(TextItem) * _textItemCount;
  const size_t pixelBytes = sizeof(uint16_t) * STATE_PIXEL_COUNT;

  crc = crc32Update(crc, (const uint8_t*)_textItems, textBytes);
  crc = crc32Update(crc, (const uint8_t*)s_pixelStage, pixelBytes);
  crc ^= 0xFFFFFFFFu;

  StateHeader header;
  header.magic = STATE_MAGIC;
  header.version = STATE_VERSION;
  header.flags = *_showText ? FLAG_SHOW_TEXT : 0;
  header.seq = seq;
  header.payloadLength = (uint32_t)(textBytes + pixelBytes);
  header.crc = crc;
  header.textItemCount = (uint16_t)_textItemCount;
  header.textItemSize = (uint16_t)sizeof(TextItem);
  header.pixelWidth = (uint16_t)STATE_PIXEL_WIDTH;
  header.pixelHeight = (uint16_t)STATE_PIXEL_HEIGHT;

  // Writing to a temporary name and renaming would not be atomic in SPIFFS
  // either, which is why the slot never holds the only copy of the state.
  File file = SPIFFS.open(SLOT_PATH[slot], "w");
  if (!file) {
    return false;
  }

  const size_t headerWritten = file.write((const uint8_t*)&header, sizeof(header));
  const size_t textWritten = file.write((const uint8_t*)_textItems, textBytes);
  const size_t pixelWritten = file.write((const uint8_t*)s_pixelStage, pixelBytes);

  const bool ok = headerWritten == sizeof(header) && textWritten == textBytes
      && pixelWritten == pixelBytes;

  file.close();

  return ok;
}

bool StateStore::readSlot(uint8_t slot, bool apply, uint32_t* outSeq)
{
  if (slot > 1) {
    return false;
  }

  File file = SPIFFS.open(SLOT_PATH[slot], "r");
  if (!file) {
    return false;
  }

  StateHeader header;
  if (file.read((uint8_t*)&header, sizeof(header)) != sizeof(header)) {
    file.close();
    return false;
  }

  if (header.magic != STATE_MAGIC || header.version != STATE_VERSION) {
    file.close();
    return false;
  }

  // Reject anything written for a different layout, so a firmware rebuilt for
  // another panel size or text structure cannot misread the payload.
  if (header.textItemCount != _textItemCount || header.textItemSize != sizeof(TextItem)
      || header.pixelWidth != STATE_PIXEL_WIDTH || header.pixelHeight != STATE_PIXEL_HEIGHT) {
    Serial.printf("StateStore: slot %c has an incompatible layout, ignoring\n",
        slot == 0 ? 'a' : 'b');
    file.close();
    return false;
  }

  const size_t textBytes = sizeof(TextItem) * _textItemCount;
  const size_t pixelBytes = sizeof(uint16_t) * STATE_PIXEL_COUNT;

  if (header.payloadLength != textBytes + pixelBytes) {
    file.close();
    return false;
  }

  uint32_t crc = 0xFFFFFFFFu;
  size_t textRead = 0;
  size_t pixelRead = 0;
  size_t remaining = textBytes + pixelBytes;
  bool ioOk = true;

  uint8_t buffer[256];
  while (remaining > 0) {
    const size_t want = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
    const int read = file.read(buffer, want);
    if (read <= 0) {
      ioOk = false;
      break;
    }

    crc = crc32Update(crc, buffer, (size_t)read);

    size_t offset = 0;
    while (offset < (size_t)read) {
      if (textRead < textBytes) {
        const size_t take = min(textBytes - textRead, (size_t)read - offset);
        memcpy((uint8_t*)s_textStage + textRead, buffer + offset, take);
        textRead += take;
        offset += take;
      } else {
        const size_t take = min(pixelBytes - pixelRead, (size_t)read - offset);
        memcpy((uint8_t*)s_pixelStage + pixelRead, buffer + offset, take);
        pixelRead += take;
        offset += take;
      }
    }

    remaining -= (size_t)read;
  }

  file.close();

  if (!ioOk || remaining != 0) {
    return false;
  }

  crc ^= 0xFFFFFFFFu;
  if (crc != header.crc) {
    Serial.printf("StateStore: slot %c failed its CRC check, ignoring\n", slot == 0 ? 'a' : 'b');
    return false;
  }

  if (outSeq != nullptr) {
    *outSeq = header.seq;
  }

  if (!apply) {
    return true;
  }

  for (size_t i = 0; i < _textItemCount; i++) {
    _textItems[i] = s_textStage[i];
  }

  *_showText = (header.flags & FLAG_SHOW_TEXT) != 0;

  // Draw through the layer's own 565 conversion so a restored pixel is stored
  // exactly as it was, instead of drifting a little on every reboot.
  for (size_t y = 0; y < STATE_PIXEL_HEIGHT; y++) {
    for (size_t x = 0; x < STATE_PIXEL_WIDTH; x++) {
      _matrix->getBackgroundLayer().drawPixel(
          (int16_t)x, (int16_t)y, s_pixelStage[y * STATE_PIXEL_WIDTH + x]);
    }
  }

  _matrix->getTextLayer().clear();

  return true;
}
