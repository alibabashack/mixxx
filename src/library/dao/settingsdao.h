#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QVariant>

// All library-specific preferences go in the library settings table
class SettingsDAO final {
  public:
    explicit SettingsDAO(QSqlDatabase database)
            : m_database(std::move(database)) {
    }

    [[nodiscard]] QString getValue(
            const QString& name,
            QString defaultValue = QString()) const;
    [[nodiscard]] bool setValue(
            const QString& name,
            const QVariant& value) const;

    [[nodiscard]] const QSqlDatabase& database() const {
        return m_database;
    }

  private:
    const QSqlDatabase m_database;
};
