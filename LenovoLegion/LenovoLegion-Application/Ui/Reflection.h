// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QStringList>
#include <QVariant>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <vector>

namespace LenovoLegionGui::Reflection {

/*
 * One component of a path: field, value of a map field by key ("mode_descriptor_map[255]")
 * or element of a repeated message field by index ("cpus[3]")
 */
struct PathStep
{
    const google::protobuf::FieldDescriptor* field  = nullptr;
    bool                                     mapKey = false;
    QString                                  key;
    bool                                     index  = false;
    int                                      position = 0;
};

/*
 * Field of a message by path ("touch_pad.current", "cpu_ltp_limit.mode_descriptor_map[255].min_value", "cpus[3].cpu_online"),
 * the last step is the leaf. Values of maps can be read only, elements of repeated fields are created by write
 * (empty elements before them, the daemon skips empty elements).
 */
using FieldPath = std::vector<PathStep>;

/*
 * Steps of the path, empty if the path is not valid (repeated fields are allowed only as the leaf, maps only with a key)
 */
FieldPath resolve(const google::protobuf::Descriptor* descriptor,const QString& path);

/*
 * Descriptors of the fields of the path (options of the fields)
 */
std::vector<const google::protobuf::FieldDescriptor*> descriptors(const FieldPath& path);

/*
 * Leaf fields in the order of the .proto, without HIDDEN, repeated and map fields
 */
void leafPaths(const google::protobuf::Descriptor* descriptor,const QString& prefix,QStringList& paths);

/*
 * The leaf and all its parents are present (explicit presence, map entry with the key), repeated leaf is not empty
 */
bool has(const google::protobuf::Message& message,const FieldPath& path);

/*
 * Value of a singular leaf (default value if not present): bool, qlonglong/qulonglong, double, QString, enum as int
 */
QVariant read(const google::protobuf::Message& message,const FieldPath& path);

/*
 * Set a singular leaf, parents are created (not through maps)
 */
void write(google::protobuf::Message& message,const FieldPath& path,const QVariant& value);

/*
 * Clear a singular leaf, parents which become empty are cleared too (not through maps)
 */
void clear(google::protobuf::Message& message,const FieldPath& path);

/*
 * Values of a repeated string (QString), enum (number) or number leaf
 */
QVariantList repeatedValues(const google::protobuf::Message& message,const FieldPath& path);

/*
 * Number of elements of a repeated leaf (0 if a parent is not present)
 */
int count(const google::protobuf::Message& message,const FieldPath& path);

/*
 * Value of a field as QVariant for QML: a message is a map of its fields (by name), a repeated field a list,
 * an enum its number
 */
QVariant toVariant(const google::protobuf::Message& message,const google::protobuf::FieldDescriptor* field);
QVariantMap toVariant(const google::protobuf::Message& message);

/*
 * Fields of a message from a map by the field names: a map for a message field, a list for a repeated field (maps
 * for messages, values for scalars), enum by number or name. Unknown fields are returned in unknown.
 */
void fromVariant(google::protobuf::Message& message,const QVariantMap& values,QStringList* unknown = nullptr);

}
