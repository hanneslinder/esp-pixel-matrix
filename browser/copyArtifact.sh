#!/bin/bash
set -e

CSS_PATH="./dist/index.css"
JS_PATH="./dist/index.js"
HTML_PATH="./dist/index.html"
DESTINATION="../esp32/data"

# The bundle is served from SPIFFS by ESPAsyncWebServer, which prefers a
# "<file>.gz" sibling over the plain file when both exist and adds
# "Content-Encoding: gzip" on its own (AsyncStaticWebHandler::_searchFile and
# the AsyncFileResponse constructor).
#
# This matters for more than load time: 152 KB of uncompressed assets fetched as
# two parallel transfers is enough to make AsyncTCP drop a connection before the
# declared Content-Length is reached, which the browser reports as
# ERR_CONTENT_LENGTH_MISMATCH. Gzipped it is 43 KB (~3.5x less).
#
# The HTML is deliberately not gzipped: it goes through a template processor and
# the library cannot process templates in gzipped responses.
#
# -n writes no name/timestamp header, so the archives are byte-identical between
# builds and only change when the bundle actually changes.
gzip -9 -n -c "$CSS_PATH" > "$CSS_PATH.gz"
gzip -9 -n -c "$JS_PATH" > "$JS_PATH.gz"

cp "$CSS_PATH" "$CSS_PATH.gz" "$JS_PATH" "$JS_PATH.gz" "$HTML_PATH" "$DESTINATION/"

echo "Files copied to $DESTINATION"
