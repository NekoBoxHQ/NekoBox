#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoGui_fmt {
    class WireGuardBean : public AbstractBean {
    public:
        QString privateKey = "";
        QString publicKey = "";
        QString presharedKey = "";
        int mtu = 1420;
        QString reserved = "";

        explicit WireGuardBean() : AbstractBean(0) {
            _add(new configItem("privateKey", &privateKey, itemType::string));
            _add(new configItem("publicKey", &publicKey, itemType::string));
            _add(new configItem("presharedKey", &presharedKey, itemType::string));
            _add(new configItem("mtu", &mtu, itemType::integer));
            _add(new configItem("reserved", &reserved, itemType::string));
        };

        QString DisplayType() override { return "WireGuard"; };

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoGui_fmt
