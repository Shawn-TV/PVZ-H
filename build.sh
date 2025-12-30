#!/bin/bash

# PVZ Maze Edition 打包脚本
# 用法: ./build.sh [win|mac|linux]
# 输出: 单个可执行文件，包含所有运行所需的内容

set -e

echo "=========================================="
echo "  PVZ Maze Edition 打包脚本"
echo "=========================================="

# 检查参数
TARGET=${1:-linux}

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

# 使用 CMake 构建（Release 模式，静态链接）
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc 2>/dev/null || echo 4)

if [ -f "pvz_game" ]; then
    echo -e "${GREEN}✓ 后端编译成功${NC}"
    # 设置可执行权限
    chmod +x pvz_game
else
    echo -e "${RED}✗ 后端编译失败${NC}"
    exit 1
fi

echo -e "${YELLOW}[2/4] 安装前端依赖...${NC}"
cd "$FRONTEND_DIR"
npm install

echo -e "${YELLOW}[3/4] 构建前端（包含所有资源）...${NC}"
npm run build

if [ -d "dist" ]; then
    echo -e "${GREEN}✓ 前端构建成功${NC}"
    # 验证资源是否包含
    if [ -d "dist/assets" ]; then
        echo -e "${GREEN}✓ 资源文件已包含${NC}"
    else
        echo -e "${YELLOW}! 警告: dist/assets 目录不存在${NC}"
    fi
else
    echo -e "${RED}✗ 前端构建失败${NC}"
    exit 1
fi

echo -e "${YELLOW}[4/4] 打包 Electron 应用...${NC}"

case $TARGET in
    win)
        echo "打包 Windows 便携版..."
        npm run pack:win
        OUTPUT_FILE="release/*portable*.exe"
        ;;
    mac)
        echo "打包 macOS 版本..."
        npm run pack:mac
        OUTPUT_FILE="release/*.dmg"
        ;;
    linux)
        echo "打包 Linux AppImage..."
        npm run pack:linux
        OUTPUT_FILE="release/*.AppImage"
        ;;
    *)
        echo -e "${RED}未知目标: $TARGET${NC}"
        echo "用法: ./build.sh [win|mac|linux]"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}=========================================="
echo "  打包完成!"
echo "==========================================${NC}"
echo ""
echo -e "${YELLOW}输出目录: frontend/release/${NC}"
echo ""

# 列出生成的文件
echo "生成的可执行文件:"
cd "$FRONTEND_DIR"
ls -lh release/ 2>/dev/null | grep -E "\.(exe|dmg|AppImage)$" || echo "(没有找到输出文件)"

echo ""
echo -e "${GREEN}提示: 发送给用户只需要发送 release/ 目录下的可执行文件${NC}"
echo -e "${GREEN}      该文件已包含所有运行所需的内容（后端程序+前端资源）${NC}"
