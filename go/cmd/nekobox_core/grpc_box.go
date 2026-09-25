package main

import (
	"context"
	"errors"
	"fmt"
	"log"

	"grpc_server"
	"grpc_server/gen"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/speedtest"
	box "github.com/sagernet/sing-box"
	"github.com/sagernet/sing-box/experimental/v2rayapi"
	"github.com/sagernet/sing-box/option"
)

type server struct {
	grpc_server.BaseServer
}

var instance_cancel context.CancelFunc
var statsService *v2rayapi.StatsService

func (s *server) Start(ctx context.Context, in *gen.LoadConfigReq) (out *gen.ErrorResp, _ error) {
	var err error

	defer func() {
		out = &gen.ErrorResp{}
		if err != nil {
			out.Error = err.Error()
			instance = nil
			statsService = nil
		}
	}()

	if neko_common.Debug {
		log.Println("Start:", in.CoreConfig)
	}

	if instance != nil {
		err = errors.New("instance already started")
		return
	}

	instance, instance_cancel, err = createBox([]byte(in.CoreConfig))
	if err != nil {
		log.Println("LoadConfig error:", err)
		return
	}

	// V2Ray stats: 官方 1.11+ 移除了 Router().SetV2RayServer，
	// 改为通过 ConnectionTracker（v2rayapi.StatsService）挂载到 Router 统计流量。
	if in.StatsOutbounds != nil {
		statsService = v2rayapi.NewStatsService(option.V2RayStatsServiceOptions{
			Enabled:   true,
			Outbounds: in.StatsOutbounds,
		})
		if statsService != nil {
			instance.Router().AppendTracker(statsService)
		}
	}

	return
}

func (s *server) Stop(ctx context.Context, in *gen.EmptyReq) (out *gen.ErrorResp, _ error) {
	var err error

	defer func() {
		out = &gen.ErrorResp{}
		if err != nil {
			out.Error = err.Error()
		}
	}()

	if instance == nil {
		return
	}

	if instance_cancel != nil {
		instance_cancel()
		instance_cancel = nil
	}
	err = instance.Close()
	instance = nil
	statsService = nil

	return
}

func (s *server) Test(ctx context.Context, in *gen.TestReq) (out *gen.TestResp, _ error) {
	var err error
	out = &gen.TestResp{Ms: 0}

	defer func() {
		if err != nil {
			out.Error = err.Error()
		}
	}()

	if in.Mode == gen.TestMode_UrlTest {
		var i *box.Box
		var cancel context.CancelFunc
		if in.Config != nil {
			// Test instance
			i, cancel, err = createBoxNoLog([]byte(in.Config.CoreConfig))
			if i != nil {
				defer func() {
					if cancel != nil {
						cancel()
					}
					_ = i.Close()
				}()
			}
			if err != nil {
				return
			}
		} else {
			// Test running instance
			i = instance
			if i == nil {
				return
			}
		}
		// Latency
		out.Ms, err = speedtest.UrlTest(neko_common.CreateProxyHttpClient(i), in.Url, in.Timeout, speedtest.UrlTestStandard_RTT)
	} else if in.Mode == gen.TestMode_TcpPing {
		out.Ms, err = speedtest.TcpPing(in.Address, in.Timeout)
	} else if in.Mode == gen.TestMode_FullTest {
		i, cancel, err := createBoxNoLog([]byte(in.Config.CoreConfig))
		if i != nil {
			defer func() {
				if cancel != nil {
					cancel()
				}
				_ = i.Close()
			}()
		}
		if err != nil {
			return
		}
		return grpc_server.DoFullTest(ctx, in, i)
	}

	return
}

func (s *server) QueryStats(ctx context.Context, in *gen.QueryStatsReq) (out *gen.QueryStatsResp, _ error) {
	out = &gen.QueryStatsResp{}

	if statsService != nil {
		name := fmt.Sprintf("outbound>>>%s>>>traffic>>>%s", in.Tag, in.Direct)
		resp, err := statsService.GetStats(ctx, &v2rayapi.GetStatsRequest{Name: name})
		if err == nil && resp != nil && resp.Stat != nil {
			out.Traffic = resp.Stat.Value
		}
	}

	return
}

func (s *server) ListConnections(ctx context.Context, in *gen.EmptyReq) (*gen.ListConnectionsResp, error) {
	out := &gen.ListConnectionsResp{
		// TODO upstream api
	}
	return out, nil
}
