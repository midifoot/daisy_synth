#!/bin/bash

echo "🧹 Cleaning project..."
make clean
sleep 1

echo "🔨 Building project..."
# We use an if-statement here so it only flashes if the build succeeds
if make; then
    echo "✅ Build successful! Waiting to flash..."
    sleep 1
    
    echo "🚀 Flashing to Daisy Seed..."
    make program-dfu
else
    echo "❌ Build failed. Aborting flash."
    exit 1
fi