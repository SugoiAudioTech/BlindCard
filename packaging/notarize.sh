#!/bin/bash
set -e

# ===========================================
# BlindCard Notarization Script
# 提交 Apple 公證並附加票據
#
# 用法:
#   ./notarize.sh                    # Universal Binary (預設)
#   ./notarize.sh --arch intel       # Intel (x86_64) only
#   ./notarize.sh --arch apple-silicon  # Apple Silicon (arm64) only
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
OUTPUT_DIR="${PROJECT_DIR}/dist"

# 根據架構設定 PKG 名稱後綴
case "${ARCH}" in
    intel)       ARCH_SUFFIX="-Intel" ;;
    apple-silicon) ARCH_SUFFIX="-AppleSilicon" ;;
    "")          ARCH_SUFFIX="" ;;
    *)
        echo "❌ Unknown architecture: ${ARCH}"
        echo "Valid options: intel, apple-silicon"
        exit 1
        ;;
esac

# 從 CMakeLists.txt 讀取版本
VERSION=$(grep 'project(BlindCard VERSION' "${PROJECT_DIR}/CMakeLists.txt" | sed 's/.*VERSION \([0-9.]*\).*/\1/')
PKG_NAME="BlindCard-${VERSION}${ARCH_SUFFIX}"
PKG_PATH="${OUTPUT_DIR}/${PKG_NAME}.pkg"

# Keychain profile 名稱 (需先執行一次性設定)
NOTARYTOOL_PROFILE="notarytool-profile"

echo "=== BlindCard Notarization ==="
echo "Package: ${PKG_PATH}"
echo ""

# 檢查 keychain profile
if ! xcrun notarytool history --keychain-profile "${NOTARYTOOL_PROFILE}" &>/dev/null; then
    echo "⚠️  找不到 notarytool keychain profile"
    echo ""
    echo "請先執行一次性設定:"
    echo "  xcrun notarytool store-credentials \"${NOTARYTOOL_PROFILE}\" \\"
    echo "      --apple-id \"你的Apple ID\" \\"
    echo "      --team-id \"46W85TC53V\" \\"
    echo "      --password \"App專用密碼\""
    echo ""
    echo "App 專用密碼從 https://appleid.apple.com/account/manage 產生"
    exit 1
fi

# 檢查套件是否存在
if [ ! -f "${PKG_PATH}" ]; then
    echo "❌ 找不到套件: ${PKG_PATH}"
    echo "請先執行 sign-and-package.sh"
    exit 1
fi

# ============================================
# Step 1: 提交公證
# ============================================
echo "[1/3] 提交公證 (可能需要幾分鐘)..."
SUBMISSION_OUTPUT=$(xcrun notarytool submit "${PKG_PATH}" \
                    --keychain-profile "${NOTARYTOOL_PROFILE}" \
                    --wait \
                    --timeout 30m \
                    2>&1)

echo "${SUBMISSION_OUTPUT}"

# 提取 submission ID
SUBMISSION_ID=$(echo "${SUBMISSION_OUTPUT}" | grep -o 'id: [a-f0-9-]*' | head -1 | cut -d' ' -f2)

# 取得公證日誌
if [ -n "${SUBMISSION_ID}" ]; then
    echo ""
    echo "取得公證日誌..."
    xcrun notarytool log "${SUBMISSION_ID}" \
          --keychain-profile "${NOTARYTOOL_PROFILE}" \
          "${OUTPUT_DIR}/notarization-log.json" 2>/dev/null || true

    if [ -f "${OUTPUT_DIR}/notarization-log.json" ]; then
        echo "日誌已儲存: ${OUTPUT_DIR}/notarization-log.json"
    fi
fi

# 檢查公證結果
if echo "${SUBMISSION_OUTPUT}" | grep -q "status: Accepted"; then
    echo ""
    echo "✅ 公證成功!"

    # ============================================
    # Step 2: 附加公證票據
    # ============================================
    echo ""
    echo "[2/3] 附加公證票據..."
    xcrun stapler staple "${PKG_PATH}"

    # ============================================
    # Step 3: 驗證
    # ============================================
    echo ""
    echo "[3/3] 驗證 Gatekeeper..."
    spctl --assess --type install --verbose "${PKG_PATH}"

    echo ""
    echo "=== Notarization Complete ==="
    echo "最終套件: ${PKG_PATH}"

    # 計算校驗碼
    echo ""
    echo "套件校驗碼:"
    echo "SHA-256: $(shasum -a 256 "${PKG_PATH}" | awk '{print $1}')"
    echo "Size: $(ls -lh "${PKG_PATH}" | awk '{print $5}')"

else
    echo ""
    echo "❌ 公證失敗!"
    echo "請查看日誌: ${OUTPUT_DIR}/notarization-log.json"
    exit 1
fi
