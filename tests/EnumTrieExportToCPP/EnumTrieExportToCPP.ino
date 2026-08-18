// Enum trie to CPP export script - mainly for dev purposes

#include <Astruino.h>

struct EnumValue {
    AstroString text;
    int typeIndex;
};

static void printSpacer(int level)
{
    for (int index = 0; index < (level << 2); ++index) { Serial.print(' '); }
}

static char charAt(const AstroString &text, int index)
{
    return index >= 0 && index < (int)text.length() ? text[index] : '\0';
}

static void printChar(char value)
{
    if (!value) { Serial.print(F("'\\0'")); }
    else if (value == '\\') { Serial.print(F("'\\\\'")); }
    else if (value == '\'') { Serial.print(F("'\\\''")); }
    else {
        Serial.print('\'');
        Serial.print(value);
        Serial.print('\'');
    }
}

static int selectPosition(EnumValue *values, int valueCount, bool *usedPositions, int maxLength)
{
    int bestPosition = -1;
    int bestLargestGroup = valueCount + 1;
    int bestGroupCount = 0;

    for (int position = 0; position <= maxLength; ++position) {
        if (usedPositions[position]) { continue; }

        char groups[64];
        uint8_t groupSizes[64];
        int groupCount = 0;
        memset(groupSizes, 0, sizeof(groupSizes));

        for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
            char value = charAt(values[valueIndex].text, position);
            int groupIndex = 0;
            while (groupIndex < groupCount && groups[groupIndex] != value) { ++groupIndex; }
            if (groupIndex == groupCount) {
                groups[groupCount] = value;
                ++groupCount;
            }
            ++groupSizes[groupIndex];
        }

        if (groupCount <= 1) { continue; }

        int largestGroup = 0;
        for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
            largestGroup = max(largestGroup, (int)groupSizes[groupIndex]);
        }

        if (largestGroup < bestLargestGroup ||
            (largestGroup == bestLargestGroup && groupCount > bestGroupCount)) {
            bestPosition = position;
            bestLargestGroup = largestGroup;
            bestGroupCount = groupCount;
        }
    }

    return bestPosition;
}

static void printTree(EnumValue *values, int valueCount, bool *usedPositions, int maxLength,
                      int level, const AstroString &varName, const AstroString &typeCast)
{
    if (valueCount <= 0) { return; }
    if (valueCount == 1) {
        printSpacer(level);
        Serial.print(F("return "));
        Serial.print(typeCast.c_str());
        Serial.print(values[0].typeIndex);
        Serial.println(';');
        return;
    }

    int position = selectPosition(values, valueCount, usedPositions, maxLength);
    if (position < 0) { return; }
    usedPositions[position] = true;

    char groups[64];
    int groupCount = 0;
    for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
        char value = charAt(values[valueIndex].text, position);
        int groupIndex = 0;
        while (groupIndex < groupCount && groups[groupIndex] != value) { ++groupIndex; }
        if (groupIndex == groupCount) { groups[groupCount++] = value; }
    }

    printSpacer(level);
    Serial.print(F("switch ("));
    Serial.print(varName.c_str());
    Serial.print(F(".length() > "));
    Serial.print(position);
    Serial.print(F(" ? "));
    Serial.print(varName.c_str());
    Serial.print('[');
    Serial.print(position);
    Serial.println(F("] : '\\0') {"));

    for (int groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
        char groupChar = groups[groupIndex];
        int subsetCount = 0;
        for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
            if (charAt(values[valueIndex].text, position) == groupChar) { ++subsetCount; }
        }

        EnumValue *subset = new EnumValue[subsetCount];
        int subsetIndex = 0;
        for (int valueIndex = 0; valueIndex < valueCount; ++valueIndex) {
            if (charAt(values[valueIndex].text, position) == groupChar) {
                subset[subsetIndex++] = values[valueIndex];
            }
        }

        printSpacer(level + 1);
        Serial.print(F("case "));
        printChar(groupChar);
        Serial.println(':');
        printTree(subset, subsetCount, usedPositions, maxLength, level + 2, varName, typeCast);
        delete [] subset;
    }

    printSpacer(level);
    Serial.println('}');
    usedPositions[position] = false;
}

template<class T>
static void buildEnumTree(int countValue, T undefinedValue, AstroString (*toStringFn)(T, bool),
                          const char *varName, const char *typeCast,
                          const char *alias = nullptr, int aliasValue = -1)
{
    int valueCount = countValue + 2 + (alias ? 1 : 0);
    EnumValue *values = new EnumValue[valueCount];
    int valueIndex = 0;
    int maxLength = 0;

    values[valueIndex++] = {toStringFn(undefinedValue, false), -1};
    for (int typeIndex = 0; typeIndex <= countValue; ++typeIndex) {
        AstroString text = toStringFn((T)typeIndex, false);
        values[valueIndex++] = {text, typeIndex};
        maxLength = max(maxLength, (int)text.length());
    }
    if (alias) {
        values[valueIndex++] = {AstroString(alias), aliasValue};
        maxLength = max(maxLength, (int)strlen(alias));
    }

    bool *usedPositions = new bool[maxLength + 1];
    memset(usedPositions, 0, sizeof(bool) * (maxLength + 1));
    printTree(values, valueCount, usedPositions, maxLength, 1, AstroString(varName), AstroString(typeCast));

    delete [] usedPositions;
    delete [] values;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("Writing enum decoding trees..."));

    Serial.println(F("System mode tree:"));
    buildEnumTree(Astro_SystemMode_Count, Astro_SystemMode_Undefined, systemModeToString, "systemModeStr", "(Astro_SystemMode)");
    Serial.println(F("Measurement mode tree:"));
    buildEnumTree(Astro_MeasurementMode_Count, Astro_MeasurementMode_Undefined, measurementModeToString, "measurementModeStr", "(Astro_MeasurementMode)");
    Serial.println(F("Actuator type tree:"));
    buildEnumTree(Astro_ActuatorType_Count, Astro_ActuatorType_Undefined, actuatorTypeToString, "actuatorTypeStr", "(Astro_ActuatorType)");
    Serial.println(F("Sensor type tree:"));
    buildEnumTree(Astro_SensorType_Count, Astro_SensorType_Undefined, sensorTypeToString, "sensorTypeStr", "(Astro_SensorType)");
    Serial.println(F("Mount type tree:"));
    buildEnumTree(Astro_MountType_Count, Astro_MountType_Unknown, mountTypeToString, "mountTypeStr", "(Astro_MountType)");
    Serial.println(F("Rail type tree:"));
    buildEnumTree(Astro_RailType_Count, Astro_RailType_Undefined, railTypeToString, "railTypeStr", "(Astro_RailType)");
    Serial.println(F("Pin mode tree:"));
    buildEnumTree(Astro_PinMode_Count, Astro_PinMode_Undefined, pinModeToString, "pinModeStr", "(Astro_PinMode)");
    Serial.println(F("Enable mode tree:"));
    buildEnumTree(Astro_EnableMode_Count, Astro_EnableMode_Undefined, enableModeToString, "enableModeStr", "(Astro_EnableMode)");
    Serial.println(F("Units category tree:"));
    buildEnumTree(Astro_UnitsCategory_Count, Astro_UnitsCategory_Undefined, unitsCategoryToString, "unitsCategoryStr", "(Astro_UnitsCategory)");
    Serial.println(F("Units type tree:"));
    buildEnumTree(Astro_UnitsType_Count, Astro_UnitsType_Undefined, unitsTypeToSymbol, "unitsSymbolStr", "(Astro_UnitsType)", "J/s", Astro_UnitsType_Power_Wattage);
    Serial.println(F("Target class tree:"));
    buildEnumTree(Astro_TargetClass_Count, Astro_TargetClass_Unknown, targetClassToString, "targetClassStr", "(Astro_TargetClass)");
    Serial.println(F("Thermal mode tree:"));
    buildEnumTree(Astro_ThermalMode_Count, Astro_ThermalMode_Undefined, thermalModeToString, "thermalModeStr", "(Astro_ThermalMode)");
    Serial.println(F("Scheduler stage tree:"));
    buildEnumTree(Astro_SchedulerStage_Count, Astro_SchedulerStage_Undefined, schedulerStageToString, "schedulerStageStr", "(Astro_SchedulerStage)");

    Serial.println(F("Done!"));
}

void loop()
{ ; }
