#include "include/ui/widget/SubInfoStatusBar.h"

#include "include/global/Configs.hpp"
#include "include/global/Utils.hpp"

#include <QHBoxLayout>
#include <QSizePolicy>
#include <QDateTime>
#include <QColor>

SubInfoStatusBar::SubInfoStatusBar(QWidget *parent) : QWidget(parent) {
    dataBar = new QProgressBar(this);
    dataBar->setRange(0, 100);
    dataBar->setValue(0);
    dataBar->setTextVisible(false);
    dataBar->setFixedHeight(6);
    dataBar->setMinimumWidth(120);
    dataBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    usageLabel = new QLabel(this);
    expireLabel = new QLabel(this);
    updateLabel = new QLabel(this);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(10);
    layout->addWidget(dataBar);
    layout->addWidget(usageLabel);
    layout->addWidget(expireLabel);
    layout->addWidget(updateLabel);
    layout->addStretch(1);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setVisible(false);
}

void SubInfoStatusBar::Refresh(const std::shared_ptr<Configs::Group> &group) {
    auto &settings = Configs::dataManager->settingsRepo;
    const auto mode = static_cast<SubInfoBarMode>(settings->sub_usage_bar);

    // Collapse everything first; hiding the strip lets QSplitter reclaim the band.
    dataBar->hide();
    usageLabel->clear();
    expireLabel->clear();
    expireLabel->setStyleSheet(QString()); // drop an "expired" red tint
    updateLabel->clear();
    setVisible(false);

    if (mode == SubInfoBarMode::Never) return;
    // A basic (non-subscription) group carries none of this data.
    if (group == nullptr || group->url.isEmpty()) return;

    const bool hasData = !group->info.trimmed().isEmpty();
    if (mode == SubInfoBarMode::Smart && !hasData) return;

    const auto sub = ParseSubUserInfo(group->info);

    bool anything = false;

    // Consumption bar + "<used> / <total>". Unlimited plans hide the bar and
    // print only the ratio, so there is never a nonsense full/empty bar.
    if (sub.hasQuota) {
        anything = true;
        if (sub.hasTotal && sub.total > 0) {
            int pct = int(double(sub.used) / double(sub.total) * 100.0);
            if (pct < 0) pct = 0;
            else if (pct > 100) pct = 100;
            const QColor color =
                pct >= 100 ? QColor(0xe7, 0x4c, 0x3c)  // exhausted
                : pct >= 80  ? QColor(0xf3, 0x9c, 0x12) // almost out
                :              QColor(0x2e, 0xcc, 0x71); // fine
            dataBar->setValue(pct);
            dataBar->setStyleSheet(
                QStringLiteral("QProgressBar::chunk { background-color: %1; }")
                    .arg(color.name()));
            dataBar->show();
        }
        usageLabel->setText(QStringLiteral("%1 / %2")
            .arg(ReadableSize(sub.used),
                 sub.hasTotal ? ReadableSize(sub.total) : QString::fromUtf8("\u221E")));
    }

    // Expiration date, absolute like Happ (never a relative "N days left").
    if (sub.hasExpire && sub.expire > 0) {
        anything = true;
        expireLabel->setText(tr("Expires: %1").arg(DisplayTime(sub.expire, QLocale::ShortFormat)));
        // Tint it red once the subscription is in the past.
        if (QDateTime::currentSecsSinceEpoch() > sub.expire)
            expireLabel->setStyleSheet(
                QStringLiteral("color: %1;").arg(QColor(0xe7, 0x4c, 0x3c).name()));
    }

    // Last update, formatted exactly as on the manage-groups cards.
    if (group->sub_last_update != 0) {
        anything = true;
        updateLabel->setText(tr("Last update: %1")
            .arg(DisplayTime(group->sub_last_update, QLocale::ShortFormat)));
    }

    setVisible(anything || mode == SubInfoBarMode::Always);
}