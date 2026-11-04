// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QList>
#include <QObject>
#include <QPointer>

namespace LenovoLegionGui {

class DataSource;

/*
 * Data sources of a page (all DataSource objects in the page) and their state for the tab of the page:
 * the page is available if the daemon provides data of any of them, the platform is "lenovo" if any of them
 * needs the Lenovo Legion kernel driver ((legion.ui.platform) in MessageRegistry.proto)
 */
class PageSources : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* page      READ page      WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(bool     available READ available               NOTIFY stateChanged)
    Q_PROPERTY(QString  platform  READ platform                NOTIFY stateChanged)

    /*
     * Why the page is not available, empty if it is available
     */
    Q_PROPERTY(QString  reason    READ reason                  NOTIFY stateChanged)

    /*
     * What controls the page (tool tip of the tab): the Lenovo Legion kernel driver or standard interfaces
     */
    Q_PROPERTY(QString  description READ description           NOTIFY stateChanged)

public:

    explicit PageSources(QObject* parent = nullptr);

    QObject* page() const;
    void     setPage(QObject* page);

    bool     available() const;
    QString  platform() const;
    QString  reason() const;
    QString  description() const;

signals:

    void pageChanged();
    void stateChanged();

private:

    void scan();

    QPointer<QObject>             m_page;
    QList<QPointer<DataSource>>   m_sources;
};

}
