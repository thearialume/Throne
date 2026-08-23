#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <memory>

#include "include/database/entities/Group.h"

// How the per-group subscription strip below the profiles table is displayed.
enum class SubInfoBarMode : int {
    Smart  = 0, // show only when the server sent usage/expiry data
    Always = 1, // show for every subscription group, falling back to last-update only
    Never  = 2, // never show the strip
};

// A thin status line rendered in the main window's vertical splitter, between
// the profiles tab (above) and the logs/connections panel (below). It shows the
// current group's data-consumption progress bar, "used / total", the expiration
// date, and last-update time — the values the server already reports through
// the de-facto `subscription-userinfo` response header and that Throne persists
// in `Group::info`. The strip collapses entirely when there is nothing to show.
class SubInfoStatusBar : public QWidget {
public:
    explicit SubInfoStatusBar(QWidget *parent = nullptr);

    // Re-renders for `group`, honouring the SubInfoBarMode stored in settings
    // and hiding itself when the data (or the mode) says to. UI thread only.
    void Refresh(const std::shared_ptr<Configs::Group> &group);

private:
    QProgressBar *dataBar;
    QLabel *usageLabel;
    QLabel *expireLabel;
    QLabel *updateLabel;
};