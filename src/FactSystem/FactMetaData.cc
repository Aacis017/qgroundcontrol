/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "FactMetaData.h"
#include "JsonHelper.h"
#include "MAVLinkLib.h"
#include "QGCLoggingCategory.h"
#include "SettingsManager.h"
#include "UnitsSettings.h"

#include <QtCore/QtMath>

QGC_LOGGING_CATEGORY(FactMetaDataLog, "qgc.factsystem.factmetadata")

// Built in translations for all Facts
const FactMetaData::BuiltInTranslation_s FactMetaData::_rgBuiltInTranslations[] = {
    { "centi-degrees",  "deg",  FactMetaData::_centiDegreesToDegrees,                   FactMetaData::_degreesToCentiDegrees },
    { "radians",        "deg",  FactMetaData::_radiansToDegrees,                        FactMetaData::_degreesToRadians },
    { "rad",            "deg",  FactMetaData::_radiansToDegrees,                        FactMetaData::_degreesToRadians },
    { "gimbal-degrees", "deg",  FactMetaData::_mavlinkGimbalDegreesToUserGimbalDegrees, FactMetaData::_userGimbalDegreesToMavlinkGimbalDegrees },
    { "norm",           "%",    FactMetaData::_normToPercent,                           FactMetaData::_percentToNorm },
};

// Translations driven by app settings
const FactMetaData::AppSettingsTranslation_s FactMetaData::_rgAppSettingsTranslations[] = {
    { "m",      "m",        FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsMeters, FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "meter",  "meter",    FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsMeters, FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "meters", "meters",   FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsMeters, FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    // NOTE: we've coined an artificial "raw unit" of "vertical metre" to separate it from the horizontal metre - a bit awkward but this is all the design permits
    { "vertical m",  "m",   FactMetaData::UnitVerticalDistance,      UnitsSettings::VerticalDistanceUnitsMeters,   FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "cm/px",  "cm/px",    FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsMeters, FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "m/s",    "m/s",      FactMetaData::UnitSpeed,                 UnitsSettings::SpeedUnitsMetersPerSecond,     FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "C",      "C",        FactMetaData::UnitTemperature,           UnitsSettings::TemperatureUnitsCelsius,       FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "m^2",    "m^2",      FactMetaData::UnitArea,                  UnitsSettings::AreaUnitsSquareMeters,         FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "m",      "ft",       FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsFeet,   FactMetaData::_metersToFeet,                        FactMetaData::_feetToMeters },
    { "meter",  "ft",       FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsFeet,   FactMetaData::_metersToFeet,                        FactMetaData::_feetToMeters },
    { "meters", "ft",       FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsFeet,   FactMetaData::_metersToFeet,                        FactMetaData::_feetToMeters },
    { "vertical m",  "ft",  FactMetaData::UnitVerticalDistance,      UnitsSettings::VerticalDistanceUnitsFeet,     FactMetaData::_metersToFeet,                        FactMetaData::_feetToMeters },
    { "cm/px",  "in/px",    FactMetaData::UnitHorizontalDistance,    UnitsSettings::HorizontalDistanceUnitsFeet,   FactMetaData::_centimetersToInches,                 FactMetaData::_inchesToCentimeters },
    { "m^2",    "km^2",     FactMetaData::UnitArea,                  UnitsSettings::AreaUnitsSquareKilometers,     FactMetaData::_squareMetersToSquareKilometers,      FactMetaData::_squareKilometersToSquareMeters },
    { "m^2",    "ha",       FactMetaData::UnitArea,                  UnitsSettings::AreaUnitsHectares,             FactMetaData::_squareMetersToHectares,              FactMetaData::_hectaresToSquareMeters },
    { "m^2",    "ft^2",     FactMetaData::UnitArea,                  UnitsSettings::AreaUnitsSquareFeet,           FactMetaData::_squareMetersToSquareFeet,            FactMetaData::_squareFeetToSquareMeters },
    { "m^2",    "ac",       FactMetaData::UnitArea,                  UnitsSettings::AreaUnitsAcres,                FactMetaData::_squareMetersToAcres,                 FactMetaData::_acresToSquareMeters },
    { "m^2",    "mi^2",     FactMetaData::UnitArea,                  UnitsSettings::AreaUnitsSquareMiles,          FactMetaData::_squareMetersToSquareMiles,           FactMetaData::_squareMilesToSquareMeters },
    { "m/s",    "ft/s",     FactMetaData::UnitSpeed,                 UnitsSettings::SpeedUnitsFeetPerSecond,       FactMetaData::_metersToFeet,                        FactMetaData::_feetToMeters },
    { "m/s",    "mph",      FactMetaData::UnitSpeed,                 UnitsSettings::SpeedUnitsMilesPerHour,        FactMetaData::_metersPerSecondToMilesPerHour,       FactMetaData::_milesPerHourToMetersPerSecond },
    { "m/s",    "km/h",     FactMetaData::UnitSpeed,                 UnitsSettings::SpeedUnitsKilometersPerHour,   FactMetaData::_metersPerSecondToKilometersPerHour,  FactMetaData::_kilometersPerHourToMetersPerSecond },
    { "m/s",    "kn",       FactMetaData::UnitSpeed,                 UnitsSettings::SpeedUnitsKnots,               FactMetaData::_metersPerSecondToKnots,              FactMetaData::_knotsToMetersPerSecond },
    { "C",      "F",        FactMetaData::UnitTemperature,           UnitsSettings::TemperatureUnitsFarenheit,     FactMetaData::_celsiusToFarenheit,                  FactMetaData::_farenheitToCelsius },
    { "g",      "g",        FactMetaData::UnitWeight,                UnitsSettings::WeightUnitsGrams,              FactMetaData::_defaultTranslator,                   FactMetaData::_defaultTranslator },
    { "g",      "kg",       FactMetaData::UnitWeight,                UnitsSettings::WeightUnitsKg,                 FactMetaData::_gramsToKilograms,                    FactMetaData::_kilogramsToGrams },
    { "g",      "oz",       FactMetaData::UnitWeight,                UnitsSettings::WeightUnitsOz,                 FactMetaData::_gramsToOunces,                       FactMetaData::_ouncesToGrams },
    { "g",      "lbs",      FactMetaData::UnitWeight,                UnitsSettings::WeightUnitsLbs,                FactMetaData::_gramsToPunds,                        FactMetaData::_poundsToGrams },
};

FactMetaData::FactMetaData(QObject *parent)
    : QObject(parent)
{
    // qCDebug(FactMetaDataLog) << Q_FUNC_INFO << this;
}

FactMetaData::FactMetaData(ValueType_t type, QObject *parent)
    : QObject(parent)
    , _type(type)
{
    // qCDebug(FactMetaDataLog) << Q_FUNC_INFO << this;
}

FactMetaData::FactMetaData(const FactMetaData &other, QObject *parent)
    : QObject(parent)
{
    // qCDebug(FactMetaDataLog) << Q_FUNC_INFO << this;
    *this = other;
}

FactMetaData::FactMetaData(ValueType_t type, const QString &name, QObject *parent)
    : QObject(parent)
    , _type(type)
    , _name(name)
{
    // qCDebug(FactMetaDataLog) << Q_FUNC_INFO << this;
}

FactMetaData::~FactMetaData()
{
    // qCDebug(FactMetaDataLog) << Q_FUNC_INFO << this;
}

const FactMetaData &FactMetaData::operator=(const FactMetaData &other)
{
    _decimalPlaces = other._decimalPlaces;
    _rawDefaultValue = other._rawDefaultValue;
    _defaultValueAvailable = other._defaultValueAvailable;
    _bitmaskStrings = other._bitmaskStrings;
    _bitmaskValues = other._bitmaskValues;
    _enumStrings = other._enumStrings;
    _enumValues = other._enumValues;
    _category = other._category;
    _group = other._group;
    _longDescription = other._longDescription;
    _rawMax = other._rawMax;
    _rawMin = other._rawMin;
    _name = other._name;
    _shortDescription = other._shortDescription;
    _type = other._type;
    _rawUnits = other._rawUnits;
    _cookedUnits = other._cookedUnits;
    _rawTranslator = other._rawTranslator;
    _cookedTranslator = other._cookedTranslator;
    _vehicleRebootRequired = other._vehicleRebootRequired;
    _qgcRebootRequired = other._qgcRebootRequired;
    _rawIncrement = other._rawIncrement;
    _hasControl = other._hasControl;
    _readOnly = other._readOnly;
    _writeOnly = other._writeOnly;
    _volatile = other._volatile;

    return *this;
}

QVariant FactMetaData::rawDefaultValue() const
{
    if (_defaultValueAvailable) {
        return _rawDefaultValue;
    } else {
        qWarning(FactMetaDataLog) << "Attempt to access unavailable default value";
        return QVariant(0);
    }
}

void FactMetaData::setRawDefaultValue(const QVariant &rawDefaultValue)
{
    if ((_type == valueTypeString) || (isInRawMinLimit(rawDefaultValue) && isInRawMaxLimit(rawDefaultValue))) {
        _rawDefaultValue = rawDefaultValue;
        _defaultValueAvailable = true;
    } else {
        qWarning(FactMetaDataLog) << "Attempt to set default value which is outside min/max range";
    }
}

void FactMetaData::setRawMin(const QVariant &rawMin)
{
    if (isInRawMinLimit(rawMin)) {
        _rawMin = rawMin;
    } else {
        qWarning(FactMetaDataLog) << "Attempt to set min below allowable value for fact:" << name()
                                  << ", value attempted:" << rawMin
                                  << ", type:" << type()
                                  << ", min for type:" << _minForType();
        _rawMin = _minForType();
    }
}

void FactMetaData::setRawMax(const QVariant &rawMax)
{
    if (isInRawMaxLimit(rawMax)) {
        _rawMax = rawMax;
    } else {
        qWarning(FactMetaDataLog) << "Attempt to set max above allowable value for fact:" << name()
                                  << ", value attempted:" << rawMax
                                  << ", type:" << type()
                                  << ", max for type:" << _maxForType();
        _rawMax = _maxForType();
    }
}

bool FactMetaData::isInRawMinLimit(const QVariant &variantValue) const
{
    switch (_type) {
    case valueTypeUint8:
        return (_rawMin.value<unsigned char>() <= variantValue.value<unsigned char>());
    case valueTypeInt8:
        return (_rawMin.value<signed char>() <= variantValue.value<signed char>());
    case valueTypeUint16:
        return (_rawMin.value<unsigned short int>() <= variantValue.value<unsigned short int>());
    case valueTypeInt16:
        return (_rawMin.value<short int>() <= variantValue.value<short int>());
    case valueTypeUint32:
        return (_rawMin.value<uint32_t>() <= variantValue.value<uint32_t>());
    case valueTypeInt32:
        return (_rawMin.value<int32_t>() <= variantValue.value<int32_t>());
    case valueTypeUint64:
        return (_rawMin.value<uint64_t>() <= variantValue.value<uint64_t>());
    case valueTypeInt64:
        return (_rawMin.value<int64_t>() <= variantValue.value<int64_t>());
    case valueTypeFloat:
        return ((qIsNaN(variantValue.toFloat())) || (_rawMin.value<float>() <= variantValue.value<float>()));
    case valueTypeDouble:
        return ((qIsNaN(variantValue.toDouble())) || (_rawMin.value<double>() <= variantValue.value<double>()));
    default:
        return true;
    }
}

bool FactMetaData::isInRawMaxLimit(const QVariant &variantValue) const
{
    switch (_type) {
    case valueTypeUint8:
        return (_rawMax.value<unsigned char>() >= variantValue.value<unsigned char>());
    case valueTypeInt8:
        return (_rawMax.value<signed char>() >= variantValue.value<signed char>());
    case valueTypeUint16:
        return (_rawMax.value<unsigned short int>() >= variantValue.value<unsigned short int>());
    case valueTypeInt16:
        return (_rawMax.value<short int>() >= variantValue.value<short int>());
    case valueTypeUint32:
        return (_rawMax.value<uint32_t>() >= variantValue.value<uint32_t>());
    case valueTypeInt32:
        return (_rawMax.value<int32_t>() >= variantValue.value<int32_t>());
    case valueTypeUint64:
        return (_rawMax.value<uint64_t>() >= variantValue.value<uint64_t>());
    case valueTypeInt64:
        return (_rawMax.value<int64_t>() >= variantValue.value<int64_t>());
    case valueTypeFloat:
        return (qIsNaN(variantValue.toFloat()) || (_rawMax.value<float>() >= variantValue.value<float>()));
    case valueTypeDouble:
        return (qIsNaN(variantValue.toDouble()) || (_rawMax.value<double>() >= variantValue.value<double>()));
    default:
        return true;
    }
}

QVariant FactMetaData::minForType(ValueType_t type)
{
    switch (type) {
    case valueTypeUint8:
        return QVariant(std::numeric_limits<unsigned char>::min());
    case valueTypeInt8:
        return QVariant(std::numeric_limits<signed char>::min());
    case valueTypeUint16:
        return QVariant(std::numeric_limits<unsigned short int>::min());
    case valueTypeInt16:
        return QVariant(std::numeric_limits<short int>::min());
    case valueTypeUint32:
        return QVariant(std::numeric_limits<uint32_t>::min());
    case valueTypeInt32:
        return QVariant(std::numeric_limits<int32_t>::min());
    case valueTypeUint64:
        return QVariant((qulonglong)std::numeric_limits<uint64_t>::min());
    case valueTypeInt64:
        return QVariant((qlonglong)std::numeric_limits<int64_t>::min());
    case valueTypeFloat:
        return QVariant(-std::numeric_limits<float>::max());
    case valueTypeDouble:
        return QVariant(-std::numeric_limits<double>::max());
    case valueTypeString:
        return QVariant();
    case valueTypeBool:
        return QVariant(0);
    case valueTypeElapsedTimeInSeconds:
        return QVariant(0.0);
    case valueTypeCustom:
    default:
        return QVariant();
    }
}

QVariant FactMetaData::maxForType(ValueType_t type)
{
    switch (type) {
    case valueTypeUint8:
        return QVariant(std::numeric_limits<unsigned char>::max());
    case valueTypeInt8:
        return QVariant(std::numeric_limits<signed char>::max());
    case valueTypeUint16:
        return QVariant(std::numeric_limits<unsigned short int>::max());
    case valueTypeInt16:
        return QVariant(std::numeric_limits<short int>::max());
    case valueTypeUint32:
        return QVariant(std::numeric_limits<uint32_t>::max());
    case valueTypeInt32:
        return QVariant(std::numeric_limits<int32_t>::max());
    case valueTypeUint64:
        return QVariant((qulonglong)std::numeric_limits<uint64_t>::max());
    case valueTypeInt64:
        return QVariant((qlonglong)std::numeric_limits<int64_t>::max());
    case valueTypeFloat:
        return QVariant(std::numeric_limits<float>::max());
    case valueTypeElapsedTimeInSeconds:
    case valueTypeDouble:
        return QVariant(std::numeric_limits<double>::max());
    case valueTypeString:
        return QVariant();
    case valueTypeBool:
        return QVariant(1);
    case valueTypeCustom:
    default:
        return QVariant();
    }
}

bool FactMetaData::convertAndValidateRaw(const QVariant &rawValue, bool convertOnly, QVariant &typedValue, QString &errorString) const
{
    bool convertOk = false;

    errorString.clear();

    switch (type()) {
    case FactMetaData::valueTypeInt8:
    case FactMetaData::valueTypeInt16:
    case FactMetaData::valueTypeInt32:
        typedValue = QVariant(rawValue.toInt(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInRawLimit<int32_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(rawMin().toInt()).arg(rawMax().toInt());
            }
        }
        break;
    case FactMetaData::valueTypeInt64:
        typedValue = QVariant(rawValue.toLongLong(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInRawLimit<int64_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(rawMin().toInt()).arg(rawMax().toInt());
            }
        }
        break;
    case FactMetaData::valueTypeUint8:
    case FactMetaData::valueTypeUint16:
    case FactMetaData::valueTypeUint32:
        typedValue = QVariant(rawValue.toUInt(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInRawLimit<uint32_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(rawMin().toUInt()).arg(rawMax().toUInt());
            }
        }
        break;
    case FactMetaData::valueTypeUint64:
        typedValue = QVariant(rawValue.toULongLong(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInRawLimit<uint64_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(rawMin().toUInt()).arg(rawMax().toUInt());
            }
        }
        break;
    case FactMetaData::valueTypeFloat:
        typedValue = QVariant(rawValue.toFloat(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInRawLimit<float>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(rawMin().toDouble()).arg(rawMax().toDouble());
            }
        }
        break;
    case FactMetaData::valueTypeElapsedTimeInSeconds:
    case FactMetaData::valueTypeDouble:
        typedValue = QVariant(rawValue.toDouble(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInRawLimit<double>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(rawMin().toDouble()).arg(rawMax().toDouble());
            }
        }
        break;
    case FactMetaData::valueTypeString:
        convertOk = true;
        typedValue = QVariant(rawValue.toString());
        break;
    case FactMetaData::valueTypeBool:
        convertOk = true;
        typedValue = QVariant(rawValue.toBool());
        break;
    case FactMetaData::valueTypeCustom:
        convertOk = true;
        typedValue = QVariant(rawValue.toByteArray());
        break;
    }

    if (!convertOk) {
        errorString += tr("Invalid number");
    }

    return (convertOk && errorString.isEmpty());
}

bool FactMetaData::convertAndValidateCooked(const QVariant &cookedValue, bool convertOnly, QVariant &typedValue, QString &errorString) const
{
    bool convertOk = false;

    errorString.clear();

    if (!convertOnly && _customCookedValidator) {
        errorString = _customCookedValidator(cookedValue);
        if (!errorString.isEmpty()) {
            return false;
        }
    }

    switch (type()) {
    case FactMetaData::valueTypeInt8:
    case FactMetaData::valueTypeInt16:
    case FactMetaData::valueTypeInt32:
        typedValue = QVariant(cookedValue.toInt(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInCookedLimit<int32_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(cookedMin().toInt()).arg(cookedMax().toInt());
            }
        }
        break;
    case FactMetaData::valueTypeInt64:
        typedValue = QVariant(cookedValue.toLongLong(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInCookedLimit<int64_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(cookedMin().toInt()).arg(cookedMax().toInt());
            }
        }
        break;
    case FactMetaData::valueTypeUint8:
    case FactMetaData::valueTypeUint16:
    case FactMetaData::valueTypeUint32:
        typedValue = QVariant(cookedValue.toUInt(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInCookedLimit<uint32_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(cookedMin().toUInt()).arg(cookedMax().toUInt());
            }
        }
        break;
    case FactMetaData::valueTypeUint64:
        typedValue = QVariant(cookedValue.toULongLong(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInCookedLimit<uint64_t>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(cookedMin().toUInt()).arg(cookedMax().toUInt());
            }
        }
        break;
    case FactMetaData::valueTypeFloat:
        typedValue = QVariant(cookedValue.toFloat(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInCookedLimit<float>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(cookedMin().toFloat()).arg(cookedMax().toFloat());
            }
        }
        break;
    case FactMetaData::valueTypeElapsedTimeInSeconds:
    case FactMetaData::valueTypeDouble:
        typedValue = QVariant(cookedValue.toDouble(&convertOk));
        if (!convertOnly && convertOk) {
            if (!isInCookedLimit<double>(typedValue)) {
                errorString = tr("Value must be within %1 and %2").arg(cookedMin().toDouble()).arg(cookedMax().toDouble());
            }
        }
        break;
    case FactMetaData::valueTypeString:
        convertOk = true;
        typedValue = QVariant(cookedValue.toString());
        break;
    case FactMetaData::valueTypeBool:
        convertOk = true;
        typedValue = QVariant(cookedValue.toBool());
        break;
    case FactMetaData::valueTypeCustom:
        convertOk = true;
        typedValue = QVariant(cookedValue.toByteArray());
        break;
    }

    if (!convertOk) {
        errorString += tr("Invalid number");
    }

    return (convertOk && errorString.isEmpty());
}

bool FactMetaData::clampValue(const QVariant &cookedValue, QVariant &typedValue) const
{
    bool convertOk = false;

    switch (type()) {
    case FactMetaData::valueTypeInt8:
    case FactMetaData::valueTypeInt16:
    case FactMetaData::valueTypeInt32:
        typedValue = QVariant(cookedValue.toInt(&convertOk));
        if (convertOk) {
            clamp<int32_t>(typedValue);
        }
        break;
    case FactMetaData::valueTypeInt64:
        typedValue = QVariant(cookedValue.toLongLong(&convertOk));
        if (convertOk) {
            clamp<int64_t>(typedValue);
        }
        break;
    case FactMetaData::valueTypeUint8:
    case FactMetaData::valueTypeUint16:
    case FactMetaData::valueTypeUint32:
        typedValue = QVariant(cookedValue.toUInt(&convertOk));
        if (convertOk) {
            clamp<uint32_t>(typedValue);
        }
        break;
    case FactMetaData::valueTypeUint64:
        typedValue = QVariant(cookedValue.toULongLong(&convertOk));
        if (convertOk) {
            clamp<uint64_t>(typedValue);
        }
        break;
    case FactMetaData::valueTypeFloat:
        typedValue = QVariant(cookedValue.toFloat(&convertOk));
        if (convertOk) {
            clamp<float>(typedValue);
        }
        break;
    case FactMetaData::valueTypeElapsedTimeInSeconds:
    case FactMetaData::valueTypeDouble:
        typedValue = QVariant(cookedValue.toDouble(&convertOk));
        if (convertOk) {
            clamp<double>(typedValue);
        }
        break;
    case FactMetaData::valueTypeString:
        convertOk = true;
        typedValue = QVariant(cookedValue.toString());
        break;
    case FactMetaData::valueTypeBool:
        convertOk = true;
        typedValue = QVariant(cookedValue.toBool());
        break;
    case FactMetaData::valueTypeCustom:
        convertOk = true;
        typedValue = QVariant(cookedValue.toByteArray());
        break;
    }

    return convertOk;
}

void FactMetaData::setBitmaskInfo(const QStringList &strings, const QVariantList &values)
{
    if (strings.count() != values.count()) {
        qWarning(FactMetaDataLog) << "Count mismatch strings:values" << strings.count() << values.count();
        return;
    }

    _bitmaskStrings = strings;
    _bitmaskValues = values;
    setBuiltInTranslator();
}

void FactMetaData::addBitmaskInfo(const QString &name, const QVariant &value)
{
    _bitmaskStrings << name;
    _bitmaskValues << value;
}

void FactMetaData::setEnumInfo(const QStringList &strings, const QVariantList &values)
{
    if (strings.count() != values.count()) {
        qWarning(FactMetaDataLog) << "Count mismatch strings:values" << strings.count() << values.count();
        return;
    }

    _enumStrings = strings;
    _enumValues = values;
    setBuiltInTranslator();
}

void FactMetaData::addEnumInfo(const QString &name, const QVariant &value)
{
    _enumStrings << name;
    _enumValues << value;
}

void FactMetaData::removeEnumInfo(const QVariant &value)
{
    const int index = _enumValues.indexOf(value);
    if (index < 0) {
        qWarning(FactMetaDataLog) << "Value does not exist in fact:" << value;
        return;
    }

    _enumValues.removeAt(index);
    _enumStrings.removeAt(index);
}

void FactMetaData::setTranslators(Translator rawTranslator, Translator cookedTranslator)
{
    _rawTranslator = rawTranslator;
    _cookedTranslator = cookedTranslator;
}

void FactMetaData::setBuiltInTranslator()
{
    if (_enumStrings.count() || _bitmaskStrings.count()) {
        // No translation if enum
        setTranslators(_defaultTranslator, _defaultTranslator);
        _cookedUnits = _rawUnits;
        return;
    } else {
        for (size_t i = 0; i < std::size(_rgBuiltInTranslations); i++) {
            const BuiltInTranslation_s *pBuiltInTranslation = &_rgBuiltInTranslations[i];

            if (pBuiltInTranslation->rawUnits.toLower() == _rawUnits.toLower()) {
                _cookedUnits = pBuiltInTranslation->cookedUnits;
                setTranslators(pBuiltInTranslation->rawTranslator, pBuiltInTranslation->cookedTranslator);
                return;
            }
        }
    }

    // Translator not yet set, try app settings translators
    _setAppSettingsTranslators();
}

QVariant FactMetaData::_degreesToRadians(const QVariant &degrees)
{
    return QVariant(qDegreesToRadians(degrees.toDouble()));
}

QVariant FactMetaData::_radiansToDegrees(const QVariant &radians)
{
    return QVariant(qRadiansToDegrees(radians.toDouble()));
}

QVariant FactMetaData::_centiDegreesToDegrees(const QVariant &centiDegrees)
{
    return QVariant(centiDegrees.toReal() / 100.0);
}

QVariant FactMetaData::_degreesToCentiDegrees(const QVariant &degrees)
{
    return QVariant(qRound(degrees.toReal() * 100.0));
}

QVariant FactMetaData::_userGimbalDegreesToMavlinkGimbalDegrees(const QVariant &userGimbalDegrees)
{
    // User facing gimbal degree values are from 0 (level) to 90 (straight down)
    // Mavlink gimbal degree values are from 0 (level) to -90 (straight down)
    return (userGimbalDegrees.toDouble() * -1.0);
}

QVariant FactMetaData::_mavlinkGimbalDegreesToUserGimbalDegrees(const QVariant& mavlinkGimbalDegrees)
{
    // User facing gimbal degree values are from 0 (level) to 90 (straight down)
    // Mavlink gimbal degree values are from 0 (level) to -90 (straight down)
    return (mavlinkGimbalDegrees.toDouble() * -1.0);
}

QVariant FactMetaData::_metersToFeet(const QVariant &meters)
{
    return QVariant((meters.toDouble() * 1.0) / constants.feetToMeters);
}

QVariant FactMetaData::_feetToMeters(const QVariant &feet)
{
    return QVariant(feet.toDouble() * constants.feetToMeters);
}

QVariant FactMetaData::_squareMetersToSquareKilometers(const QVariant &squareMeters)
{
    return QVariant(squareMeters.toDouble() * 0.000001);
}

QVariant FactMetaData::_squareKilometersToSquareMeters(const QVariant &squareKilometers)
{
    return QVariant(squareKilometers.toDouble() * 1000000.0);
}

QVariant FactMetaData::_squareMetersToHectares(const QVariant &squareMeters)
{
    return QVariant(squareMeters.toDouble() * 0.0001);
}

QVariant FactMetaData::_hectaresToSquareMeters(const QVariant &hectares)
{
    return QVariant(hectares.toDouble() * 1000.0);
}

QVariant FactMetaData::_squareMetersToSquareFeet(const QVariant &squareMeters)
{
    return QVariant(squareMeters.toDouble() * constants.squareMetersToSquareFeet);
}

QVariant FactMetaData::_squareFeetToSquareMeters(const QVariant &squareFeet)
{
    return QVariant(squareFeet.toDouble() * constants.feetToSquareMeters);
}

QVariant FactMetaData::_squareMetersToAcres(const QVariant &squareMeters)
{
    return QVariant(squareMeters.toDouble() * constants.squareMetersToAcres);
}

QVariant FactMetaData::_acresToSquareMeters(const QVariant &acres)
{
    return QVariant(acres.toDouble() * constants.acresToSquareMeters);
}

QVariant FactMetaData::_squareMetersToSquareMiles(const QVariant &squareMeters)
{
    return QVariant(squareMeters.toDouble() * constants.squareMetersToSquareMiles);
}

QVariant FactMetaData::_squareMilesToSquareMeters(const QVariant &squareMiles)
{
    return QVariant(squareMiles.toDouble() * constants.squareMilesToSquareMeters);
}

QVariant FactMetaData::_metersPerSecondToMilesPerHour(const QVariant &metersPerSecond)
{
    return QVariant(((metersPerSecond.toDouble() * 1.0) / constants.milesToMeters) * constants.secondsPerHour);
}

QVariant FactMetaData::_milesPerHourToMetersPerSecond(const QVariant &milesPerHour)
{
    return QVariant((milesPerHour.toDouble() * constants.milesToMeters) / constants.secondsPerHour);
}

QVariant FactMetaData::_metersPerSecondToKilometersPerHour(const QVariant &metersPerSecond)
{
    return QVariant((metersPerSecond.toDouble() / 1000.0) * constants.secondsPerHour);
}

QVariant FactMetaData::_kilometersPerHourToMetersPerSecond(const QVariant &kilometersPerHour)
{
    return QVariant((kilometersPerHour.toDouble() * 1000.0) / constants.secondsPerHour);
}

QVariant FactMetaData::_metersPerSecondToKnots(const QVariant &metersPerSecond)
{
    return QVariant((metersPerSecond.toDouble() * constants.secondsPerHour) / (1000.0 * constants.knotsToKPH));
}

QVariant FactMetaData::_knotsToMetersPerSecond(const QVariant& knots)
{
    return QVariant(knots.toDouble() * (1000.0 * constants.knotsToKPH / constants.secondsPerHour));
}

QVariant FactMetaData::_percentToNorm(const QVariant &percent)
{
    return QVariant(percent.toDouble() / 100.0);
}

QVariant FactMetaData::_normToPercent(const QVariant &normalized)
{
    return QVariant(normalized.toDouble() * 100.0);
}

QVariant FactMetaData::_centimetersToInches(const QVariant &centimeters)
{
    return QVariant((centimeters.toDouble() * 1.0) / constants.inchesToCentimeters);
}

QVariant FactMetaData::_inchesToCentimeters(const QVariant &inches)
{
    return QVariant(inches.toDouble() * constants.inchesToCentimeters);
}

QVariant FactMetaData::_celsiusToFarenheit(const QVariant &celsius)
{
    return QVariant((celsius.toDouble() * (9.0 / 5.0)) + 32);
}

QVariant FactMetaData::_farenheitToCelsius(const QVariant &farenheit)
{
    return QVariant((farenheit.toDouble() - 32) * (5.0 / 9.0));
}

QVariant FactMetaData::_kilogramsToGrams(const QVariant &kg)
{
    return QVariant(kg.toDouble() * 1000);
}

QVariant FactMetaData::_ouncesToGrams(const QVariant &oz)
{
    return QVariant(oz.toDouble() * constants.ouncesToGrams);
}

QVariant FactMetaData::_poundsToGrams(const QVariant &lbs)
{
    return QVariant(lbs.toDouble() * constants.poundsToGrams);
}

QVariant FactMetaData::_gramsToKilograms(const QVariant &g)
{
    return QVariant(g.toDouble() / 1000);
}

QVariant FactMetaData::_gramsToOunces(const QVariant &g)
{
    return QVariant(g.toDouble() / constants.ouncesToGrams);
}

QVariant FactMetaData::_gramsToPunds(const QVariant &g)
{
    return QVariant(g.toDouble() / constants.poundsToGrams);
}

void FactMetaData::setRawUnits(const QString &rawUnits)
{
    _rawUnits = rawUnits;
    _cookedUnits = rawUnits;

    setBuiltInTranslator();
}

FactMetaData::ValueType_t FactMetaData::stringToType(const QString &typeString, bool &unknownType)
{
    unknownType = false;

    for (size_t i = 0; i < std::size(_rgKnownTypeStrings); i++) {
        if (typeString.compare(_rgKnownTypeStrings[i], Qt::CaseInsensitive) == 0) {
            return _rgKnownValueTypes[i];
        }
    }

    unknownType = true;

    return valueTypeDouble;
}

QString FactMetaData::typeToString(ValueType_t type)
{
    for (size_t i = 0; i < std::size(_rgKnownTypeStrings); i++) {
        if (type == _rgKnownValueTypes[i]) {
            return _rgKnownTypeStrings[i];
        }
    }

    return QStringLiteral("UnknownType%1").arg(type);
}

size_t FactMetaData::typeToSize(ValueType_t type)
{
    switch (type) {
    case valueTypeUint8:
    case valueTypeInt8:
        return 1;
    case valueTypeUint16:
    case valueTypeInt16:
        return 2;
    case valueTypeUint32:
    case valueTypeInt32:
    case valueTypeFloat:
        return 4;
    case valueTypeUint64:
    case valueTypeInt64:
    case valueTypeDouble:
        return 8;
    case valueTypeCustom:
        return MAVLINK_MSG_PARAM_EXT_SET_FIELD_PARAM_VALUE_LEN;
    default:
        qWarning(FactMetaDataLog) << "Unsupported fact value type" << type;
        return 0;
    }
}

void FactMetaData::_setAppSettingsTranslators()
{
    // We can only translate between real numbers
    if (_enumStrings.isEmpty() && ((type() == valueTypeDouble) || (type() == valueTypeFloat))) {
        for (size_t i = 0; i < std::size(_rgAppSettingsTranslations); i++) {
            const AppSettingsTranslation_s *pAppSettingsTranslation = &_rgAppSettingsTranslations[i];

            if (_rawUnits.toLower() != pAppSettingsTranslation->rawUnits.toLower()) {
                continue;
            }

            UnitsSettings *const settings = SettingsManager::instance()->unitsSettings();
            uint settingsUnits = 0;

            switch (pAppSettingsTranslation->unitType) {
            case UnitHorizontalDistance:
                settingsUnits = settings->horizontalDistanceUnits()->rawValue().toUInt();
                break;
            case UnitVerticalDistance:
                settingsUnits = settings->verticalDistanceUnits()->rawValue().toUInt();
                break;
            case UnitSpeed:
                settingsUnits = settings->speedUnits()->rawValue().toUInt();
                break;
            case UnitArea:
                settingsUnits = settings->areaUnits()->rawValue().toUInt();
                break;
            case UnitTemperature:
                settingsUnits = settings->temperatureUnits()->rawValue().toUInt();
                break;
            case UnitWeight:
                settingsUnits = settings->weightUnits()->rawValue().toUInt();
                break;
            default:
                break;
            }

            if (settingsUnits == pAppSettingsTranslation->unitOption) {
                _cookedUnits = pAppSettingsTranslation->cookedUnits;
                setTranslators(pAppSettingsTranslation->rawTranslator, pAppSettingsTranslation->cookedTranslator);
                return;
            }
        }
    }
}

const FactMetaData::AppSettingsTranslation_s* FactMetaData::_findAppSettingsUnitsTranslation(const QString &rawUnits, UnitTypes type)
{
    for (size_t i = 0; i < std::size(_rgAppSettingsTranslations); i++) {
        const AppSettingsTranslation_s *const pAppSettingsTranslation = &_rgAppSettingsTranslations[i];

        if (rawUnits.toLower() != pAppSettingsTranslation->rawUnits.toLower()) {
            continue;
        }

        uint unitOption = 0;
        UnitsSettings *unitsSettings = SettingsManager::instance()->unitsSettings();
        switch (type) {
        case UnitHorizontalDistance:
            unitOption = unitsSettings->horizontalDistanceUnits()->rawValue().toUInt();
            break;
        case UnitVerticalDistance:
            unitOption = unitsSettings->verticalDistanceUnits()->rawValue().toUInt();
            break;
        case UnitArea:
            unitOption = unitsSettings->areaUnits()->rawValue().toUInt();
            break;
        case UnitSpeed:
            unitOption = unitsSettings->speedUnits()->rawValue().toUInt();
            break;
        case UnitTemperature:
            unitOption = unitsSettings->temperatureUnits()->rawValue().toUInt();
            break;
        case UnitWeight:
            unitOption = unitsSettings->weightUnits()->rawValue().toUInt();
            break;
        }

        if ((pAppSettingsTranslation->unitType == type) && (pAppSettingsTranslation->unitOption == unitOption)) {
            return pAppSettingsTranslation;
        }
    }

    return nullptr;
}

QVariant FactMetaData::metersToAppSettingsHorizontalDistanceUnits(const QVariant &meters)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m", UnitHorizontalDistance);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->rawTranslator(meters);
    } else {
        return meters;
    }
}

QVariant FactMetaData::metersToAppSettingsVerticalDistanceUnits(const QVariant &meters)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("vertical m", UnitVerticalDistance);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->rawTranslator(meters);
    } else {
        return meters;
    }
}

QVariant FactMetaData::appSettingsHorizontalDistanceUnitsToMeters(const QVariant &distance)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m", UnitHorizontalDistance);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedTranslator(distance);
    } else {
        return distance;
    }
}

QVariant FactMetaData::appSettingsVerticalDistanceUnitsToMeters(const QVariant &distance)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("vertical m", UnitVerticalDistance);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedTranslator(distance);
    } else {
        return distance;
    }
}

QString FactMetaData::appSettingsHorizontalDistanceUnitsString()
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m", UnitHorizontalDistance);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedUnits;
    } else {
        return QStringLiteral("m");
    }
}

QString FactMetaData::appSettingsVerticalDistanceUnitsString()
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("vertical m", UnitVerticalDistance);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedUnits;
    } else {
        return QStringLiteral("m");
    }
}

QString FactMetaData::appSettingsWeightUnitsString()
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("g", UnitWeight);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedUnits;
    } else {
        return QStringLiteral("g");
    }
}

QVariant FactMetaData::squareMetersToAppSettingsAreaUnits(const QVariant &squareMeters)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m^2", UnitArea);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->rawTranslator(squareMeters);
    } else {
        return squareMeters;
    }
}

QVariant FactMetaData::appSettingsAreaUnitsToSquareMeters(const QVariant &area)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m^2", UnitArea);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedTranslator(area);
    } else {
        return area;
    }
}

QString FactMetaData::appSettingsAreaUnitsString()
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m^2", UnitArea);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedUnits;
    } else {
        return QStringLiteral("m^2");
    }
}

QVariant FactMetaData::gramsToAppSettingsWeightUnits(const QVariant &grams) {
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("g", UnitWeight);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->rawTranslator(grams);
    } else {
        return grams;
    }
}

QVariant FactMetaData::appSettingsWeightUnitsToGrams(const QVariant &weight) {
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("g", UnitWeight);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedTranslator(weight);
    } else {
        return weight;
    }
}

QVariant FactMetaData::metersSecondToAppSettingsSpeedUnits(const QVariant &metersSecond)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m/s", UnitSpeed);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->rawTranslator(metersSecond);
    } else {
        return metersSecond;
    }
}

QVariant FactMetaData::appSettingsSpeedUnitsToMetersSecond(const QVariant &speed)
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m/s", UnitSpeed);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedTranslator(speed);
    } else {
        return speed;
    }
}

QString FactMetaData::appSettingsSpeedUnitsString()
{
    const AppSettingsTranslation_s *const pAppSettingsTranslation = _findAppSettingsUnitsTranslation("m/s", UnitSpeed);
    if (pAppSettingsTranslation) {
        return pAppSettingsTranslation->cookedUnits;
    } else {
        return QStringLiteral("m/s");
    }
}

double FactMetaData::cookedIncrement() const
{
    return _rawTranslator(this->rawIncrement()).toDouble();
}

int FactMetaData::decimalPlaces() const
{
    int actualDecimalPlaces = kDefaultDecimalPlaces;
    int incrementDecimalPlaces = kUnknownDecimalPlaces;

    // First determine decimal places from increment
    double increment = _rawTranslator(this->rawIncrement()).toDouble();
    if (!qIsNaN(increment)) {
        double integralPart;

        // Get the fractional part only
        increment = fabs(modf(increment, &integralPart));
        if (increment == 0.0) {
            // No fractional part, so no decimal places
            incrementDecimalPlaces = 0;
        } else {
            incrementDecimalPlaces = -ceil(log10(increment));
        }
    }

    if (_decimalPlaces == kUnknownDecimalPlaces) {
        if (incrementDecimalPlaces != kUnknownDecimalPlaces) {
            actualDecimalPlaces = incrementDecimalPlaces;
        } else {
            // Adjust decimal places for cooked translation
            int settingsDecimalPlaces = (_decimalPlaces == kUnknownDecimalPlaces) ? kDefaultDecimalPlaces : _decimalPlaces;
            const double ctest = _rawTranslator(1.0).toDouble();

            settingsDecimalPlaces += -log10(ctest);

            settingsDecimalPlaces = qMin(25, settingsDecimalPlaces);
            settingsDecimalPlaces = qMax(0, settingsDecimalPlaces);

            actualDecimalPlaces = settingsDecimalPlaces;
        }
    } else {
        actualDecimalPlaces = _decimalPlaces;
    }

    return actualDecimalPlaces;
}

FactMetaData *FactMetaData::createFromJsonObject(const QJsonObject &json, const QMap<QString, QString> &defineMap, QObject *metaDataParent)
{
    QString errorString;

    static const QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { _nameJsonKey,                 QJsonValue::String, true },
        { _typeJsonKey,                 QJsonValue::String, true },
        { _shortDescriptionJsonKey,     QJsonValue::String, false },
        { _longDescriptionJsonKey,      QJsonValue::String, false },
        { _unitsJsonKey,                QJsonValue::String, false },
        { _decimalPlacesJsonKey,        QJsonValue::Double, false },
        { _minJsonKey,                  QJsonValue::Double, false },
        { _maxJsonKey,                  QJsonValue::Double, false },
        { _hasControlJsonKey,           QJsonValue::Bool,   false },
        { _qgcRebootRequiredJsonKey,    QJsonValue::Bool,   false },
        { _rebootRequiredJsonKey,       QJsonValue::Bool,   false },
        { _categoryJsonKey,             QJsonValue::String, false },
        { _groupJsonKey,                QJsonValue::String, false },
        { _volatileJsonKey,             QJsonValue::Bool,   false },
        { _enumBitmaskArrayJsonKey,     QJsonValue::Array,  false },
        { _enumValuesArrayJsonKey,      QJsonValue::Array,  false },
        { _enumValuesJsonKey,           QJsonValue::String, false },
        { _enumStringsJsonKey,          QJsonValue::String, false },
    };

    if (!JsonHelper::validateKeys(json, keyInfoList, errorString)) {
        qWarning(FactMetaDataLog) << errorString;
        return new FactMetaData(valueTypeUint32, metaDataParent);
    }

    bool unknownType;
    const FactMetaData::ValueType_t type = FactMetaData::stringToType(json[_typeJsonKey].toString(), unknownType);
    if (unknownType) {
        qWarning(FactMetaDataLog) << "Unknown type" << json[_typeJsonKey].toString();
        return new FactMetaData(valueTypeUint32, metaDataParent);
    }

    FactMetaData *const metaData = new FactMetaData(type, metaDataParent);

    metaData->_name = json[_nameJsonKey].toString();

    QStringList rgDescriptions;
    QList<double> rgDoubleValues;
    QList<int> rgIntValues;
    QStringList rgStringValues;

    bool foundBitmask = false;
    if (!_parseValuesArray(json, rgDescriptions, rgDoubleValues, errorString)) {
        qWarning(FactMetaDataLog) << QStringLiteral("FactMetaData::createFromJsonObject _parseValueDescriptionArray for '%1' failed. %2").arg(metaData->name(), errorString);
    }
    if (rgDescriptions.isEmpty()) {
        if (!_parseBitmaskArray(json, rgDescriptions, rgIntValues, errorString)) {
            qWarning(FactMetaDataLog) << QStringLiteral("FactMetaData::createFromJsonObject _parseBitmaskArray for '%1' failed. %2").arg(metaData->name(), errorString);
        }
        foundBitmask = rgDescriptions.count() != 0;
    }
    if (rgDescriptions.isEmpty()) {
        if (!_parseEnum(metaData->_name, json, defineMap, rgDescriptions, rgStringValues, errorString)) {
            qWarning(FactMetaDataLog) << QStringLiteral("FactMetaData::createFromJsonObject _parseEnum for '%1' failed. %2").arg(metaData->name(), errorString);
        }
    }

    if (errorString.isEmpty() && !rgDescriptions.isEmpty()) {
        for (qsizetype i = 0; i < rgDescriptions.count(); i++) {
            if (foundBitmask) {
                metaData->addBitmaskInfo(rgDescriptions[i], 1 << rgIntValues[i]);
            } else {
                const QVariant rawValueVariant = !rgDoubleValues.isEmpty() ? QVariant(rgDoubleValues[i]) : QVariant(rgStringValues[i]);
                QVariant convertedValueVariant;
                QString errorString;
                if (metaData->convertAndValidateRaw(rawValueVariant, false /* validate */, convertedValueVariant, errorString)) {
                    metaData->addEnumInfo(rgDescriptions[i], convertedValueVariant);
                } else {
                    qWarning(FactMetaDataLog) << QStringLiteral("FactMetaData::createFromJsonObject convertAndValidateRaw on enum value for %1 failed.").arg(metaData->name())
                                              << "type:" << metaData->type()
                                              << "value:" << rawValueVariant
                                              << "error:" << errorString;
                }
            }
        }
    }

    metaData->setDecimalPlaces(json[_decimalPlacesJsonKey].toInt(kUnknownDecimalPlaces));
    metaData->setShortDescription(json[_shortDescriptionJsonKey].toString());
    metaData->setLongDescription(json[_longDescriptionJsonKey].toString());

    if (json.contains(_unitsJsonKey)) {
        metaData->setRawUnits(json[_unitsJsonKey].toString());
    }

    QString defaultValueJsonKey = _defaultValueJsonKey;
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    if (json.contains(_mobileDefaultValueJsonKey)) {
        defaultValueJsonKey = _mobileDefaultValueJsonKey;
    }
#endif

    if (json.contains(defaultValueJsonKey)) {
        const QJsonValue jsonValue = json[defaultValueJsonKey];

        if ((jsonValue.type() == QJsonValue::Null) && (type == valueTypeFloat || type == valueTypeDouble)) {
            metaData->setRawDefaultValue((type == valueTypeFloat) ? std::numeric_limits<float>::quiet_NaN() : std::numeric_limits<double>::quiet_NaN());
        } else {
            QVariant typedValue;
            QString errorString;
            const QVariant initialValue = jsonValue.toVariant();

            if (metaData->convertAndValidateRaw(initialValue, true /* convertOnly */, typedValue, errorString)) {
                metaData->setRawDefaultValue(typedValue);
            } else {
                qWarning(FactMetaDataLog) << "Invalid default value,"
                                          << "name:" << metaData->name()
                                          << "type:" << metaData->type()
                                          << "value:" << initialValue
                                          << "error:" << errorString;
            }
        }
    }

    if (json.contains(_incrementJsonKey)) {
        QVariant typedValue;
        QString errorString;
        const QVariant initialValue = json[_incrementJsonKey].toVariant();
        if (metaData->convertAndValidateRaw(initialValue, true /* convertOnly */, typedValue, errorString)) {
            metaData->setRawIncrement(typedValue.toDouble());
        } else {
            qWarning(FactMetaDataLog) << "Invalid increment value,"
                                      << "name:" << metaData->name()
                                      << "type:" << metaData->type()
                                      << "value:" << initialValue
                                      << "error:" << errorString;
        }
    }

    if (json.contains(_minJsonKey)) {
        QVariant typedValue;
        QString errorString;
        const QVariant initialValue = json[_minJsonKey].toVariant();
        if (metaData->convertAndValidateRaw(initialValue, true /* convertOnly */, typedValue, errorString)) {
            metaData->setRawMin(typedValue);
        } else {
            qWarning(FactMetaDataLog) << "Invalid min value,"
                                      << "name:" << metaData->name()
                                      << "type:" << metaData->type()
                                      << "value:" << initialValue
                                      << "error:" << errorString;
        }
    }

    if (json.contains(_maxJsonKey)) {
        QVariant typedValue;
        QString errorString;
        const QVariant initialValue = json[_maxJsonKey].toVariant();
        if (metaData->convertAndValidateRaw(initialValue, true /* convertOnly */, typedValue, errorString)) {
            metaData->setRawMax(typedValue);
        } else {
            qWarning(FactMetaDataLog) << "Invalid max value,"
                                      << "name:" << metaData->name()
                                      << "type:" << metaData->type()
                                      << "value:" << initialValue
                                      << "error:" << errorString;
        }
    }

    bool hasControlJsonKey = true;
    if (json.contains(_hasControlJsonKey)) {
        hasControlJsonKey = json[_hasControlJsonKey].toBool();
    }
    metaData->setHasControl(hasControlJsonKey);

    bool qgcRebootRequired = false;
    if (json.contains(_qgcRebootRequiredJsonKey)) {
        qgcRebootRequired = json[_qgcRebootRequiredJsonKey].toBool();
    }
    metaData->setQGCRebootRequired(qgcRebootRequired);

    bool rebootRequired = false;
    if (json.contains(_rebootRequiredJsonKey)) {
        rebootRequired = json[_rebootRequiredJsonKey].toBool();
    }
    metaData->setVehicleRebootRequired(rebootRequired);

    bool volatileValue = false;
    if (json.contains(_volatileJsonKey)) {
        volatileValue = json[_volatileJsonKey].toBool();
    }
    metaData->setVolatileValue(volatileValue);

    if (json.contains(_groupJsonKey)) {
        metaData->setGroup(json[_groupJsonKey].toString());
    }

    if (json.contains(_categoryJsonKey)) {
        metaData->setCategory(json[_categoryJsonKey].toString());
    }

    return metaData;
}

void FactMetaData::_loadJsonDefines(const QJsonObject &jsonDefinesObject, QMap<QString, QString> &defineMap)
{
    for (const QString &defineName: jsonDefinesObject.keys()) {
        const QString mapKey = _jsonMetaDataDefinesName + QStringLiteral(".") + defineName;
        defineMap[mapKey] = jsonDefinesObject[defineName].toString();
    }
}

QMap<QString, FactMetaData*> FactMetaData::createMapFromJsonFile(const QString &jsonFilename, QObject *metaDataParent)
{
    QMap<QString, FactMetaData*> metaDataMap;

    QString errorString;
    int version;
    const QJsonObject jsonObject = JsonHelper::openInternalQGCJsonFile(jsonFilename, qgcFileType, 1, 1, version, errorString);
    if (!errorString.isEmpty()) {
        qWarning(FactMetaDataLog) << "Internal Error:" << errorString;
        return metaDataMap;
    }

    static const QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { FactMetaData::_jsonMetaDataDefinesName, QJsonValue::Object, false },
        { FactMetaData::_jsonMetaDataFactsName, QJsonValue::Array, true },
    };
    if (!JsonHelper::validateKeys(jsonObject, keyInfoList, errorString)) {
        qWarning(FactMetaDataLog) << "Json document incorrect format:" << errorString;
        return metaDataMap;
    }

    QMap<QString /* define name */, QString /* define value */> defineMap;
    _loadJsonDefines(jsonObject[FactMetaData::_jsonMetaDataDefinesName].toObject(), defineMap);
    const QJsonArray factArray = jsonObject[FactMetaData::_jsonMetaDataFactsName].toArray();

    return createMapFromJsonArray(factArray, defineMap, metaDataParent);
}

QMap<QString, FactMetaData*> FactMetaData::createMapFromJsonArray(const QJsonArray &jsonArray, const QMap<QString, QString> &defineMap, QObject *metaDataParent)
{
    QMap<QString, FactMetaData*> metaDataMap;
    for (const QJsonValue &jsonValue : jsonArray) {
        if (!jsonValue.isObject()) {
            qWarning(FactMetaDataLog) << "JsonValue is not an object";
            continue;
        }

        const QJsonObject jsonObject = jsonValue.toObject();
        FactMetaData *const metaData = createFromJsonObject(jsonObject, defineMap, metaDataParent);
        if (metaDataMap.contains(metaData->name())) {
            qWarning(FactMetaDataLog) << "Duplicate fact name:" << metaData->name();
            delete metaData;
        } else {
            metaDataMap[metaData->name()] = metaData;
        }
    }

    return metaDataMap;
}

QVariant FactMetaData::cookedMax() const
{
    // We have to be careful with cooked min/max. Running the raw values through the translator could flip min and max.
    return qMax(_rawTranslator(_rawMax).toDouble(), _rawTranslator(_rawMin).toDouble());
}

QVariant FactMetaData::cookedMin() const
{
    // We have to be careful with cooked min/max. Running the raw values through the translator could flip min and max.
    return qMin(_rawTranslator(_rawMax).toDouble(), _rawTranslator(_rawMin).toDouble());
}

void FactMetaData::setVolatileValue(bool bValue)
{
    _volatile = bValue;
    if (_volatile) {
        _readOnly = true;
    }
}

QStringList FactMetaData::splitTranslatedList(const QString &translatedList)
{
    const QRegularExpression splitRegex("[,，、]"); // Note chinese commas for translations which have modified the english comma
    QStringList valueList = translatedList.split(splitRegex, Qt::SkipEmptyParts);
    for (QString &value: valueList) {
        value = value.trimmed();
    }
    return valueList;
}

bool FactMetaData::_parseEnum(const QString& name, const QJsonObject &jsonObject, const DefineMap_t &defineMap, QStringList &rgDescriptions, QStringList &rgValues, QString &errorString)
{
    rgDescriptions.clear();
    rgValues.clear();
    errorString.clear();

    if (!jsonObject.contains(_enumStringsJsonKey)) {
        return true;
    }

    const QString jsonStrings = jsonObject.value(_enumStringsJsonKey).toString();
    const QString defineMapStrings = defineMap.value(jsonStrings, jsonStrings);
    rgDescriptions = splitTranslatedList(defineMapStrings);

    const QString jsonValues = jsonObject.value(_enumValuesJsonKey).toString();
    const QString defineMapValues = defineMap.value(jsonValues, jsonValues);
    rgValues = splitTranslatedList(defineMapValues); // Never translated but still useful to use common string splitting code

    if (rgDescriptions.count() != rgValues.count()) {
        errorString = QStringLiteral("Enum strings/values count mismatch - name: '%1' strings: '%2'[%3] values: '%4'[%5]").arg(name).arg(defineMapStrings).arg(rgDescriptions.count()).arg(defineMapValues).arg(rgValues.count());
        return false;
    }

    return true;
}

bool FactMetaData::_parseValuesArray(const QJsonObject &jsonObject, QStringList &rgDescriptions, QList<double> &rgValues, QString &errorString)
{
    rgDescriptions.clear();
    rgValues.clear();
    errorString.clear();

    if (!jsonObject.contains(_enumValuesArrayJsonKey)) {
        return true;
    }

    static const QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { _enumValuesArrayDescriptionJsonKey, QJsonValue::String, true },
        { _enumValuesArrayValueJsonKey, QJsonValue::Double, true },
    };

    const QJsonArray &rgValueDescription = jsonObject[_enumValuesArrayJsonKey].toArray();
    for (const QJsonValue& jsonValue : rgValueDescription) {
        if (jsonValue.type() != QJsonValue::Object) {
            errorString = QStringLiteral("Value in \"values\" array is not an object.");
            return false;
        }

        const QJsonObject &valueDescriptionObject = jsonValue.toObject();
        if (!JsonHelper::validateKeys(valueDescriptionObject, keyInfoList, errorString)) {
            errorString = QStringLiteral("Object in \"values\" array failed validation '%2'.").arg(errorString);
            return false;
        }

        rgDescriptions.append(valueDescriptionObject[_enumValuesArrayDescriptionJsonKey].toString());
        rgValues.append(valueDescriptionObject[_enumValuesArrayValueJsonKey].toDouble());
    }

    return true;
}

bool FactMetaData::_parseBitmaskArray(const QJsonObject &jsonObject, QStringList &rgDescriptions, QList<int> &rgValues, QString &errorString)
{
    rgDescriptions.clear();
    rgValues.clear();
    errorString.clear();

    if (!jsonObject.contains(_enumBitmaskArrayJsonKey)) {
        return true;
    }

    static const QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { _enumBitmaskArrayDescriptionJsonKey, QJsonValue::String, true },
        { _enumBitmaskArrayIndexJsonKey, QJsonValue::Double, true },
    };

    const QJsonArray &rgValueDescription = jsonObject[_enumBitmaskArrayJsonKey].toArray();
    for (const QJsonValue &jsonValue : rgValueDescription) {
        if (jsonValue.type() != QJsonValue::Object) {
            errorString = QStringLiteral("Value in \"values\" array is not an object.");
            return false;
        }

        const QJsonObject &valueDescriptionObject = jsonValue.toObject();
        if (!JsonHelper::validateKeys(valueDescriptionObject, keyInfoList, errorString)) {
            errorString = QStringLiteral("Object in \"values\" array failed validation '%2'.").arg(errorString);
            return false;
        }

        rgDescriptions.append(valueDescriptionObject[_enumBitmaskArrayDescriptionJsonKey].toString());
        rgValues.append(valueDescriptionObject[_enumBitmaskArrayIndexJsonKey].toInt());
    }

    return true;
}
