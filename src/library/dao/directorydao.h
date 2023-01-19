#pragma once

#include <QList>

#include "library/dao/dao.h"
#include "library/relocatedtrack.h"
#include "util/fileinfo.h"

class DirectoryDAO : public DAO {
  public:
    ~DirectoryDAO() override = default;

    [[nodiscard]] QList<mixxx::FileInfo> loadAllDirectories(
            bool skipInvalidOrMissing = false) const;

    enum class AddResult {
        Ok,
        AlreadyWatching,
        InvalidOrMissingDirectory,
        SqlError,
    };
    [[nodiscard]] AddResult addDirectory(const mixxx::FileInfo& newDir) const;

    enum class RemoveResult {
        Ok,
        NotFound,
        SqlError,
    };
    [[nodiscard]] RemoveResult removeDirectory(const mixxx::FileInfo& oldDir) const;

    // TODO: Move this function out of the DAO
    [[nodiscard]] QList<RelocatedTrack> relocateDirectory(
            const QString& oldDirectory,
            const QString& newDirectory) const;
};
