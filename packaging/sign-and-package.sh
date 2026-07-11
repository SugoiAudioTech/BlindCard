#!/bin/bash
set -e

# ===========================================
# BlindCard Code Signing and Packaging Script
# 簽署插件並建立 .pkg 安裝程式
#
# 用法:
#   ./sign-and-package.sh                    # Universal Binary (預設)
#   ./sign-and-package.sh --arch intel       # Intel (x86_64) only
#   ./sign-and-package.sh --arch apple-silicon  # Apple Silicon (arm64) only
#
# ⚠️ 注意事項（2026-04-07 慘痛教訓）:
#
#   v1.0.4 打包時 build-apple-silicon/ 裡實際放的是 x86_64 binary，
#   但腳本沒有驗證架構就直接打包，產出了一個「檔名寫 AppleSilicon、
#   內容是 x86_64」的 PKG。
#
#   後果：Cubase Elements 15 嚴格要求 native arm64，直接拒絕載入
#   並顯示 "Unsupported Architecture (x86_64)"。而 Reaper/Live/FL Studio
#   因為有 Rosetta 2 相容層所以正常運作，導致問題長時間未被發現。
#
#   修復：已加入「架構驗證 Gate」，打包前自動用 lipo -archs 檢查
#   binary 架構是否符合 --arch 參數，不符合直接 exit 1 擋下。
#
#   預防：打包完成後務必用以下指令二次確認 PKG 內容：
#     pkgutil --expand xxx.pkg /tmp/expanded
#     file /tmp/expanded/BlindCard-VST3.pkg/payload_out/.../Blind\ Card
# ===========================================

# 解析參數
ARCH=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --arch)
            ARCH="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 設定（自動偵測專案目錄）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "${SCRIPT_DIR}")"
PACKAGING_DIR="${PROJECT_DIR}/Packaging"
OUTPUT_DIR="${PROJECT_DIR}/dist"

# 根據架構設定 build 目錄和 PKG 名稱後綴
case "${ARCH}" in
    intel)
        BUILD_DIR="${PROJECT_DIR}/build-intel"
        ARCH_SUFFIX="-Intel"
        ;;
    apple-silicon)
        BUILD_DIR="${PROJECT_DIR}/build-apple-silicon"
        ARCH_SUFFIX="-AppleSilicon"
        ;;
    "")
        BUILD_DIR="${PROJECT_DIR}/build"
        ARCH_SUFFIX=""
        ;;
    *)
        echo "❌ Unknown architecture: ${ARCH}"
        echo "Valid options: intel, apple-silicon"
        exit 1
        ;;
esac

# 支援兩種 build 路徑：Makefile (無 Release/) 和 Xcode (有 Release/)
if [ -d "${BUILD_DIR}/BlindCard_artefacts/Release/AU" ]; then
    ARTIFACTS_DIR="${BUILD_DIR}/BlindCard_artefacts/Release"
else
    ARTIFACTS_DIR="${BUILD_DIR}/BlindCard_artefacts"
fi

# 從 CMakeLists.txt 讀取版本
VERSION=$(grep 'project(BlindCard VERSION' "${PROJECT_DIR}/CMakeLists.txt" | sed 's/.*VERSION \([0-9.]*\).*/\1/')
PKG_NAME="BlindCard-${VERSION}${ARCH_SUFFIX}"

# 自動更新 distribution.xml 的版本號
DIST_XML="${PACKAGING_DIR}/Distribution/distribution.xml"
if [ -f "${DIST_XML}" ]; then
    sed -i '' "s/version=\"[0-9]*\.[0-9]*\.[0-9]*\"/version=\"${VERSION}\"/g" "${DIST_XML}"
    echo "✓ distribution.xml 已更新至 v${VERSION}"
fi

# Developer ID 憑證
DEVELOPER_ID_APP="Developer ID Application: WEICHE LAI (46W85TC53V)"
DEVELOPER_ID_INSTALLER="Developer ID Installer: WEICHE LAI (46W85TC53V)"

# 插件路徑
AU_COMPONENT="${ARTIFACTS_DIR}/AU/Blind Card.component"
VST3_PLUGIN="${ARTIFACTS_DIR}/VST3/Blind Card.vst3"
STANDALONE_APP="${ARTIFACTS_DIR}/Standalone/Blind Card.app"
AAX_PLUGIN="${ARTIFACTS_DIR}/AAX/Blind Card.aaxplugin"
ENTITLEMENTS="${PACKAGING_DIR}/BlindCard.entitlements"

# PACE Eden 簽署（AAX 專用）
WRAPTOOL="/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool"
PACE_ACCOUNT="steven961302"
AAX_WRAP_GUID="2BC7F230-7D47-11F1-A961-005056928F3B"   # BlindCard Wrap Config (SDK5)
AAX_AVID_INSTALL="/Library/Application Support/Avid/Audio/Plug-Ins"

# AAX 只在 universal（預設）build 打包；per-arch installer 不含 AAX
BUILD_AAX=false
if [ -z "${ARCH}" ]; then BUILD_AAX=true; fi

echo "=== BlindCard Code Signing and Packaging ==="
echo "Version: ${VERSION}"
echo "AU:         ${AU_COMPONENT}"
echo "VST3:       ${VST3_PLUGIN}"
echo "Standalone: ${STANDALONE_APP}"
echo ""

# 檢查憑證
echo "檢查 Developer ID 憑證..."
if ! security find-identity -v -p codesigning | grep -q "Developer ID Application"; then
    echo "⚠️  警告: 找不到 Developer ID Application 憑證"
    exit 1
fi

# 檢查 build artifacts
if [ ! -d "${AU_COMPONENT}" ]; then
    echo "❌ 找不到 AU Component: ${AU_COMPONENT}"
    echo "請先執行 cmake --build build"
    exit 1
fi
if [ ! -d "${VST3_PLUGIN}" ]; then
    echo "❌ 找不到 VST3 Plugin: ${VST3_PLUGIN}"
    exit 1
fi
if [ ! -d "${STANDALONE_APP}" ]; then
    echo "❌ 找不到 Standalone App: ${STANDALONE_APP}"
    exit 1
fi

# ============================================
# AAX Pre-flight（PACE 簽署前置條件，只在 universal build）
# ============================================
if [ "${BUILD_AAX}" = true ]; then
    echo "AAX pre-flight 檢查..."
    if [ ! -x "${WRAPTOOL}" ]; then
        echo "❌ 找不到 wraptool: ${WRAPTOOL}（PACE Eden 未安裝）"
        exit 1
    fi
    if ! ioreg -p IOUSB 2>/dev/null | grep -qi "ilok"; then
        echo "❌ iLok 未連接（AAX PACE 簽署需要實體 iLok + Sugoi signing cert）"
        exit 1
    fi
    if [ ! -d "${AAX_PLUGIN}" ]; then
        echo "❌ 找不到 AAX Plugin: ${AAX_PLUGIN}"
        echo "   請先執行 universal build: cmake --build build --config Release"
        exit 1
    fi
    echo "✓ AAX pre-flight OK（wraptool + iLok + .aaxplugin 皆就緒）"
fi

# ============================================
# 架構驗證 Gate（防止打包錯誤架構的 binary）
# ============================================
verify_arch() {
    local binary_path="$1"
    local label="$2"
    local actual_arch
    actual_arch=$(lipo -archs "${binary_path}" 2>/dev/null)

    case "${ARCH}" in
        intel)
            if [[ "${actual_arch}" != *"x86_64"* ]]; then
                echo "❌ 架構驗證失敗: ${label}"
                echo "   期望: x86_64, 實際: ${actual_arch}"
                echo "   請重新編譯: cmake -B build-intel -DCMAKE_OSX_ARCHITECTURES=x86_64 && cmake --build build-intel --config Release"
                exit 1
            fi
            if [[ "${actual_arch}" == *"arm64"* ]]; then
                echo "⚠️  警告: ${label} 包含 arm64（期望 Intel-only），將繼續但 PKG 會偏大"
            fi
            ;;
        apple-silicon)
            if [[ "${actual_arch}" != *"arm64"* ]]; then
                echo "❌ 架構驗證失敗: ${label}"
                echo "   期望: arm64, 實際: ${actual_arch}"
                echo "   請重新編譯: cmake -B build-apple-silicon -DCMAKE_OSX_ARCHITECTURES=arm64 && cmake --build build-apple-silicon --config Release"
                exit 1
            fi
            if [[ "${actual_arch}" == *"x86_64"* ]]; then
                echo "⚠️  警告: ${label} 包含 x86_64（期望 Apple Silicon-only），將繼續但 PKG 會偏大"
            fi
            ;;
        "")
            if [[ "${actual_arch}" != *"arm64"* ]] || [[ "${actual_arch}" != *"x86_64"* ]]; then
                echo "❌ 架構驗證失敗: ${label}"
                echo "   期望: Universal (arm64 + x86_64), 實際: ${actual_arch}"
                echo "   請重新編譯: cmake -B build -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64' && cmake --build build --config Release"
                exit 1
            fi
            ;;
    esac
    echo "✓ ${label}: ${actual_arch}"
}

echo "驗證 binary 架構..."
verify_arch "${AU_COMPONENT}/Contents/MacOS/Blind Card" "AU Component"
verify_arch "${VST3_PLUGIN}/Contents/MacOS/Blind Card" "VST3 Plugin"
verify_arch "${STANDALONE_APP}/Contents/MacOS/Blind Card" "Standalone App"
if [ "${BUILD_AAX}" = true ]; then
    verify_arch "${AAX_PLUGIN}/Contents/MacOS/Blind Card" "AAX Plugin"
fi
echo ""

# 建立輸出目錄
mkdir -p "${OUTPUT_DIR}"

# ============================================
# Step 1: 簽署 AU Component
# ============================================
echo "[1/7] 簽署 AU Component..."
codesign --force --deep --timestamp \
         --options runtime \
         --entitlements "${ENTITLEMENTS}" \
         --sign "${DEVELOPER_ID_APP}" \
         "${AU_COMPONENT}"

echo "驗證 AU 簽章..."
codesign --verify --deep --strict --verbose=2 "${AU_COMPONENT}"

# ============================================
# Step 2: 簽署 VST3 Plugin
# ============================================
echo ""
echo "[2/7] 簽署 VST3 Plugin..."
codesign --force --deep --timestamp \
         --options runtime \
         --entitlements "${ENTITLEMENTS}" \
         --sign "${DEVELOPER_ID_APP}" \
         "${VST3_PLUGIN}"

echo "驗證 VST3 簽章..."
codesign --verify --deep --strict --verbose=2 "${VST3_PLUGIN}"

# ============================================
# Step 3: 簽署 Standalone App
# ============================================
echo ""
echo "[3/7] 簽署 Standalone App..."
codesign --force --deep --timestamp \
         --options runtime \
         --entitlements "${ENTITLEMENTS}" \
         --sign "${DEVELOPER_ID_APP}" \
         "${STANDALONE_APP}"

echo "驗證 Standalone 簽章..."
codesign --verify --deep --strict --verbose=2 "${STANDALONE_APP}"

# ============================================
# Step 3.5: PACE Eden 簽署 AAX（含 hardened runtime，notarization-ready）
# ⚠️ 必帶 --extrasigningoptions，否則 wraptool 內部 codesign 不加 hardened
#    runtime/timestamp/entitlements → Apple 公證會失敗。
# ⚠️ 簽完不要再修改 bundle 內容（會破壞 PACE dsig symlink → PT 拒載）。
# ============================================
if [ "${BUILD_AAX}" = true ]; then
    echo ""
    echo "[3.5/8] PACE Eden 簽署 AAX Plugin..."
    "${WRAPTOOL}" sign \
        --verbose \
        --account "${PACE_ACCOUNT}" \
        --wcguid "${AAX_WRAP_GUID}" \
        --signid "${DEVELOPER_ID_APP}" \
        --in "${AAX_PLUGIN}" \
        --autoinstall on \
        --extrasigningoptions "--timestamp --options runtime --entitlements ${ENTITLEMENTS}"

    echo "驗證 AAX PACE 簽章..."
    "${WRAPTOOL}" verify --in "${AAX_PLUGIN}"

    echo "驗證 AAX Apple 簽章（必含 hardened runtime + timestamp）..."
    codesign -dvvv "${AAX_PLUGIN}" 2>&1 | grep -E "flags=0x10000|Timestamp=" || {
        echo "❌ AAX 缺 hardened runtime 或 timestamp，公證會失敗"
        exit 1
    }
fi

# ============================================
# Step 4: 建立元件包
# ============================================
echo ""
echo "[4/7] 建立元件包..."

# AU Package (含 preinstall 腳本清除舊插件)
pkgbuild --component "${AU_COMPONENT}" \
         --install-location "/Library/Audio/Plug-Ins/Components" \
         --identifier "com.sugoi.blindcard.au" \
         --version "${VERSION}" \
         --scripts "${PACKAGING_DIR}/Scripts" \
         "${OUTPUT_DIR}/BlindCard-AU.pkg"

# VST3 Package
pkgbuild --component "${VST3_PLUGIN}" \
         --install-location "/Library/Audio/Plug-Ins/VST3" \
         --identifier "com.sugoi.blindcard.vst3" \
         --version "${VERSION}" \
         "${OUTPUT_DIR}/BlindCard-VST3.pkg"

# Standalone Package
pkgbuild --component "${STANDALONE_APP}" \
         --install-location "/Applications" \
         --identifier "com.sugoi.blindcard.standalone" \
         --version "${VERSION}" \
         "${OUTPUT_DIR}/BlindCard-Standalone.pkg"

# AAX Package（安裝到 Avid 插件路徑；pkgbuild 的 payload 封裝會保留 PACE dsig symlink）
if [ "${BUILD_AAX}" = true ]; then
    pkgbuild --component "${AAX_PLUGIN}" \
             --install-location "${AAX_AVID_INSTALL}" \
             --identifier "com.sugoi.blindcard.aax" \
             --version "${VERSION}" \
             "${OUTPUT_DIR}/BlindCard-AAX.pkg"
fi

# ============================================
# Step 5: 建立發布安裝程式
# ============================================
echo ""
echo "[5/7] 建立發布安裝程式..."

# 選擇 distribution：universal 含 AAX；per-arch 剝掉 AAX pkg-ref（該 arch 無 AAX pkg）
DIST_SRC="${PACKAGING_DIR}/Distribution/distribution.xml"
if [ "${BUILD_AAX}" = true ]; then
    DIST_FILE="${DIST_SRC}"
else
    DIST_FILE="${OUTPUT_DIR}/distribution-active.xml"
    grep -v 'com.sugoi.blindcard.aax' "${DIST_SRC}" > "${DIST_FILE}"
fi

productbuild --distribution "${DIST_FILE}" \
             --package-path "${OUTPUT_DIR}" \
             --resources "${PACKAGING_DIR}/Distribution" \
             "${OUTPUT_DIR}/${PKG_NAME}-unsigned.pkg"

# ============================================
# Step 6: 簽署安裝程式
# ============================================
echo ""
echo "[6/7] 簽署安裝程式..."
productsign --sign "${DEVELOPER_ID_INSTALLER}" \
            "${OUTPUT_DIR}/${PKG_NAME}-unsigned.pkg" \
            "${OUTPUT_DIR}/${PKG_NAME}.pkg"

# 移除中間檔案
rm "${OUTPUT_DIR}/${PKG_NAME}-unsigned.pkg"
rm "${OUTPUT_DIR}/BlindCard-AU.pkg"
rm "${OUTPUT_DIR}/BlindCard-VST3.pkg"
rm "${OUTPUT_DIR}/BlindCard-Standalone.pkg"
[ "${BUILD_AAX}" = true ] && rm -f "${OUTPUT_DIR}/BlindCard-AAX.pkg"
[ -f "${OUTPUT_DIR}/distribution-active.xml" ] && rm -f "${OUTPUT_DIR}/distribution-active.xml"

# ============================================
# Step 7: 驗證
# ============================================
echo ""
echo "[7/7] 驗證安裝程式簽章..."
pkgutil --check-signature "${OUTPUT_DIR}/${PKG_NAME}.pkg"

echo ""
echo "=== Signing and Packaging Complete ==="
echo "Package: ${OUTPUT_DIR}/${PKG_NAME}.pkg"
