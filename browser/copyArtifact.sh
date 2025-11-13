#!/bin/bash

CSS_PATH="./dist/index.css"
JS_PATH="./dist/index.js"
HTML_PATH="./dist/index.html"
DESTINATION="../esp32/data"

cp "$CSS_PATH" "$JS_PATH" "$HTML_PATH" "$DESTINATION/"

echo "Files copied to $DESTINATION"
