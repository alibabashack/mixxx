#pragma once

#include <QList>
#include <QObject>
#include <QSet>

#include "library/trackset/crate/cratesummary.h"
#include "track/trackid.h"
#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlstorage.h"
#include "util/db/sqlsubselectmode.h"

class CrateQueryFields {
  public:
    CrateQueryFields() {
    }
    explicit CrateQueryFields(const FwdSqlQuery& query);
    virtual ~CrateQueryFields() = default;

    [[nodiscard]] CrateId getId(const FwdSqlQuery& query) const {
        return CrateId(query.fieldValue(m_iId));
    }
    [[nodiscard]] QString getName(const FwdSqlQuery& query) const {
        return query.fieldValue(m_iName).toString();
    }
    [[nodiscard]] bool isLocked(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iLocked);
    }
    [[nodiscard]] bool isAutoDjSource(const FwdSqlQuery& query) const {
        return query.fieldValueBoolean(m_iAutoDjSource);
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            Crate* pCrate) const;

  private:
    DbFieldIndex m_iId;
    DbFieldIndex m_iName;
    DbFieldIndex m_iLocked;
    DbFieldIndex m_iAutoDjSource;
};

class CrateSelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateSelectResult(CrateSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSelectResult() override = default;

    bool populateNext(Crate* pCrate) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrate);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class CrateStorage;
    CrateSelectResult() = default;
    explicit CrateSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateQueryFields m_queryFields;
};

class CrateSummaryQueryFields : public CrateQueryFields {
  public:
    CrateSummaryQueryFields() = default;
    explicit CrateSummaryQueryFields(const FwdSqlQuery& query);
    ~CrateSummaryQueryFields() override = default;

    [[nodiscard]] uint getTrackCount(const FwdSqlQuery& query) const {
        QVariant varTrackCount = query.fieldValue(m_iTrackCount);
        if (varTrackCount.isNull()) {
            return 0; // crate is empty
        } else {
            return varTrackCount.toUInt();
        }
    }
    [[nodiscard]] double getTrackDuration(const FwdSqlQuery& query) const {
        QVariant varTrackDuration = query.fieldValue(m_iTrackDuration);
        if (varTrackDuration.isNull()) {
            return 0.0; // crate is empty
        } else {
            return varTrackDuration.toDouble();
        }
    }

    void populateFromQuery(
            const FwdSqlQuery& query,
            CrateSummary* pCrateSummary) const;

  private:
    DbFieldIndex m_iTrackCount;
    DbFieldIndex m_iTrackDuration;
};

class CrateSummarySelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateSummarySelectResult(CrateSummarySelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateSummarySelectResult() override = default;

    bool populateNext(CrateSummary* pCrateSummary) {
        if (next()) {
            m_queryFields.populateFromQuery(query(), pCrateSummary);
            return true;
        } else {
            return false;
        }
    }

  private:
    friend class CrateStorage;
    CrateSummarySelectResult() = default;
    explicit CrateSummarySelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateSummaryQueryFields m_queryFields;
};

class CrateTrackQueryFields {
  public:
    CrateTrackQueryFields() = default;
    explicit CrateTrackQueryFields(const FwdSqlQuery& query);
    virtual ~CrateTrackQueryFields() = default;

    [[nodiscard]] CrateId crateId(const FwdSqlQuery& query) const {
        return CrateId(query.fieldValue(m_iCrateId));
    }
    [[nodiscard]] TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iCrateId;
    DbFieldIndex m_iTrackId;
};

class TrackQueryFields {
  public:
    TrackQueryFields() = default;
    explicit TrackQueryFields(const FwdSqlQuery& query);
    virtual ~TrackQueryFields() = default;

    [[nodiscard]] TrackId trackId(const FwdSqlQuery& query) const {
        return TrackId(query.fieldValue(m_iTrackId));
    }

  private:
    DbFieldIndex m_iTrackId;
};

class CrateTrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    CrateTrackSelectResult(CrateTrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~CrateTrackSelectResult() override = default;

    [[nodiscard]] CrateId crateId() const {
        return m_queryFields.crateId(query());
    }
    [[nodiscard]] TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class CrateStorage;
    CrateTrackSelectResult() = default;
    explicit CrateTrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    CrateTrackQueryFields m_queryFields;
};

class TrackSelectResult : public FwdSqlQuerySelectResult {
  public:
    TrackSelectResult(TrackSelectResult&& other)
            : FwdSqlQuerySelectResult(std::move(other)),
              m_queryFields(std::move(other.m_queryFields)) {
    }
    ~TrackSelectResult() override = default;

    [[nodiscard]] TrackId trackId() const {
        return m_queryFields.trackId(query());
    }

  private:
    friend class CrateStorage;
    TrackSelectResult() = default;
    explicit TrackSelectResult(FwdSqlQuery&& query)
            : FwdSqlQuerySelectResult(std::move(query)),
              m_queryFields(FwdSqlQuerySelectResult::query()) {
    }

    TrackQueryFields m_queryFields;
};

class CrateStorage : public virtual /*implements*/ SqlStorage {
  public:
    CrateStorage() = default;
    ~CrateStorage() override = default;

    void repairDatabase(
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    /////////////////////////////////////////////////////////////////////////
    // Crate write operations (transactional, non-const)
    // Only invoked by TrackCollection!
    //
    // Naming conventions:
    //  on<present participle>...()
    //    - Invoked within active transaction
    //    - May fail
    //    - Performs only database modifications that are either committed
    //      or implicitly reverted on rollback
    //  after<present participle>...()
    //    - Invoked after preceding transaction has been committed (see above)
    //    - Must not fail
    //    - Typical use case: Update internal caches and compute change set
    //      for notifications
    /////////////////////////////////////////////////////////////////////////

    bool onInsertingCrate(
            const Crate& crate,
            CrateId* pCrateId = nullptr);

    bool onUpdatingCrate(
            const Crate& crate);

    bool onDeletingCrate(
            CrateId crateId);

    bool onAddingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onRemovingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // Crate read operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    [[nodiscard]] uint countCrates() const;

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateById(
            CrateId id,
            Crate* pCrate = nullptr) const;
    bool readCrateByName(
            const QString& name,
            Crate* pCrate = nullptr) const;

    // The following list results are ordered by crate name:
    //  - case-insensitive
    //  - locale-aware
    [[nodiscard]] CrateSelectResult selectCrates() const; // all crates
    [[nodiscard]] CrateSelectResult selectCratesByIds(    // subset of crates
            const QString& subselectForCrateIds,
            SqlSubselectMode subselectMode) const;

    // TODO(XXX): Move this function into the AutoDJ component after
    // fixing various database design flaws in AutoDJ itself (see also:
    // crateschema.h). AutoDJ should use the function selectCratesByIds()
    // from this class for the actual implementation.
    // This refactoring should be deferred until consensus on the
    // redesign of the AutoDJ feature has been reached. The main
    // ideas of the new design should be documented for verification
    // before starting to code.
    [[nodiscard]] CrateSelectResult selectAutoDjCrates(bool autoDjSource = true) const;

    // Crate content, i.e. the crate's tracks referenced by id
    [[nodiscard]] uint countCrateTracks(CrateId crateId) const;

    // Format a subselect query for the tracks contained in crate.
    static QString formatSubselectQueryForCrateTrackIds(
            CrateId crateId); // no db access

    [[nodiscard]] QString formatQueryForTrackIdsByCrateNameLike(
            const QString& crateNameLike) const;      // no db access
    static QString formatQueryForTrackIdsWithCrate(); // no db access
    // Select the track ids of a crate or the crate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    [[nodiscard]] CrateTrackSelectResult selectCrateTracksSorted(
            CrateId crateId) const;
    [[nodiscard]] CrateTrackSelectResult selectTrackCratesSorted(
            TrackId trackId) const;
    [[nodiscard]] CrateSummarySelectResult selectCratesWithTrackCount(
            const QList<TrackId>& trackIds) const;
    [[nodiscard]] CrateTrackSelectResult selectTracksSortedByCrateNameLike(
            const QString& crateNameLike) const;
    [[nodiscard]] TrackSelectResult selectAllTracksSorted() const;

    // Returns the set of crate ids for crates that contain any of the
    // provided track ids.
    [[nodiscard]] QSet<CrateId> collectCrateIdsOfTracks(
            const QList<TrackId>& trackIds) const;

    /////////////////////////////////////////////////////////////////////////
    // CrateSummary view operations (read-only, const)
    /////////////////////////////////////////////////////////////////////////

    // Track summaries of all crates:
    //  - Hidden tracks are excluded from the crate summary statistics
    //  - The result list is ordered by crate name:
    //     - case-insensitive
    //     - locale-aware
    [[nodiscard]] CrateSummarySelectResult selectCrateSummaries() const; // all crates

    // Omit the pCrate parameter for checking if the corresponding crate exists.
    bool readCrateSummaryById(CrateId id, CrateSummary* pCrateSummary = nullptr) const;

  private:
    void createViews();

    QSqlDatabase m_database;
};
