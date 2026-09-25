package main

import (
	"context"
	"net"
	"net/http"

	"github.com/matsuridayo/libneko/neko_common"
	"github.com/matsuridayo/libneko/neko_log"
	box "github.com/sagernet/sing-box"
	M "github.com/sagernet/sing/common/metadata"
)

var instance *box.Box

func setupCore() {
	neko_log.SetupLog(50*1024, "./neko.log")
	//
	neko_common.GetCurrentInstance = func() interface{} {
		return instance
	}
	neko_common.DialContext = func(ctx context.Context, specifiedInstance interface{}, network, addr string) (net.Conn, error) {
		i := pickInstance(specifiedInstance)
		if i == nil {
			return neko_common.DialContextSystem(ctx, network, addr)
		}
		outbound := i.Outbound().Default()
		if outbound == nil {
			return neko_common.DialContextSystem(ctx, network, addr)
		}
		return outbound.DialContext(ctx, network, M.ParseSocksaddr(addr))
	}
	neko_common.DialUDP = func(ctx context.Context, specifiedInstance interface{}) (net.PacketConn, error) {
		i := pickInstance(specifiedInstance)
		if i == nil {
			return neko_common.DialUDPSystem(ctx)
		}
		outbound := i.Outbound().Default()
		if outbound == nil {
			return neko_common.DialUDPSystem(ctx)
		}
		return outbound.ListenPacket(ctx, M.Socksaddr{})
	}
	neko_common.CreateProxyHttpClient = func(specifiedInstance interface{}) *http.Client {
		i := pickInstance(specifiedInstance)
		tr := &http.Transport{}
		if i != nil {
			tr.DialContext = func(ctx context.Context, network, addr string) (net.Conn, error) {
				outbound := i.Outbound().Default()
				if outbound == nil {
					return neko_common.DialContextSystem(ctx, network, addr)
				}
				return outbound.DialContext(ctx, network, M.ParseSocksaddr(addr))
			}
		}
		return &http.Client{Transport: tr}
	}
}

func pickInstance(specifiedInstance interface{}) *box.Box {
	if i, ok := specifiedInstance.(*box.Box); ok {
		return i
	}
	return instance
}
