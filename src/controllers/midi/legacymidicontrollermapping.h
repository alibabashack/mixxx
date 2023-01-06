#pragma once

#include <QMultiHash>

#include "controllers/legacycontrollermapping.h"
#include "controllers/midi/midimessage.h"

/// Represents a MIDI controller mapping, containing the data elements that make
/// it up.
class LegacyMidiControllerMapping : public LegacyControllerMapping {
  public:
    LegacyMidiControllerMapping(){};
    ~LegacyMidiControllerMapping() override{};

    [[nodiscard]] std::shared_ptr<LegacyControllerMapping> clone() const override {
        return std::make_shared<LegacyMidiControllerMapping>(*this);
    }

    [[nodiscard]] bool saveMapping(const QString& fileName) const override;

    [[nodiscard]] bool isMappable() const override;

    // Input mappings
    void addInputMapping(uint16_t key, const MidiInputMapping& mapping);
    void removeInputMapping(uint16_t key);
    [[nodiscard]] const QMultiHash<uint16_t, MidiInputMapping>& getInputMappings() const;
    void setInputMappings(const QMultiHash<uint16_t, MidiInputMapping>& mappings);

    // Output mappings
    void addOutputMapping(const ConfigKey& key, const MidiOutputMapping& mapping);
    void removeOutputMapping(const ConfigKey& key);
    [[nodiscard]] const QMultiHash<ConfigKey, MidiOutputMapping>& getOutputMappings() const;
    void setOutputMappings(const QMultiHash<ConfigKey, MidiOutputMapping>& mappings);

  private:
    // MIDI input and output mappings.
    QMultiHash<uint16_t, MidiInputMapping> m_inputMappings;
    QMultiHash<ConfigKey, MidiOutputMapping> m_outputMappings;
};
