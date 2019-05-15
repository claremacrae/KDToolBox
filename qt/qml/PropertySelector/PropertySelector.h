/*
  This file is part of KDToolBox.

  Copyright (C) 2018-2019 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: André Somers <andre.somers@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PROPERTYSWITCH_H
#define PROPERTYSWITCH_H

#include <QQuickItem>
#include <bitset>
#include <private/qqmlcustomparser_p.h>
#include <QQmlParserStatus>


namespace cpp {
namespace qmltypes {

class PropertySelectorCustomParser;

class PropertySelector : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QObject* target READ target WRITE setTarget NOTIFY targetChanged)
    Q_INTERFACES(QQmlParserStatus)

public:
    ///@brief the maximum number of conditions supported
    static const int MaxConditionCount = 32;

    PropertySelector(QQuickItem *parent = nullptr);

    QObject *target() const;
    Q_SLOT void setTarget(QObject *target);
    Q_SIGNAL void targetChanged(QObject *target);

    // QQmlParserStatus interface
    void componentComplete() override;
    void classBegin() override;

private: //types
    using ConditionBits = std::bitset<MaxConditionCount>;
    struct Rule {
        Rule() {}
        Rule(QStringList conditions, QString property, QVariant simpleValue, QV4::CompiledData::Location location)
            :conditions(conditions), property(property), simpleValue(simpleValue), location(location), binding(nullptr)
        {}
        Rule(QStringList conditions, QString property, const QV4::CompiledData::Binding *binding, QV4::CompiledData::Location location)
            :conditions(conditions), property(property), location(location), binding(binding), id(binding->value.compiledScriptIndex)
        {}

        QStringList conditions;
        QString property;
        QVariant simpleValue;

        QV4::CompiledData::Location location;
        const QV4::CompiledData::Binding * binding;
        QQmlBinding::Identifier id = QQmlBinding::Invalid;

        ConditionBits conditionBits;

        bool operator<(const Rule& other);
    };
    struct PropertyRules {
        QQmlProperty property;
        QVector<Rule>::const_iterator begin;
        QVector<Rule>::const_iterator end;
        QVector<Rule>::const_iterator currentRule;
        QQmlBinding *currentBinding;
    };

private: //methods
    void connectChangeSignals();
    Q_SLOT void apply();
    void applyRule(PropertyRules &propertyRules, QVector<Rule>::const_iterator rule);
    void prepareRules();
    ConditionBits bitmaskFromConditionList(const QStringList& conditions) const;

private: //members
    QObject *m_target = nullptr;
    QStringList m_conditions;
    QVector<Rule> m_ruleList;
    QQmlRefPointer<QV4::CompiledData::CompilationUnit> m_compilationUnit;
    QVector<PropertyRules> m_properties;

    friend class PropertySelectorCustomParser;
};


class PropertySelectorCustomParser: public QQmlCustomParser
{
public:
    // QQmlCustomParser interface
    void verifyBindings(const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compiledUnit, const QList<const QV4::CompiledData::Binding *> &bindings) override Q_DECL_FINAL;
    void applyBindings(QObject *, const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compiledUnit, const QList<const QV4::CompiledData::Binding *> &bindings) override Q_DECL_FINAL;

    void parseBinding(PropertySelector *selector, QStringList conditions, const QV4::CompiledData::Binding *binding);
};

}}
#endif // PROPERTYSWITCH_H
