#pragma once

#include "control/controlaudiotaperpot.h"
#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "engine/channels/enginechannel.h"
#include "soundio/soundmanagerutil.h"
#include "util/circularbuffer.h"

class ControlAudioTaperPot;

/// EngineAux is an EngineChannel that implements a mixing source whose
/// samples are fed directly from the SoundManager
class EngineAux : public EngineChannel, public AudioDestination {
    Q_OBJECT
  public:
    EngineAux(const ChannelHandleAndGroup& handleGroup, EffectsManager* pEffectsManager);

    ActiveState updateActiveState() override;

    /// Called by EngineMaster whenever is requesting a new buffer of audio.
    void process(CSAMPLE* pOutput, const int iBufferSize) override;
    void collectFeatures(GroupFeatureState* pGroupFeatures) const override;
    void postProcess(const int iBufferSize) override {
        Q_UNUSED(iBufferSize)
    }

    /// This is called by SoundManager whenever there are new samples from the
    /// configured input to be processed. This is run in the callback thread of
    /// the soundcard this AudioDestination was registered for! Beware, in the
    /// case of multiple soundcards, this method is not re-entrant but it may be
    /// concurrent with EngineMaster processing.
    void receiveBuffer(const AudioInput& input,
            const CSAMPLE* pBuffer,
            unsigned int nFrames) override;

    /// Called by SoundManager whenever the aux input is connected to a
    /// soundcard input.
    void onInputConfigured(const AudioInput& input) override;

    /// Called by SoundManager whenever the aux input is disconnected from
    /// a soundcard input.
    void onInputUnconfigured(const AudioInput& input) override;

  private:
    ControlObject m_inputConfigured;
    ControlAudioTaperPot m_pregain;
};
