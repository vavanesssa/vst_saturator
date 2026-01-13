#!/bin/bash
PASSWORD="aaaa"
BUILD_HASH=$(openssl rand -hex 3 | cut -c1-5 | tr '[:lower:]' '[:upper:]')
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

echo "🔨 Building steverator (Standalone + VST3)..."
echo "📦 BUILD HASH: $BUILD_HASH"
echo ""

mkdir -p Assets
cat > Assets/version.txt << VERSIONEOF
$BUILD_HASH
$TIMESTAMP
VERSIONEOF

echo "✓ version.txt: $BUILD_HASH"
echo ""

cmake -B build -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1
cmake --build build --config Release 2>&1 | grep -E "(Building|Linking|Built|error)" | tail -15

echo ""
echo "✨ BUILD COMPLETE! Hash: $BUILD_HASH"
echo ""

STANDALONE_APP="build/steverator_artefacts/Release/Standalone/steverator.app"

# Copy version.txt and PNG images to both VST3 and Standalone Resources
cp Assets/version.txt "build/steverator_artefacts/Release/Standalone/steverator.app/Contents/Resources/" 2>/dev/null || true
cp Assets/*.png "build/steverator_artefacts/Release/Standalone/steverator.app/Contents/Resources/" 2>/dev/null || true
cp Assets/version.txt "build/steverator_artefacts/Release/VST3/steverator.vst3/Contents/Resources/" 2>/dev/null || true
cp Assets/*.png "build/steverator_artefacts/Release/VST3/steverator.vst3/Contents/Resources/" 2>/dev/null || true

echo "✓ version.txt and PNG images copied to Resources"
echo ""

if [ -d "$STANDALONE_APP" ]; then
    echo "🎛️  Opening Standalone app..."
    echo "   Path: $STANDALONE_APP"
    open "$STANDALONE_APP"
    sleep 1
    echo ""
    echo "✅ App should be opening now!"
    echo "   Look for '$BUILD_HASH' in ORANGE at the bottom"
else
    echo "❌ Standalone app not found at:"
    echo "   $STANDALONE_APP"
    echo ""
    echo "Checking what was built..."
    ls -la build/steverator_artefacts/Release/Standalone/ 2>/dev/null || echo "No Standalone folder found"
fi
