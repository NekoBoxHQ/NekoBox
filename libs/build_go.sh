#!/bin/bash
set -e

source libs/env_deploy.sh
[ "$GOOS" == "windows" ] && [ "$GOARCH" == "amd64" ] && DEST=$DEPLOYMENT/windows64 || true
[ "$GOOS" == "windows" ] && [ "$GOARCH" == "arm64" ] && DEST=$DEPLOYMENT/windows-arm64 || true
[ "$GOOS" == "linux" ] && [ "$GOARCH" == "amd64" ] && DEST=$DEPLOYMENT/linux64 || true
[ "$GOOS" == "linux" ] && [ "$GOARCH" == "arm64" ] && DEST=$DEPLOYMENT/linux-arm64 || true
if [ -z $DEST ]; then
  echo "Please set GOOS GOARCH"
  exit 1
fi
rm -rf $DEST
mkdir -p $DEST

export CGO_ENABLED=0

#### Go: updater ####
pushd go/cmd/updater
[ "$GOOS" == "darwin" ] || go build -o $DEST -trimpath -ldflags "-w -s"
[ "$GOOS" == "linux" ] && mv $DEST/updater $DEST/launcher || true
popd

#### Go: nekobox_core ####
pushd go/cmd/nekobox_core
# 注意: sing-box 官方 1.14 起 with_ech tag 已废弃(ECH 并入 Go 标准库), 新增 with_grpc
SINGBOX_VERSION="v1.14.1" # 升级 sing-box 时同步修改
go build -v -o $DEST -trimpath -ldflags "-w -s -X github.com/matsuridayo/libneko/neko_common.Version_neko=$version_standalone -X github.com/sagernet/sing-box/constant.Version=$SINGBOX_VERSION" -tags "with_clash_api,with_gvisor,with_quic,with_wireguard,with_utls,with_grpc"
popd
