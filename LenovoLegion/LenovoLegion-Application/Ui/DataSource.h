// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QObject>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QVariant>

#include <google/protobuf/message.h>

#include <map>
#include <memory>

namespace LenovoLegionGui {

class DataProvider;
class FieldModel;

/*
 * Message of one data type of the daemon for the generic GUI (UiDesign.md).
 * Message type is taken from the (legion.ui.message) option of the DataType value, fields are accessed by path
 * ("touch_pad.current") through protobuf reflection. Changes of the user are kept in a separate message,
 * apply() sends only them.
 */
class DataSource : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString dataType  READ dataType  WRITE setDataType  NOTIFY dataTypeChanged)
    Q_PROPERTY(bool    autoApply READ autoApply WRITE setAutoApply NOTIFY autoApplyChanged)

    /*
     * Periodic reload in ms (monitoring), 0 = only on demand and on daemon notifications.
     * The request runs in a worker thread, the GUI is not blocked (the same as HWMonitoring of the widget pages).
     */
    Q_PROPERTY(int     interval  READ interval  WRITE setInterval  NOTIFY intervalChanged)
    Q_PROPERTY(bool    dirty     READ dirty                        NOTIFY messageChanged)
    Q_PROPERTY(bool    valid     READ valid                        NOTIFY messageChanged)

    /*
     * The last request for the data succeeded (the daemon provides the data type, e.g. the Lenovo driver is loaded)
     */
    Q_PROPERTY(bool    available READ available                    NOTIFY messageChanged)

    /*
     * (legion.ui.platform) of the data type: "generic" or "lenovo" (Lenovo Legion kernel driver needed)
     */
    Q_PROPERTY(QString platform  READ platform                     NOTIFY dataTypeChanged)
    Q_PROPERTY(QString lastError READ lastError                    NOTIFY lastErrorChanged)

    /*
     * Incremented on every change of the message, bindings with value() depend on it ("source.revision, source.value(...)")
     */
    Q_PROPERTY(int     revision  READ revision                     NOTIFY messageChanged)

    /*
     * Leaf fields in the order of the .proto, without HIDDEN and repeated fields (AutoSection)
     */
    Q_PROPERTY(QStringList fieldPaths READ fieldPaths              NOTIFY dataTypeChanged)

public:

    explicit DataSource(QObject* parent = nullptr);
    ~DataSource() override;

    /*
     * Connection to the daemon for all data sources, set by the page host
     */
    static void setDataProvider(DataProvider* dataProvider);

    /*
     * Reload all data sources (daemon notification), changes of the user are kept
     */
    static void reloadAll();

    QString dataType() const;
    void    setDataType(const QString& dataType);

    bool    autoApply() const;
    void    setAutoApply(bool autoApply);

    int     interval() const;
    void    setInterval(int interval);

    bool    dirty() const;
    bool    valid() const;
    bool    available() const;
    QString platform() const;
    QString lastError() const;
    int     revision() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void apply();
    Q_INVOKABLE void cancel();

    /*
     * Preset of the layout ({path: displayed value}): changes of the user are discarded, the values are sent immediately
     * (the same as the Default button of the widget pages)
     */
    Q_INVOKABLE void applyValues(const QVariantMap& values);

    /*
     * Preset of the layout ({path: displayed value}) filled in as changes of the user, sent by Apply
     * (the same as the preset combo boxes of the widget offsets page)
     */
    Q_INVOKABLE void setValues(const QVariantMap& values);

    /*
     * Changes of many fields (all threads of the frequency grid, all points of the fan curve): one notification
     * (messageChanged) and one autoApply at the end instead of one for every field. Calls can be nested.
     */
    Q_INVOKABLE void beginChanges();
    Q_INVOKABLE void endChanges();

    /*
     * Command of the data type ((legion.ui.command), e.g. PROFILES: ProfileCommand {action: "LOAD", name: ...}),
     * enum values by name or number. The response (message of the data type) replaces the data. True if sent.
     */
    Q_INVOKABLE bool sendCommand(const QVariantMap& values);

    /*
     * All data sources read the data again (after a command which changes everything, e.g. a loaded profile)
     */
    Q_INVOKABLE void reloadAllSources();

    QStringList fieldPaths() const;

    /*
     * Model of the field, created once per path (owned by the data source)
     */
    Q_INVOKABLE LenovoLegionGui::FieldModel* field(const QString& path);

    /*
     * Effective value of the field (change of the user or the value from the daemon)
     */
    Q_INVOKABLE QVariant value(const QString& path) const;

    /*
     * Field was sent by the daemon (explicit presence), not sent = not supported
     */
    bool present(const QString& path) const;

    /*
     * Value from the daemon, without the change of the user
     */
    Q_INVOKABLE QVariant daemonValue(const QString& path) const;

    /*
     * Change of the user, sent by apply() (immediately with autoApply).
     * Value equal to the value from the daemon removes the change, force writes it anyway.
     */
    void setValue(const QString& path,const QVariant& value,bool force = false);

    /*
     * Remove the change of the user of the field
     */
    void clearChange(const QString& path);

    /*
     * Field has a change of the user which is not applied yet
     */
    bool modified(const QString& path) const;

    /*
     * Values of a repeated string, enum or number field (choices_from, default fan curves),
     * a string field is split by white spaces (available_governors "performance powersave")
     */
    Q_INVOKABLE QVariantList repeatedValues(const QString& path) const;

    /*
     * Number of elements of a repeated field in the data from the daemon (CPUs, ranges of the CPU topology)
     */
    Q_INVOKABLE int count(const QString& path) const;

    const google::protobuf::Descriptor* descriptor() const;

signals:

    void dataTypeChanged();
    void autoApplyChanged();
    void intervalChanged();
    void messageChanged();
    void lastErrorChanged();

private:

    void setLastError(const QString& error);

    /*
     * Periodic reload in a worker thread, the message is updated in the GUI thread
     */
    void reloadAsync();

    /*
     * Data from the daemon -> message, changes of the user are kept
     */
    void parse(const QByteArray& data);

private:

    static DataProvider*      s_dataProvider;
    static QSet<DataSource*>  s_dataSources;

    QString                                      m_dataType;
    int                                          m_dataTypeValue;
    bool                                         m_autoApply;
    QTimer*                                      m_timer;
    bool                                         m_reloading = false;    // periodic request is running
    QString                                      m_lastError;
    bool                                         m_available = false;
    int                                          m_batchDepth = 0;       // beginChanges() without endChanges()
    bool                                         m_batchChanged = false; // a field changed during the batch
    QString                                      m_platform  = "generic";
    int                                          m_revision = 0;

    std::unique_ptr<google::protobuf::Message>   m_message;    // state from the daemon
    std::unique_ptr<google::protobuf::Message>   m_changes;    // changed fields only

    std::map<QString,FieldModel*>                m_fields;
};

}
