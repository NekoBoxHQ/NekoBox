#!/bin/bash
set -e

source libs/env_deploy.sh
ENV_NEKORAY=1
source libs/get_source_env.sh
pushd ..

####

# sing-box: 使用官方源（MatsuriDayo fork 已停止维护）
if [ ! -d "sing-box" ]; then
  git clone --no-checkout https://github.com/SagerNet/sing-box.git
fi
pushd sing-box
git checkout "$COMMIT_SING_BOX"
popd

####

# sing-quic: 不再需要本地 fork，使用官方 go 模块版本（见 go.mod）

####

if [ ! -d "libneko" ]; then
  git clone --no-checkout https://github.com/MatsuriDayo/libneko.git
fi
pushd libneko
git checkout "$COMMIT_LIBNEKO"
popd

####

popd
