#pragma once

#include <QList>
#include <QString>

#include "preferences/usersettings.h"
#include "soundio/sounddevicestatus.h"
#include "soundio/soundmanagerutil.h"
#include "util/types.h"

class SoundDevice;
class SoundManager;
class AudioOutput;
class AudioInput;
class AudioOutputBuffer;
class AudioInputBuffer;

const QString kNetworkDeviceInternalName = "Network stream";

class SoundDevice {
  public:
    SoundDevice(UserSettingsPointer config, SoundManager* sm);
    virtual ~SoundDevice() = default;

    [[nodiscard]] inline const SoundDeviceId& getDeviceId() const {
        return m_deviceId;
    }
    [[nodiscard]] inline const QString& getDisplayName() const {
        return m_strDisplayName;
    }
    [[nodiscard]] inline const QString& getHostAPI() const {
        return m_hostAPI;
    }
    void setSampleRate(double sampleRate);
    void setFramesPerBuffer(unsigned int framesPerBuffer);
    virtual SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) = 0;
    [[nodiscard]] virtual bool isOpen() const = 0;
    virtual SoundDeviceStatus close() = 0;
    virtual void readProcess() = 0;
    virtual void writeProcess() = 0;
    [[nodiscard]] virtual QString getError() const = 0;
    [[nodiscard]] virtual unsigned int getDefaultSampleRate() const = 0;
    [[nodiscard]] int getNumOutputChannels() const;
    [[nodiscard]] int getNumInputChannels() const;
    SoundDeviceStatus addOutput(const AudioOutputBuffer& out);
    SoundDeviceStatus addInput(const AudioInputBuffer& in);
    [[nodiscard]] const QList<AudioInputBuffer>& inputs() const {
        return m_audioInputs;
    }
    [[nodiscard]] const QList<AudioOutputBuffer>& outputs() const {
        return m_audioOutputs;
    }

    void clearOutputs();
    void clearInputs();
    bool operator==(const SoundDevice &other) const;
    bool operator==(const QString &other) const;

  protected:
    void composeOutputBuffer(CSAMPLE* outputBuffer,
                             const SINT iFramesPerBuffer,
                             const SINT readOffset,
                             const int iFrameSize);

    void composeInputBuffer(const CSAMPLE* inputBuffer,
                            const SINT framesToPush,
                            const SINT framesWriteOffset,
                            const int iFrameSize);

    void clearInputBuffer(const SINT framesToPush,
                          const SINT framesWriteOffset);

    SoundDeviceId m_deviceId;
    UserSettingsPointer m_pConfig;
    // Pointer to the SoundManager object which we'll request audio from.
    SoundManager* m_pSoundManager;
    // The name of the soundcard, as displayed to the user
    QString m_strDisplayName;
    // The number of output channels that the soundcard has
    int m_iNumOutputChannels;
    // The number of input channels that the soundcard has
    int m_iNumInputChannels;
    // The current samplerate for the sound device.
    double m_dSampleRate;
    // The name of the audio API used by this device.
    QString m_hostAPI;
    SINT m_framesPerBuffer;
    QList<AudioOutputBuffer> m_audioOutputs;
    QList<AudioInputBuffer> m_audioInputs;
};

typedef QSharedPointer<SoundDevice> SoundDevicePointer;
