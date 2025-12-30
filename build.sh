#!/bin/bash

# PVZ Maze Edition 打包脚本
# 用法: ./build.sh [win|mac|linux|all]

set -e

echo "=========================================="
echo "  PVZ Maze Edition 打包脚本"
echo "=========================================="

# 检查参数
TARGET=${1:-all}

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 项目根目录
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BACKEND_DIR="$ROOT_DIR/backend"
FRONTEND_DIR="$ROOT_DIR/frontend"

echo -e "${YELLOW}[1/4] 编译 C++ 后端...${NC}"
cd "$BACKEND_DIR"

# 创建 build 目录
mkdir -p build
cd build

# 使用 CMake 构建（Release 模式）
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc 2>/dev/null || echo 4)

if [ -f "pvz_game" ]; then
    echo -e "${GREEN}✓ 后端编译成功${NC}"
else
    echo -e "${RED}✗ 后端编译失败${NC}"
    exit 1
fi

echo -e "${YELLOW}[2/4] 安装前端依赖...${NC}"
cd "$FRONTEND_DIR"
npm install

echo -e "${YELLOW}[3/4] 构建前端...${NC}"
npm run build

if [ -d "dist" ]; then
    echo -e "${GREEN}✓ 前端构建成功${NC}"
else
    echo -e "${RED}✗ 前端构建失败${NC}"
    exit 1
fi

echo -e "${YELLOW}[4/4] 打包 Electron 应用...${NC}"

case $TARGET in
    win)
        echo "打包 Windows 版本..."
        npm run pack:win
        ;;
    mac)
        echo "打包 macOS 版本..."
        npm run pack:mac
        ;;
    linux)
        echo "打包 Linux 版本..."
        npm run pack:linux
        ;;
    all)
        echo "打包所有平台版本..."
        npm run electron:build
        ;;
    *)
        echo -e "${RED}未知目标: $TARGET${NC}"
        echo "用法: ./build.sh [win|mac|linux|all]"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}=========================================="
echo "  打包完成!"
echo "  输出目录: frontend/release/"
echo "==========================================${NC}"

# 列出生成的文件
echo ""
echo "生成的安装包:"
ls -lh "$FRONTEND_DIR/release/" 2>/dev/null || echo "(没有找到输出文件)"
