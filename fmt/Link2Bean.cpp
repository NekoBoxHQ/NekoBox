#include "db/ProxyEntity.hpp"
#include "fmt/includes.h"

#include <QUrlQuery>

namespace NekoGui_fmt {

#define DECODE_V2RAY_N_1                                                                                                        \
    QString linkN = DecodeB64IfValid(SubStrBefore(SubStrAfter(link, "://"), "#"), QByteArray::Base64Option::Base64UrlEncoding); \
    if (linkN.isEmpty()) return false;                                                                                          \
    auto hasRemarks = link.contains("#");                                                                                       \
    if (hasRemarks) linkN += "#" + SubStrAfter(link, "#");                                                                      \
    auto url = QUrl("https://" + linkN);

    bool SocksHttpBean::TryParseLink(const QString &link) {
        QString link2 = link;
        // 修复密码中含 '#'（位于 '@' 之前）的链接：转义为 %23，避免被当作 fragment
        int at = link2.lastIndexOf('@');
        int hash = link2.indexOf('#');
        if (at > 0 && hash >= 0 && hash < at) {
            auto userinfo = link2.left(at);
            userinfo.replace("#", "%23");
            link2 = userinfo + link2.mid(at);
        }
        // 空认证 (://:@) 直接去掉，避免 QUrl 解析失败
        if (link2.contains("://:@")) link2.replace("://:@", "://");
        auto url = QUrl(link2);
        if (!url.isValid()) return false;
        auto query = GetQuery(url);

        if (link.startsWith("socks4")) socks_http_type = type_Socks4;
        if (link.startsWith("http")) socks_http_type = type_HTTP;
        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        username = url.userName();
        password = url.password();
        if (serverPort == -1) serverPort = socks_http_type == type_HTTP ? 443 : 1080;

        // v2rayN fmt
        if (password.isEmpty() && !username.isEmpty()) {
            QString n = DecodeB64IfValid(username);
            if (!n.isEmpty()) {
                username = SubStrBefore(n, ":");
                password = SubStrAfter(n, ":");
            }
        }

        stream->security = GetQueryValue(query, "security", "");
        stream->sni = GetQueryValue(query, "sni");
        if (link.startsWith("https")) stream->security = "tls";

        return !serverAddress.isEmpty();
    }

    bool TrojanVLESSBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        auto query = GetQuery(url);

        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        password = url.userName();
        if (serverPort == -1) serverPort = 443;

        // security

        auto type = GetQueryValue(query, "type", "tcp");
        if (type == "h2") {
            type = "http";
        }
        stream->network = type;

        if (proxy_type == proxy_Trojan) {
            stream->security = GetQueryValue(query, "security", "tls").replace("reality", "tls").replace("none", "");
        } else {
            stream->security = GetQueryValue(query, "security", "").replace("reality", "tls").replace("none", "");
        }
        auto sni1 = GetQueryValue(query, "sni");
        auto sni2 = GetQueryValue(query, "peer");
        if (!sni1.isEmpty()) stream->sni = sni1;
        if (!sni2.isEmpty()) stream->sni = sni2;
        stream->alpn = GetQueryValue(query, "alpn");
        if (!query.queryItemValue("allowInsecure").isEmpty()) stream->allow_insecure = true;
        stream->reality_pbk = GetQueryValue(query, "pbk", "");
        stream->reality_sid = GetQueryValue(query, "sid", "");
        stream->reality_spx = GetQueryValue(query, "spx", "");
        stream->utlsFingerprint = GetQueryValue(query, "fp", "");
        if (stream->utlsFingerprint.isEmpty()) {
            stream->utlsFingerprint = NekoGui::dataStore->utlsFingerprint;
        }

        // type
        if (stream->network == "ws") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "");
        } else if (stream->network == "http") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "").replace("|", ",");
        } else if (stream->network == "httpupgrade") {
            stream->path = GetQueryValue(query, "path", "");
            stream->host = GetQueryValue(query, "host", "");
        } else if (stream->network == "grpc") {
            stream->path = GetQueryValue(query, "serviceName", "");
        } else if (stream->network == "tcp") {
            if (GetQueryValue(query, "headerType") == "http") {
                stream->header_type = "http";
                stream->host = GetQueryValue(query, "host", "");
                stream->path = GetQueryValue(query, "path", "");
            }
        }

        // protocol
        if (proxy_type == proxy_VLESS) {
            flow = GetQueryValue(query, "flow", "");
        }

        return !(password.isEmpty() || serverAddress.isEmpty());
    }

    bool ShadowSocksBean::TryParseLink(const QString &link) {
        if (SubStrBefore(link, "#").contains("@")) {
            // SS
            auto url = QUrl(link);
            if (!url.isValid()) return false;

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();

            if (url.password().isEmpty()) {
                // traditional format
                auto method_password = DecodeB64IfValid(url.userName(), QByteArray::Base64Option::Base64UrlEncoding);
                if (method_password.isEmpty()) return false;
                method = SubStrBefore(method_password, ":");
                password = SubStrAfter(method_password, ":");
            } else {
                // 2022 format
                method = url.userName();
                password = url.password();
            }

            auto query = GetQuery(url);
            plugin = query.queryItemValue("plugin").replace("simple-obfs;", "obfs-local;");
        } else {
            // v2rayN
            DECODE_V2RAY_N_1

            if (hasRemarks) name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            method = url.userName();
            password = url.password();
        }
        return !(serverAddress.isEmpty() || method.isEmpty() || password.isEmpty());
    }

    bool VMessBean::TryParseLink(const QString &link) {
        // V2RayN Format
        auto linkN = DecodeB64IfValid(SubStrAfter(link, "vmess://"));
        if (!linkN.isEmpty()) {
            auto objN = QString2QJsonObject(linkN);
            if (objN.isEmpty()) return false;
            // REQUIRED
            uuid = objN["id"].toString();
            serverAddress = objN["add"].toString();
            serverPort = objN["port"].toVariant().toInt();
            // OPTIONAL
            name = objN["ps"].toString();
            aid = objN["aid"].toVariant().toInt();
            stream->host = objN["host"].toString();
            stream->path = objN["path"].toString();
            stream->sni = objN["sni"].toString();
            stream->header_type = objN["type"].toString();
            auto net = objN["net"].toString();
            if (!net.isEmpty()) {
                if (net == "h2") {
                    net = "http";
                }
                stream->network = net;
            }
            auto scy = objN["scy"].toString();
            if (!scy.isEmpty()) security = scy;
            // TLS (XTLS?)
            stream->security = objN["tls"].toString();
            // TODO quic & kcp
            return true;
        } else {
            // https://github.com/XTLS/Xray-core/discussions/716
            auto url = QUrl(link);
            if (!url.isValid()) return false;
            auto query = GetQuery(url);

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            uuid = url.userName();
            if (serverPort == -1) serverPort = 443;

            aid = 0; // “此分享标准仅针对 VMess AEAD 和 VLESS。”
            security = GetQueryValue(query, "encryption", "auto");

            // security
            auto type = GetQueryValue(query, "type", "tcp");
            if (type == "h2") {
                type = "http";
            }
            stream->network = type;
            stream->security = GetQueryValue(query, "security", "tls").replace("reality", "tls");
            auto sni1 = GetQueryValue(query, "sni");
            auto sni2 = GetQueryValue(query, "peer");
            if (!sni1.isEmpty()) stream->sni = sni1;
            if (!sni2.isEmpty()) stream->sni = sni2;
            if (!query.queryItemValue("allowInsecure").isEmpty()) stream->allow_insecure = true;
            stream->reality_pbk = GetQueryValue(query, "pbk", "");
            stream->reality_sid = GetQueryValue(query, "sid", "");
            stream->reality_spx = GetQueryValue(query, "spx", "");
            stream->utlsFingerprint = GetQueryValue(query, "fp", "");
            if (stream->utlsFingerprint.isEmpty()) {
                stream->utlsFingerprint = NekoGui::dataStore->utlsFingerprint;
            }

            // type
            if (stream->network == "ws") {
                stream->path = GetQueryValue(query, "path", "");
                stream->host = GetQueryValue(query, "host", "");
            } else if (stream->network == "http") {
                stream->path = GetQueryValue(query, "path", "");
                stream->host = GetQueryValue(query, "host", "").replace("|", ",");
            } else if (stream->network == "httpupgrade") {
                stream->path = GetQueryValue(query, "path", "");
                stream->host = GetQueryValue(query, "host", "");
            } else if (stream->network == "grpc") {
                stream->path = GetQueryValue(query, "serviceName", "");
            } else if (stream->network == "tcp") {
                if (GetQueryValue(query, "headerType") == "http") {
                    stream->header_type = "http";
                    stream->path = GetQueryValue(query, "path", "");
                    stream->host = GetQueryValue(query, "host", "");
                }
            }
            return !(uuid.isEmpty() || serverAddress.isEmpty());
        }

        return false;
    }

    bool NaiveBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;

        protocol = url.scheme().replace("naive+", "");
        if (protocol != "https" && protocol != "quic") return false;

        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        username = url.userName();
        password = url.password();

        return !(username.isEmpty() || password.isEmpty() || serverAddress.isEmpty());
    }

    bool QUICBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        auto query = QUrlQuery(url.query());
        if (url.host().isEmpty() || url.port() == -1) return false;

        if (url.scheme() == "tuic") {
            // by daeuniverse
            // https://github.com/daeuniverse/dae/discussions/182

            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            if (serverPort == -1) serverPort = 443;
            serverPort = url.port();

            uuid = url.userName();
            password = url.password();

            congestionControl = query.queryItemValue("congestion_control");
            alpn = query.queryItemValue("alpn");
            sni = query.queryItemValue("sni");
            udpRelayMode = query.queryItemValue("udp_relay_mode");
            allowInsecure = query.queryItemValue("allow_insecure") == "1";
            disableSni = query.queryItemValue("disable_sni") == "1";
        } else if (QStringList{"hy", "hysteria"}.contains(url.scheme())) {
            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            password = query.queryItemValue("auth");
            if (password.isEmpty()) password = url.userName();
            obfs = query.queryItemValue("obfs");
            sni = query.queryItemValue("peer");
            if (sni.isEmpty()) sni = query.queryItemValue("sni");
            allowInsecure = QStringList{"1", "true"}.contains(query.queryItemValue("insecure"));
            uploadMbps = query.queryItemValue("up").toInt();
            downloadMbps = query.queryItemValue("down").toInt();
        } else if (QStringList{"hy2", "hysteria2"}.contains(url.scheme())) {
            name = url.fragment(QUrl::FullyDecoded);
            serverAddress = url.host();
            serverPort = url.port();
            hopPort = query.queryItemValue("mport");
            obfsPassword = query.queryItemValue("obfs-password");
            allowInsecure = QStringList{"1", "true"}.contains(query.queryItemValue("insecure"));

            if (url.password().isEmpty()) {
                password = url.userName();
            } else {
                password = url.userName() + ":" + url.password();
            }

            sni = query.queryItemValue("sni");
        }

        return true;
    }

    // WireGuard
    bool WireGuardBean::TryParseLink(const QString &link) {
        auto linkN = DecodeB64IfValid(SubStrAfter(link, "wireguard://").split("#")[0], QByteArray::Base64UrlEncoding);
        if (link.contains("#")) name = SubStrAfter(link, "#");
        if (!linkN.isEmpty()) {
            auto objN = QString2QJsonObject(linkN);
            if (objN.isEmpty()) return false;
            serverAddress = objN["server"].toString();
            serverPort = objN["port"].toVariant().toInt();
            privateKey = objN["private_key"].toString();
            publicKey = objN["public_key"].toString();
            presharedKey = objN["preshared_key"].toString();
            auto m = objN["mtu"].toVariant().toInt();
            if (m > 0) mtu = m;
            if (objN.contains("reserved")) reserved = objN["reserved"].toString();
        } else {
            // wireguard://host:port?public_key=..&private_key=..#name
            auto url = QUrl(link);
            if (!url.isValid()) return false;
            auto query = QUrlQuery(url.query());
            serverAddress = url.host();
            serverPort = url.port();
            if (serverPort == -1) serverPort = 51820;
            privateKey = query.queryItemValue("private_key");
            publicKey = query.queryItemValue("public_key");
            presharedKey = query.queryItemValue("preshared_key");
            auto m = query.queryItemValue("mtu").toInt();
            if (m > 0) mtu = m;
            reserved = query.queryItemValue("reserved");
        }
        return !(serverAddress.isEmpty() || publicKey.isEmpty());
    }

    // SSH
    bool SSHBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        if (serverPort == -1) serverPort = 22;
        username = url.userName();
        password = url.password();
        return !(serverAddress.isEmpty() || username.isEmpty());
    }

    // AnyTLS
    bool AnyTLSBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        auto query = QUrlQuery(url.query());
        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        if (serverPort == -1) serverPort = 443;
        password = url.userName();
        sni = query.queryItemValue("sni");
        auto ins = query.queryItemValue("allowInsecure");
        if (ins == "1" || ins == "true") allowInsecure = true;
        return !(serverAddress.isEmpty() || password.isEmpty());
    }

    // Brook
    bool BrookBean::TryParseLink(const QString &link) {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        auto query = QUrlQuery(url.query());
        name = url.fragment(QUrl::FullyDecoded);
        serverAddress = url.host();
        serverPort = url.port();
        if (serverPort == -1) serverPort = 443;
        password = url.userName();
        auto proto = query.queryItemValue("protocol");
        if (!proto.isEmpty()) protocol = proto;
        return !(serverAddress.isEmpty() || password.isEmpty());
    }

} // namespace NekoGui_fmt