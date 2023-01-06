#pragma once

#include <hidapi.h>

#include <QObject>
#include <string>

struct ProductInfo;

namespace mixxx {

namespace hid {

constexpr unsigned short kGenericDesktopUsagePage = 0x01;

constexpr unsigned short kGenericDesktopMouseUsage = 0x02;
constexpr unsigned short kGenericDesktopKeyboardUsage = 0x06;

// Apple has two two different vendor IDs which are used for different devices.
constexpr unsigned short kAppleVendorId = 0x5ac;
constexpr unsigned short kAppleIncVendorId = 0x004c;

/// Detached copy of `struct hid_device_info`.
///
/// Stores a detached copy of hid_device_info and its contents.
///
/// All instances of hid_device_info are returned by the HIDAPI
/// library as a linked list. The memory of both the members
/// of this list as well as their contents are managed by the
/// library and freed immediately after iterating through this
/// list.
///
/// Includes some basic validations and implicit conversion to
/// QString if needed.
class DeviceInfo final {
  public:
    explicit DeviceInfo(
            const hid_device_info& device_info);

    // The VID.
    [[nodiscard]] unsigned short vendorId() const {
        return vendor_id;
    }
    // The PID.
    [[nodiscard]] unsigned short productId() const {
        return product_id;
    }
    /// The release number as a binary-coded decimal (BCD).
    [[nodiscard]] unsigned short releaseNumberBCD() const {
        return release_number;
    }

    /// The raw path, needed for subsequent HIDAPI requests.
    [[nodiscard]] const char* pathRaw() const {
        return m_pathRaw.c_str();
    }
    /// The raw serial number, needed for subsequent HIDAPI requests.
    [[nodiscard]] const wchar_t* serialNumberRaw() const {
        return m_serialNumberRaw.c_str();
    }

    [[nodiscard]] const QString& manufacturerString() const {
        return m_manufacturerString;
    }
    [[nodiscard]] const QString& productString() const {
        return m_productString;
    }
    [[nodiscard]] const QString& serialNumber() const {
        return m_serialNumber;
    }

    [[nodiscard]] bool isValid() const {
        return !productString().isNull() && !serialNumber().isNull();
    }

    [[nodiscard]] QString formatVID() const;
    [[nodiscard]] QString formatPID() const;
    [[nodiscard]] QString formatReleaseNumber() const;
    [[nodiscard]] QString formatInterface() const;
    [[nodiscard]] QString formatUsage() const;
    [[nodiscard]] QString formatName() const;

    [[nodiscard]] bool matchProductInfo(
            const ProductInfo& productInfo) const;

  private:
    friend class DeviceCategory;
    friend QDebug operator<<(
            QDebug dbg,
            const DeviceInfo& deviceInfo);

    // members from hid_device_info
    unsigned short vendor_id;
    unsigned short product_id;
    unsigned short release_number;
    unsigned short usage_page;
    unsigned short usage;
    int interface_number;

    std::string m_pathRaw;
    std::wstring m_serialNumberRaw;

    QString m_manufacturerString;
    QString m_productString;
    QString m_serialNumber;
};

class DeviceCategory final : public QObject {
    // QObject needed for i18n device category
    Q_OBJECT
  public:
    [[nodiscard]] static QString guessFromDeviceInfo(
            const DeviceInfo& deviceInfo) {
        return DeviceCategory().guessFromDeviceInfoImpl(deviceInfo);
    }

  private:
    [[nodiscard]] QString guessFromDeviceInfoImpl(
            const DeviceInfo& deviceInfo) const;

    DeviceCategory() = default;
};

} // namespace hid

} // namespace mixxx
