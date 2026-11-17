#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
mkdir -p build
CGO_ENABLED=0 GOOS=linux GOARCH=arm64 go build -tags lambda.norpc -o build/bootstrap .
zip -j -q build/card_read.zip build/bootstrap
