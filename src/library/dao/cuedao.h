#pragma once

#include <QSqlDatabase>

#include "library/dao/dao.h"
#include "track/cue.h"
#include "track/trackid.h"

#define CUE_TABLE "cues"

class Cue;

class CueDAO : public DAO {
  public:
    ~CueDAO() override = default;

    [[nodiscard]] QList<CuePointer> getCuesForTrack(TrackId trackId) const;

    void saveTrackCues(TrackId trackId, const QList<CuePointer>& cueList) const;
    [[nodiscard]] bool deleteCuesForTrack(TrackId trackId) const;
    [[nodiscard]] bool deleteCuesForTracks(const QList<TrackId>& trackIds) const;

  private:
    bool saveCue(TrackId trackId, Cue* pCue) const;
    bool deleteCue(Cue* pCue) const;
};
