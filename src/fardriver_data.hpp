#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>

#pragma pack(push, 1)

enum class PIN : uint8_t {
    NC = 0,
    PIN24 = 1,
    PIN15 = 2,
    PIN5 = 3,
    PIN17 = 4,
    PIN14 = 5,
    PIN3 = 6,
    PIN8 = 7,
    PB4 = 8,
    PINInvalid1 = 9,
    PIN2 = 10,
    PIN18 = 11,
    PIN9 = 12,
    PD1 = 13,
    PINInvalid2 = 14,
    PINInvalid3 = 15
};

struct BigEnd24b {
    uint32_t byte_0 : 8;
    uint32_t byte_1 : 8;
    uint32_t byte_2 : 8;

    float computeValue() const {
        uint32_t combined = ((uint32_t)byte_0 << 16) + ((uint32_t)byte_1 << 8) + (uint32_t)byte_2;
        return 1.953125f * std::sqrt(static_cast<float>(combined));
    }
};

struct FardriverData {
    FardriverData() {
        std::memset(this, 0, sizeof(FardriverData));
    }

    FardriverData(const uint8_t *heb_data) {
        const Addr00 *p = reinterpret_cast<const Addr00 *>(heb_data);
        memcpy(getAddr(0x00), p + 0, 12);
        memcpy(getAddr(0x06), p + 1, 12);
        memcpy(getAddr(0x0C), p + 2, 12);
        memcpy(getAddr(0x12), p + 3, 12);
        memcpy(getAddr(0x18), p + 4, 12);
        memcpy(getAddr(0x1E), p + 5, 12);
        memcpy(getAddr(0x24), p + 6, 12);
        memcpy(getAddr(0x2A), p + 7, 12);
        memcpy(getAddr(0x30), p + 8, 12);
        memcpy(getAddr(0x63), p + 9, 12);
        memcpy(getAddr(0x69), p + 10, 12);
        memcpy(getAddr(0x7C), p + 11, 12);
        memcpy(getAddr(0x82), p + 12, 12);
        memcpy(getAddr(0x88), p + 13, 12);
        memcpy(getAddr(0x8E), p + 14, 12);
        memcpy(getAddr(0x94), p + 15, 12);
        memcpy(getAddr(0x9A), p + 16, 12);
        memcpy(getAddr(0xA0), p + 17, 12);
        memcpy(getAddr(0xA6), p + 18, 12);
        memcpy(getAddr(0xAC), p + 19, 12);
        memcpy(getAddr(0xB2), p + 20, 12);
        memcpy(getAddr(0xB8), p + 21, 12);
        memcpy(getAddr(0xBE), p + 22, 12);
        memcpy(getAddr(0xC4), p + 23, 12);
        memcpy(getAddr(0xCA), p + 24, 12);
        memcpy(getAddr(0xD0), p + 25, 12);
    }

    uint8_t *getAddr(uint16_t addr) {
        return reinterpret_cast<uint8_t *>(this) + (addr << 1);
    }

    const uint8_t *getAddr(uint16_t addr) const {
        return reinterpret_cast<const uint8_t *>(this) + (addr << 1);
    }

    uint16_t getWord(uint16_t addr) const {
        return *reinterpret_cast<const uint16_t *>(
            reinterpret_cast<const uint8_t *>(this) + (addr << 1));
    }

    static constexpr uint16_t kMaxAddr = 0xFA;

    struct Addr00 {
        int16_t VolCoeff;
        int16_t Voltage2Coeff;
        int16_t PhaseACoeff;
        int16_t LineCoeff;
        int16_t PhaseCCoeff;
        int16_t SaveNum;
    } addr00;
    static_assert(sizeof(Addr00) == 12);

    struct Addr06 {
        uint8_t Arg2 : 1;
        uint8_t AntiTheftPulse : 2;
        uint8_t unk02a : 1;
        uint8_t Protocol485 : 4;
        uint8_t MorseCode : 7;
        uint8_t unk06b : 1;
        uint8_t SpeedKI;
        uint8_t SppedKP;
        uint8_t ThrottleLow;
        uint8_t ThrottleHigh;
        int16_t FAIF;
        int16_t CurveTime;
        uint8_t BrakeConfig : 4;
        uint8_t TempSensor : 3;
        uint8_t PhaseExchange : 1;
        uint8_t SlowDown : 3;
        uint8_t PC13Config : 1;
        uint8_t CurrAntiTheft : 1;
        uint8_t ParkConfig : 2;
        uint8_t Direction : 1;
    } addr06;
    static_assert(sizeof(Addr06) == 12);

    struct Addr0C {
        int16_t PhaseOffset;
        int16_t ZeroBattCoeff;
        int16_t FullBattCoeff;
        uint8_t StartKI;
        uint8_t MidKI;
        uint8_t MaxKI;
        uint8_t StartKP;
        uint8_t MidKP;
        uint8_t MaxKP;
    } addr0C;
    static_assert(sizeof(Addr0C) == 12);

    struct Addr12 {
        int16_t LD;
        uint16_t AlarmDelay;
        uint8_t PolePairs;
        uint8_t unk14b;
        uint16_t MaxSpeed;
        uint16_t RatedPower;
        uint16_t RatedVoltage;
    } addr12;
    static_assert(sizeof(Addr12) == 12);

    struct Addr18 {
        uint16_t RatedSpeed;
        uint16_t MaxLineCurr;
        uint8_t FollowConfig : 2;
        uint8_t ThrottleResponse : 2;
        uint8_t WeakA : 2;
        uint8_t RXD : 2;
        uint8_t SpeedPulse : 5;
        uint8_t GearConfig : 3;
        uint16_t LQ;
        uint16_t BattRatedCap;
        uint16_t IntRes;
    } addr18;
    static_assert(sizeof(Addr18) == 12);

    struct Addr1E {
        uint8_t FwReRatio;
        uint8_t ReverseSideStandSignal : 1;
        uint8_t AllowSmallCurrentSwitching : 1;
        uint8_t NeutralAntiSkidParking : 1;
        uint8_t RecordMileage : 1;
        uint8_t LowerMagneticBrake : 1;
        uint8_t DownhillReverseOvervoltageCutoff : 1;
        uint8_t ShieldedGearSpeedLimit2 : 1;
        uint8_t PD1Input : 1;
        uint16_t LowVolProtect;
        char CustomCode[2];
        uint8_t BCState : 1;
        uint8_t SeatEnable : 1;
        uint8_t PEnable : 1;
        uint8_t AutoBackPEnable : 1;
        uint8_t CruiseEnable : 1;
        uint8_t EABSEnable : 1;
        uint8_t PushEnable : 1;
        uint8_t ForceAntiTheft : 1;
        uint8_t OverSpeedAlarm : 1;
        uint8_t BrakeStillPark : 1;
        uint8_t RememberGear : 1;
        uint8_t unk21a : 1;
        uint8_t unk21b : 1;
        uint8_t unk21c : 1;
        uint8_t BackEnable : 1;
        uint8_t RelayDelay1S : 1;
        uint8_t ModelYear;
        uint8_t ModelMonth;
        uint8_t ModelDay;
        uint8_t TimeHour;
    } addr1E;
    static_assert(sizeof(Addr1E) == 12);

    struct Addr24 {
        uint8_t TimeMin;
        uint8_t TimeSecond;
        uint16_t HighVolProtect;
        uint16_t CustomMaxLineCurr;
        uint16_t CustomMaxPhaseCurr;
        uint16_t BackSpeed;
        uint16_t LowSpeed;
    } addr24;
    static_assert(sizeof(Addr24) == 12);

    struct Addr2A {
        uint16_t MidSpeed;
        uint16_t Max_Dec;
        uint8_t FreeThrottle;
        uint8_t unk2Cb;
        uint16_t MaxPhaseCurr;
        uint16_t SpeedAnalog;
        uint16_t Max_Acc;
    } addr2A;
    static_assert(sizeof(Addr2A) == 12);

    struct Addr30 {
        uint16_t StopBackCurr;
        uint16_t MaxBackCurr;
        uint8_t LowSpeedLineCurr;
        uint8_t MidSpeedLineCurr;
        uint8_t LowSpeedPhaseCurr;
        uint8_t MidSppedPhaseCurr;
        uint16_t BlockTime;
        uint16_t SpdPulseNum;
    } addr30;
    static_assert(sizeof(Addr30) == 12);

    uint16_t unk36[6];
    uint16_t unk3A[6];
    uint16_t unk42[6];
    uint16_t unk48[6];
    uint16_t unk4E[6];
    uint16_t unk54[6];
    uint16_t unk5A[3];

    struct Addr5D {
        uint16_t unk5D;
        uint16_t unk5E;
        uint16_t unk5F;
        uint8_t unk60;
        uint8_t unk60b;
        uint16_t unk61;
        uint8_t unk62;
        uint8_t unk62b;
    } addr5D;
    static_assert(sizeof(Addr5D) == 12);

    struct Addr63 {
        uint16_t MaxLineCurr2;
        uint16_t MaxPhaseCurr2;
        uint8_t MotorDia;
        uint8_t unk65b;
        uint16_t TempCoeff;
        uint16_t ProdMaxVol;
        uint16_t ISMax;
    } addr63;
    static_assert(sizeof(Addr63) == 12);

    struct Addr69 {
        PIN PausePin : 4;
        PIN SideStandPin : 4;
        PIN CruisePin : 4;
        PIN BoostPin : 4;
        PIN LowSpeedPin : 4;
        PIN HighSpeedPin : 4;
        PIN ReversePin : 4;
        PIN ForwardPin : 4;
        PIN SwitchVolPin : 4;
        PIN SeatPin : 4;
        PIN AntiTheftPin : 4;
        PIN ChargePin : 4;
        uint16_t LmtSpeed;
        uint16_t DistanceLSB;
        uint8_t ParaIndex;
        char SpecialCode;
    } addr69;
    static_assert(sizeof(Addr69) == 12);

    uint16_t unk6F;
    uint16_t unk70;
    uint16_t unk71;
    uint16_t unk72;
    uint16_t unk73;
    uint16_t unk74;
    uint16_t unk75;
    uint16_t unk76;
    uint16_t unk77;
    uint16_t unk78;
    uint16_t unk79;
    uint16_t unk7A;
    uint16_t unk7B;

    struct Addr7C {
        uint8_t unk7Ca : 1;
        uint8_t WeakTime : 3;
        uint8_t unk7Cb : 1;
        uint8_t QuickDown : 3;
        uint8_t SpeedMeterConfig1 : 1;
        uint8_t SpeedMeterConfig2 : 1;
        uint8_t FastRE : 1;
        uint8_t SpecialWeak : 1;
        uint8_t ZeroSwitch : 1;
        uint8_t unk7Cc : 1;
        uint8_t MOE : 1;
        uint8_t SpeedMeterConfig3 : 1;
        uint32_t TotalTime;
        uint32_t infoc1;
        uint16_t DistanceMSB;
    } addr7C;
    static_assert(sizeof(Addr7C) == 12);

    struct Addr82 {
        uint16_t ThrottleVoltage;
        uint16_t HighVolRestore;
        uint8_t MotorTempProtect;
        uint8_t MotorTempRestore;
        uint8_t MosTempProtect;
        uint8_t MosTempRestore;
        uint8_t CANConfig : 6;
        uint8_t unk86b : 2;
        char HardwareVersion;
        char SoftwareVersionMajor;
        uint8_t SoftwareVersionMinor;
    } addr82;

    struct Addr88 {
        uint8_t RatioMin;
        uint8_t Ratio500;
        uint8_t Ratio1000;
        uint8_t Ratio1500;
        uint8_t Ratio2000;
        uint8_t Ratio2500;
        uint8_t Ratio3000;
        uint8_t Ratio3500;
        uint8_t Ratio4000;
        uint8_t Ratio4500;
        uint8_t Ratio5000;
        uint8_t Ratio5500;
    } addr88;

    struct Addr8E {
        uint8_t Ratio6000;
        uint8_t Ratio6500;
        uint8_t Ratio7000;
        uint8_t Ratio7500;
        uint8_t Ratio8000;
        uint8_t Ratio8500;
        uint8_t Ratio9000;
        uint8_t RatioMax;
        int8_t nratio_0;
        int8_t nratio_1;
        int8_t nratio_2;
        int8_t nratio_3;
    } addr8E;

    struct Addr94 {
        int8_t nratio_4;
        int8_t nratio_5;
        int8_t nratio_6;
        int8_t nratio_7;
        int8_t nratio_8;
        int8_t nratio_9;
        int8_t nratio_10;
        int8_t nratio_11;
        int8_t nratio_12;
        int8_t nratio_13;
        int8_t nratio_14;
        int8_t nratio_15;
    } addr94;

    struct Addr9A {
        int8_t nratio_16;
        int8_t nratio_17;
        int8_t nratio_18;
        int8_t nratio_19;
        uint8_t AN : 4;
        uint8_t AlarmRecord_1 : 1;
        uint8_t RelayOut : 1;
        uint8_t EmptySpeed : 2;
        uint8_t LM : 5;
        uint8_t unk9Db : 3;
        int16_t InitVol;
        int16_t Stage1Curr;
        uint8_t VolSelectRatio;
        uint8_t unk9Fb;
    } addr9A;

    struct AddrA0 {
        uint8_t unkA0;
        uint8_t SysCmd;
        char ModelName[10];
    } addrA0;

    struct AddrA6 {
        char ModelName[10];
        uint16_t unkAB;
    } addrA6;

    struct AddrAC {
        uint16_t unkAC;
        uint16_t unkAD;
        uint16_t unkAE;
        uint16_t unkAF;
        uint16_t unkB0;
        uint16_t unkB1;
    } addrAC;

    struct AddrB2 {
        uint16_t unkB2;
        uint16_t unkB3;
        uint8_t OneCommSec_0;
        uint8_t OneCommSec_1;
        uint8_t OneCommSec_2;
        uint8_t OneCommSec_3;
        uint8_t OneCommSec_4;
        uint8_t OneCommSec_5;
        uint8_t OneCommSec_6;
        uint8_t OneCommSec_7;
    } addrB2;

    struct AddrB8 {
        uint8_t PausePosition : 4;
        uint8_t SideStandPosition : 4;
        uint8_t HBarPosition : 4;
        uint8_t AntiTheftPosition : 4;
        uint16_t unkB9;
        uint8_t Pulse;
        uint8_t SQH;
        uint16_t OnelineCurrCoeff;
        uint8_t BackPTime : 5;
        uint8_t ReleaseToSeat : 3;
        uint8_t CANBaud : 2;
        uint8_t unkBC : 2;
        uint8_t PasswordStatus : 2;
        uint8_t unkBCb : 2;
        uint8_t Stage1Soc;
        uint8_t Stage2Soc;
    } addrB8;

    struct AddrBE {
        uint8_t LowVolWay : 8;
        uint8_t unkBEb : 4;
        uint8_t AccCoeff : 4;
        uint16_t BstTime;
        uint16_t BstRelease;
        uint16_t ParkTime;
        uint8_t ReverseTime : 6;
        uint8_t JLNationalStandardStatus : 1;
        uint8_t Press2ForP : 1;
        uint8_t LowMediumToHighSpeed : 1;
        uint8_t NoSoundPrompt : 1;
        uint8_t Reserved : 1;
        uint8_t SilenceBuzzer : 1;
        uint8_t EnableBackupBuzzer : 1;
        uint8_t DisableMotorAngleSensorRepair : 1;
        uint8_t AllowBrakeFaultRepair : 1;
        uint8_t ReverseOneLine : 1;
        uint16_t TorqueCoeff;
    } addrBE;

    struct AddrC4 {
        uint8_t IdleClutchFunction : 1;
        uint8_t AutomaticVoltageSwitching : 1;
        uint8_t ReverseNeutralForIgnition : 1;
        uint8_t KnobToAdjustMaximumBusBar : 1;
        uint8_t OneTouchRepairSwitching : 1;
        uint8_t SqueezeBrake2xForPark : 1;
        uint8_t SqueezeBrakeToReleasePark : 1;
        uint8_t SingleCellNoBoost : 1;
        uint8_t LearnVolLow;
        uint8_t TapForwardAndBack : 1;
        uint8_t DualThrottleVoltage : 1;
        uint8_t CenterThrottleNeutral : 1;
        uint8_t EnableOneClickRepair : 1;
        uint8_t EnhancedInpuSignalFiltering : 1;
        uint8_t UnlimitedCruiseSpeed : 1;
        uint8_t HallDetectionX6 : 1;
        uint8_t NewMTPAVector : 1;
        uint8_t LearnVolHigh;
        uint16_t SlowDownRpm;
        uint16_t StartIs;
        uint16_t ThrottleInsert;
        uint8_t ExitFollowSpeed;
        uint8_t ReCurrRatio;
    } addrC4;

    struct AddrCA {
        uint8_t AngleLearn;
        PIN SpeedLimitPin : 4;
        PIN RepairPin : 4;
        uint8_t NoCanCnt;
        uint8_t SPModeConfig : 4;
        uint8_t Temp70 : 2;
        uint8_t LongBack : 1;
        uint8_t ThrottleLost : 1;
        uint8_t LearnThrottle;
        uint8_t SpeedLowCap;
        uint8_t MidSpeedCap;
        uint8_t SpeedLimitByCap;
        uint8_t MinSpeedCapCoeff;
        uint8_t ParkCoeff : 4;
        uint8_t BattSignal : 4;
        uint16_t ReIsinAcc;
    } addrCA;

    struct AddrD0 {
        uint8_t Data0;
        uint8_t Data1;
        uint8_t BMQHALL;
        uint8_t AVGPower;
        uint8_t WheelRatio;
        uint8_t WheelRadius;
        uint8_t AVGSpeed;
        uint8_t WheelWidth;
        uint16_t RateRatio;
        uint8_t Idle : 2;
        uint8_t Stop : 2;
        uint8_t ByteOption : 4;
        uint8_t SpecialFrame : 8;
    } addrD0;

    struct AddrD6 {
        int16_t unkD6;
        uint8_t Global_state1a : 1;
        uint8_t RunDir1 : 1;
        uint8_t HallPosError : 1;
        uint8_t Global_state1b : 1;
        uint8_t AutoLearn : 1;
        uint8_t NeedAutoLearn : 1;
        uint8_t Global_state1c : 1;
        uint8_t HallError : 1;
        uint8_t MotorRun : 1;
        uint8_t Global_state1d : 1;
        uint8_t Global_state1e : 1;
        uint8_t PhaseLostAlarm : 1;
        uint8_t MotorCloseEnable : 1;
        uint8_t MotorRun2 : 1;
        uint8_t VoltageReport : 1;
        uint8_t AngleFix : 1;
        uint8_t FlashWay : 1;
        uint8_t Global_state_2a : 1;
        uint8_t INVControl : 1;
        uint8_t WeakStat : 1;
        uint8_t LowVolStop : 1;
        uint8_t LowVolControl : 1;
        uint8_t EABSStart : 1;
        uint8_t Exchange : 1;
        uint8_t Global_state_2b : 1;
        uint8_t GearDisable : 1;
        uint8_t DefaultFW : 1;
        uint8_t SpeedLimit : 1;
        uint8_t MidSpeedEnabled : 1;
        uint8_t HighSpeedEnabled : 1;
        uint8_t FDEnable : 1;
        uint8_t FDBrake : 1;
        uint8_t Global_state3 : 1;
        uint8_t FDStart : 1;
        uint8_t FDClose : 1;
        uint8_t FDWay : 1;
        uint8_t SpeedLimitEnabled : 1;
        uint8_t PhaseCZero : 1;
        uint8_t FDWork : 1;
        uint8_t WUVD : 1;
        uint8_t PhaseAZero : 1;
        uint8_t ShouldBeStat : 1;
        uint8_t ZeroUseable : 1;
        uint8_t VUWD : 1;
        uint8_t WUUD : 1;
        uint8_t UUWD : 1;
        uint8_t VUUD : 1;
        uint8_t UUVD : 1;
        uint8_t VolFlash : 1;
        uint8_t FlashError : 1;
        uint8_t DeadZone : 1;
        uint8_t CANType : 1;
        uint8_t unkDAa : 1;
        uint8_t unkDAb : 1;
        uint8_t unkDAc : 1;
        uint8_t Zsignal : 1;
        uint8_t ABError : 1;
        uint8_t ZError : 1;
        uint8_t PError : 1;
        uint8_t MagnetError : 1;
        uint8_t ZABError : 1;
        uint8_t PABError : 1;
        uint8_t AngleError : 1;
        int16_t MosTemp;
    } addrD6;

    struct AddrDC {
        int16_t unkDC;
        int16_t unkDD;
        int16_t unkDE;
        int16_t unkDF;
        int16_t unkE0;
        int16_t unkE1;
    } addrDC;

    struct AddrE2 {
        uint8_t forward : 1;
        uint8_t reverse : 1;
        uint8_t gear : 2;
        uint8_t sliding_backwards : 1;
        uint8_t motion : 1;
        uint8_t unkE2a : 1;
        uint8_t compPhoneOK : 1;
        uint8_t phaseA_active : 1;
        uint8_t phaseB_active : 1;
        uint8_t phaseC_active : 1;
        uint8_t passOK : 2;
        uint8_t Bmq_Hall : 1;
        uint8_t LowVolRem : 1;
        uint8_t MosfetCheck : 1;
        uint8_t motor_hall_error : 1;
        uint8_t throttle_error : 1;
        uint8_t current_protect_restart : 1;
        uint8_t phase_current_surge_protect : 1;
        uint8_t voltage_protect : 1;
        uint8_t alarm_protect : 1;
        uint8_t motor_temp_protect : 1;
        uint8_t controller_temp_protect : 1;
        uint8_t phase_current_overflow_protect : 1;
        uint8_t phase_zero_error : 1;
        uint8_t phase_int16_t_protect : 1;
        uint8_t line_current_zero_error : 1;
        uint8_t mosfet_high_side_error : 1;
        uint8_t mosfet_low_side_error : 1;
        uint8_t moe_current_protect : 1;
        uint8_t brake : 1;
        uint8_t modulation;
        uint8_t pad;
        uint16_t MeasureSpeed;
        int16_t unkE6;
        int16_t unkE7;
    } addrE2;

    struct AddrE8 {
        int16_t deci_volts;
        int16_t per_mille;
        int16_t lineCurrent;
        int16_t unk3;
        int16_t unk4;
        int16_t ThrottleDepth;
    } addrE8;

    struct AddrEE {
        int8_t unkEE[2];
        int16_t unkEF;
        BigEnd24b PhaseACurr;
        BigEnd24b PhaseCCurr;
        int16_t volts;
    } addrEE;

    struct AddrF4 {
        int16_t motor_temp;
        int8_t unkF5;
        int8_t batt_cap;
        uint8_t unkF6;
        uint8_t ModifyYear;
        uint8_t ModifyMonth;
        uint8_t ModifyDay;
        uint16_t EXETotal;
        uint16_t EXESingle;
    } addrF4;

    struct AddrFA {
        int16_t PhaseAZero2;
        int16_t PhaseCZero2;
        uint8_t ErrorStop : 1;
        uint8_t pad_6 : 7;
        uint8_t unkFCa : 1;
        uint8_t old_blue : 1;
        uint8_t unkFCb : 2;
        uint8_t unkFCc : 1;
        uint8_t BlockStat : 1;
        uint8_t EABSStat : 1;
        uint8_t BrakeControl : 1;
        uint8_t unkFDa;
        uint8_t unkFDb;
        uint8_t motor_running_state_0 : 1;
        uint8_t motor_running_state_1 : 1;
        uint8_t motor_running_state_2 : 2;
        uint8_t motor_running_state_4 : 1;
        uint8_t motor_running_state_5 : 1;
        uint8_t motor_running_state_6 : 2;
        uint8_t unkFEb : 4;
        uint8_t GearControl : 1;
        uint8_t HXControl : 1;
        uint8_t TbarControl : 1;
        uint8_t HbarControl : 1;
        uint16_t unkFF;
    } addrFA;
    static_assert(sizeof(AddrFA) == 12);

    float getVoltage() const {
        return addrE8.deci_volts / 10.0f;
    }

    float getLineCurrent() const {
        return addrE8.lineCurrent / 4.0f;
    }

    // Raw live throttle depth (0 = idle, 32767 = full throttle)
    int16_t getThrottleRaw() const {
        return addrE8.ThrottleDepth;
    }

    // Throttle as 0.0–100.0 % (clamped)
    float getThrottlePct() const {
        float pct = addrE8.ThrottleDepth / 327.67f;
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 100.0f) pct = 100.0f;
        return pct;
    }

    float getSpeed() const {
        return addrE2.MeasureSpeed * (0.00376991136f *
            (addrD0.WheelRadius * 1270.0f + addrD0.WheelWidth * addrD0.WheelRatio) /
            addrD0.RateRatio);
    }

    float getMotorTemp() const {
        return addrF4.motor_temp * 9.0f / 5.0f + 32.0f;
    }

    float getMosTemp() const {
        return addrD6.MosTemp * 9.0f / 5.0f + 32.0f;
    }

    float getBatteryP() const {
        return 100.0f * (addrE8.deci_volts - addr0C.ZeroBattCoeff) /
            (addr0C.FullBattCoeff - addr0C.ZeroBattCoeff);
    }

    float getPhaseACurrent() const {
        return addrEE.PhaseACurr.computeValue();
    }

    float getPhaseCCurrent() const {
        return addrEE.PhaseCCurr.computeValue();
    }

    uint32_t getTorque() const {
        return ((addrE2.unkE6 * addrE2.unkE6 + addrE2.unkE7 * addrE2.unkE7) << 9) /
            addrBE.TorqueCoeff;
    }
};

static_assert(sizeof(FardriverData) == 512);

#pragma pack(pop)
