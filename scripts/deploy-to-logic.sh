#!/bin/bash
#
# BusGlue AU Plugin Deployment Script
# Builds, validates, and deploys the AU plugin to Logic Pro
#
# Usage: ./deploy-to-logic.sh [--skip-build] [--verbose]
#

set -e

# Configuration
PROJECT_DIR="/Users/ianfletcher/busglue"
XCODE_PROJECT="${PROJECT_DIR}/Builds/MacOSX/BusGlue.xcodeproj"
BUILD_DIR="${PROJECT_DIR}/Builds/MacOSX/build/Release"
COMPONENT_NAME="BusGlue.component"
DEST_DIR="${HOME}/Library/Audio/Plug-Ins/Components"
AU_TYPE="aufx"
AU_SUBTYPE="BusG"
AU_MANUFACTURER="Flet"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Flags
SKIP_BUILD=false
VERBOSE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--skip-build] [--verbose]"
            echo ""
            echo "Options:"
            echo "  --skip-build    Skip the build step (use existing build)"
            echo "  --verbose       Show detailed output"
            echo "  -h, --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

log_section() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

# Check if a command succeeded
check_result() {
    if [ $? -eq 0 ]; then
        log_success "$1"
        return 0
    else
        log_error "$1"
        return 1
    fi
}

# Track test results
TESTS_PASSED=0
TESTS_FAILED=0

record_pass() {
    ((TESTS_PASSED++))
    log_success "$1"
}

record_fail() {
    ((TESTS_FAILED++))
    log_error "$1"
}

#
# STEP 1: Pre-flight Checks
#
log_section "Step 1: Pre-flight Checks"

# Check Xcode project exists
if [ -d "$XCODE_PROJECT" ]; then
    record_pass "Xcode project exists"
else
    record_fail "Xcode project not found at $XCODE_PROJECT"
    exit 1
fi

# Check destination directory
if [ -d "$DEST_DIR" ]; then
    record_pass "Components directory exists"
else
    log_info "Creating Components directory..."
    mkdir -p "$DEST_DIR"
    record_pass "Components directory created"
fi

# Check if plugin is in use
if lsof "${DEST_DIR}/${COMPONENT_NAME}/Contents/MacOS/BusGlue" 2>/dev/null | grep -q .; then
    record_fail "Plugin is currently in use by another application"
    log_warning "Please close Logic Pro and any other DAWs before deploying"
    exit 1
else
    record_pass "Plugin not locked by other applications"
fi

#
# STEP 2: Build AU Plugin
#
log_section "Step 2: Build AU Plugin"

if [ "$SKIP_BUILD" = true ]; then
    log_info "Skipping build (--skip-build flag set)"
    if [ -d "${BUILD_DIR}/${COMPONENT_NAME}" ]; then
        record_pass "Existing build found"
    else
        record_fail "No existing build found and --skip-build was set"
        exit 1
    fi
else
    log_info "Building AU plugin (Release configuration)..."

    BUILD_OUTPUT=$(mktemp)

    if $VERBOSE; then
        xcodebuild -project "$XCODE_PROJECT" \
            -scheme "BusGlue - AU" \
            -configuration Release \
            build 2>&1 | tee "$BUILD_OUTPUT"
        BUILD_RESULT=${PIPESTATUS[0]}
    else
        xcodebuild -project "$XCODE_PROJECT" \
            -scheme "BusGlue - AU" \
            -configuration Release \
            build > "$BUILD_OUTPUT" 2>&1
        BUILD_RESULT=$?
    fi

    if [ $BUILD_RESULT -eq 0 ]; then
        record_pass "Build succeeded"
    else
        record_fail "Build failed"
        echo ""
        echo "Build output:"
        tail -50 "$BUILD_OUTPUT"
        rm "$BUILD_OUTPUT"
        exit 1
    fi

    # Check for warnings
    WARNING_COUNT=$(grep -c "warning:" "$BUILD_OUTPUT" 2>/dev/null || echo "0")
    if [ "$WARNING_COUNT" -gt 0 ]; then
        log_warning "Build completed with $WARNING_COUNT warning(s)"
    fi

    rm "$BUILD_OUTPUT"
fi

# Verify component was created
if [ -d "${BUILD_DIR}/${COMPONENT_NAME}" ]; then
    record_pass "Component bundle created"
else
    record_fail "Component bundle not found after build"
    exit 1
fi

#
# STEP 3: Verify Build
#
log_section "Step 3: Verify Build"

# Check binary architecture
ARCH_INFO=$(file "${BUILD_DIR}/${COMPONENT_NAME}/Contents/MacOS/BusGlue" 2>/dev/null)
if echo "$ARCH_INFO" | grep -q "arm64\|x86_64"; then
    record_pass "Binary architecture valid"
    if $VERBOSE; then
        log_info "Architecture: $ARCH_INFO"
    fi
else
    record_fail "Invalid binary architecture"
    exit 1
fi

# Verify code signature
if codesign --verify --verbose "${BUILD_DIR}/${COMPONENT_NAME}" 2>/dev/null; then
    record_pass "Code signature valid"
else
    log_warning "Component not signed - signing with ad-hoc signature..."
    codesign --force --deep --sign - "${BUILD_DIR}/${COMPONENT_NAME}"
    if codesign --verify --verbose "${BUILD_DIR}/${COMPONENT_NAME}" 2>/dev/null; then
        record_pass "Ad-hoc signature applied successfully"
    else
        record_fail "Failed to sign component"
        exit 1
    fi
fi

#
# STEP 4: AU Validation
#
log_section "Step 4: AU Validation"

log_info "Running auval validation..."

AUVAL_OUTPUT=$(mktemp)
auval -v "$AU_TYPE" "$AU_SUBTYPE" "$AU_MANUFACTURER" > "$AUVAL_OUTPUT" 2>&1
AUVAL_RESULT=$?

if [ $AUVAL_RESULT -eq 0 ]; then
    # Check for any FAIL results in output
    if grep -q "FAIL" "$AUVAL_OUTPUT"; then
        record_fail "AU validation had failures"
        grep "FAIL" "$AUVAL_OUTPUT"
    else
        record_pass "AU validation passed"
    fi
else
    record_fail "AU validation returned error code $AUVAL_RESULT"
    if $VERBOSE; then
        cat "$AUVAL_OUTPUT"
    else
        tail -20 "$AUVAL_OUTPUT"
    fi
fi

# Count pass/fail from auval
AUVAL_PASSES=$(grep -c "PASS" "$AUVAL_OUTPUT" 2>/dev/null || echo "0")
log_info "auval reported $AUVAL_PASSES passed tests"

rm "$AUVAL_OUTPUT"

#
# STEP 5: Deploy to Components Folder
#
log_section "Step 5: Deploy to Components Folder"

# Remove old version if exists
if [ -d "${DEST_DIR}/${COMPONENT_NAME}" ]; then
    log_info "Removing old version..."
    rm -rf "${DEST_DIR}/${COMPONENT_NAME}"
    if [ $? -eq 0 ]; then
        record_pass "Old version removed"
    else
        record_fail "Failed to remove old version"
        exit 1
    fi
fi

# Copy new version
log_info "Copying new version to Components folder..."
cp -R "${BUILD_DIR}/${COMPONENT_NAME}" "${DEST_DIR}/"
if [ $? -eq 0 ]; then
    record_pass "New version deployed"
else
    record_fail "Failed to copy component"
    exit 1
fi

# Verify copy
if [ -f "${DEST_DIR}/${COMPONENT_NAME}/Contents/MacOS/BusGlue" ]; then
    record_pass "Deployment verified"
else
    record_fail "Component binary missing after deployment"
    exit 1
fi

#
# STEP 6: Clear AU Cache
#
log_section "Step 6: Clear AU Cache"

log_info "Clearing AudioUnit cache..."

# Kill AudioComponentRegistrar if running
killall -9 AudioComponentRegistrar 2>/dev/null || true

# Remove cache directory
if [ -d ~/Library/Caches/AudioUnitCache ]; then
    rm -rf ~/Library/Caches/AudioUnitCache
    log_info "AudioUnitCache directory removed"
fi

# Also clear the newer cache location
if [ -d ~/Library/Caches/com.apple.audiounits.cache ]; then
    rm -rf ~/Library/Caches/com.apple.audiounits.cache
    log_info "com.apple.audiounits.cache directory removed"
fi

record_pass "AU cache cleared"

#
# STEP 7: Post-Deployment Validation
#
log_section "Step 7: Post-Deployment Validation"

log_info "Running post-deployment validation..."

# Give system a moment to recognize new component
sleep 1

# Re-run auval on deployed component
AUVAL_OUTPUT=$(mktemp)
auval -v "$AU_TYPE" "$AU_SUBTYPE" "$AU_MANUFACTURER" > "$AUVAL_OUTPUT" 2>&1
AUVAL_RESULT=$?

if [ $AUVAL_RESULT -eq 0 ] && ! grep -q "FAIL" "$AUVAL_OUTPUT"; then
    record_pass "Post-deployment validation passed"
else
    record_fail "Post-deployment validation failed"
    if $VERBOSE; then
        cat "$AUVAL_OUTPUT"
    fi
fi

rm "$AUVAL_OUTPUT"

# Verify plugin appears in AU list
if auval -a 2>/dev/null | grep -q "${AU_SUBTYPE}.*${AU_MANUFACTURER}"; then
    record_pass "Plugin appears in AU component list"
else
    log_warning "Plugin may not appear in AU list yet - try rescanning in Logic Pro"
fi

# Check deployed signature
if codesign --verify --verbose "${DEST_DIR}/${COMPONENT_NAME}" 2>/dev/null; then
    record_pass "Deployed component signature valid"
else
    log_warning "Deployed component signature may need attention"
fi

#
# Summary
#
log_section "Deployment Summary"

echo ""
echo -e "Tests Passed: ${GREEN}${TESTS_PASSED}${NC}"
echo -e "Tests Failed: ${RED}${TESTS_FAILED}${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  DEPLOYMENT SUCCESSFUL!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo "Plugin deployed to: ${DEST_DIR}/${COMPONENT_NAME}"
    echo ""
    echo "Next steps:"
    echo "  1. Open Logic Pro"
    echo "  2. Go to Logic Pro > Settings > Plug-in Manager"
    echo "  3. Click 'Reset & Rescan Selection' if needed"
    echo "  4. Add plugin to a track: Audio FX > Audio Units > Fletcher > Bus Glue"
    echo ""
    exit 0
else
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}  DEPLOYMENT COMPLETED WITH ERRORS${NC}"
    echo -e "${RED}========================================${NC}"
    echo ""
    echo "Please review the failures above and address them."
    echo ""
    exit 1
fi
