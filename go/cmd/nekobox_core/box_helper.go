package main

import (
	"context"
	"io"

	"github.com/matsuridayo/libneko/neko_log"
	box "github.com/sagernet/sing-box"
	"github.com/sagernet/sing-box/include"
	"github.com/sagernet/sing-box/log"
	"github.com/sagernet/sing-box/option"
	"github.com/sagernet/sing/common/json"
)

// platformLogWriter 将 sing-box 内部日志转发到 neko 日志 writer。
// 适配官方 sing-box >= 1.11 的 log.PlatformWriter 接口（旧 boxmain.SetLogWritter 已移除）。
type platformLogWriter struct {
	w io.Writer
}

func (p *platformLogWriter) WriteMessage(level log.Level, message string) {
	if p.w == nil {
		return
	}
	// 跳过 trace/debug 噪音，日志保持干净
	if level < log.LevelInfo {
		return
	}
	_, _ = p.w.Write([]byte(message + "\n"))
}

// createBox 解析配置并创建、启动 sing-box 实例。
// 替代旧版 boxmain.Create（官方 1.11+ 的 cmd/sing-box 不再提供库式入口）。
// 注意：传 PlatformLogWriter 会令 sing-box 无条件初始化 cache-file（cache.db）；
// 同目录下多个实例（如主实例 + URL 测试实例）会争用 bbolt 文件锁导致初始化超时，
// 因此测试/临时实例必须使用 createBoxNoLog。
func createBox(configContent []byte) (*box.Box, context.CancelFunc, error) {
	return createBoxInternal(configContent, true)
}

func createBoxNoLog(configContent []byte) (*box.Box, context.CancelFunc, error) {
	return createBoxInternal(configContent, false)
}

func createBoxInternal(configContent []byte, withLog bool) (*box.Box, context.CancelFunc, error) {
	ctx, cancel := context.WithCancel(include.Context(context.Background()))
	options, err := json.UnmarshalExtendedContext[option.Options](ctx, configContent)
	if err != nil {
		cancel()
		return nil, nil, err
	}
	// 日志不输出 ANSI 颜色码，GUI 日志面板与 neko.log 保持干净
	options.Log.DisableColor = true
	boxOptions := box.Options{
		Context: ctx,
		Options: options,
	}
	if withLog {
		boxOptions.PlatformLogWriter = &platformLogWriter{w: neko_log.LogWriter}
	}
	instance, err := box.New(boxOptions)
	if err != nil {
		cancel()
		return nil, nil, err
	}
	err = instance.Start()
	if err != nil {
		cancel()
		_ = instance.Close()
		return nil, nil, err
	}
	return instance, cancel, nil
}
