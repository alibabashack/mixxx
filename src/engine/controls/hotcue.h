#pragma once

#include "track/cue.h"
#include "track/cueinfo.h"

/// This is the position of a fresh loaded tack without any seek
constexpr int kNoHotCueNumber = 0;

/// Convert hot cue index to 1-based number
///
/// Works independent of if the hot cue index is either 0-based
/// or 1..n-based.
inline int hotcueIndexToHotcueNumber(int hotcueIndex) {
    if (hotcueIndex >= mixxx::kFirstHotCueIndex) {
        DEBUG_ASSERT(hotcueIndex != Cue::kNoHotCue);
        return (hotcueIndex - mixxx::kFirstHotCueIndex) + 1; // to 1-based numbering
    } else {
        DEBUG_ASSERT(hotcueIndex == Cue::kNoHotCue);
        return kNoHotCueNumber;
    }
}

/// Convert 1-based hot cue number to hot cue index.
///
/// Works independent of if the hot cue index is either 0-based
/// or 1..n-based.
inline int hotcueNumberToHotcueIndex(int hotcueNumber) {
    if (hotcueNumber >= 1) {
        DEBUG_ASSERT(hotcueNumber != kNoHotCueNumber);
        return mixxx::kFirstHotCueIndex + (hotcueNumber - 1); // from 1-based numbering
    } else {
        DEBUG_ASSERT(hotcueNumber == kNoHotCueNumber);
        return Cue::kNoHotCue;
    }
}
