#pragma once

#include "fmt/AbstractBean.hpp"

namespace NekoGui_fmt {
    class SSHBean : public AbstractBean {
    public:
        QString username = "";
        QString password = "";

        explicit SSHBean() : AbstractBean(0) {
            _add(new configItem("username", &username, itemType::string));
            _add(new configItem("password", &password, itemType::string));
        };

        QString DisplayType() override { return "SSH"; };

        CoreObjOutboundBuildResult BuildCoreObjSingBox() override;

        bool TryParseLink(const QString &link);

        QString ToShareLink() override;
    };
} // namespace NekoGui_fmt
