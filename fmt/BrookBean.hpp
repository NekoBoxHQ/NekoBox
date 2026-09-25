#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoGui_fmt {
    class BrookBean : public AbstractBean {
    public:
        QString password = "";
        QString protocol = "ws";

        explicit BrookBean() : AbstractBean(0) {
            _add(new configItem("password", &password, itemType::string));
            _add(new configItem("protocol", &protocol, itemType::string));
        };

        QString DisplayType() override { return "Brook"; };

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoGui_fmt
