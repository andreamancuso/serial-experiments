#include "cc_ublox/Message.h"
#include "cc_ublox/message/NavPvt.h"
#include "cc_ublox/message/NavPosllh.h"
#include "cc_ublox/frame/UbloxFrame.h"

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"

using namespace itas109;

class Session : public CSerialPortListener
{
    using InMessage =
        cc_ublox::Message<
            comms::option::ReadIterator<const std::uint8_t*>,
            comms::option::Handler<Session> // Dispatch to this object
        >;

    using OutBuffer = std::vector<std::uint8_t>;
    using OutMessage =
        cc_ublox::Message<
            comms::option::IdInfoInterface,
            comms::option::WriteIterator<std::back_insert_iterator<OutBuffer>>,
            comms::option::LengthInfoInterface
        >;

    using InNavPvt = cc_ublox::message::NavPvt<InMessage>;
    using InNavPosLlh = cc_ublox::message::NavPosllh<InMessage>;

    using OutCfgValset = cc_ublox::message::CfgValset<OutMessage>;
    using CfgdataElement = cc_ublox::message::CfgValsetFields<>::CfgdataMembers::Element;
    using CfgValKeyId = cc_ublox::field::CfgValKeyIdCommon::ValueType;

public:
    Session();
    ~Session() override;

    bool start(const char* portName);

    void stop();

    void handle(InNavPvt& msg);

    void handle(InNavPosLlh& msg);

    void handle(InMessage& msg);

    void onReadEvent(const char *portName, unsigned int readBufferLen) override;

    void sendValSetWithSingleKeyValuePair(CfgValKeyId valKeyId, long valValue);

    void enableMessage(CfgValKeyId valKeyId);

    void disableMessage(CfgValKeyId valKeyId);

private:

    using AllInMessages =
        std::tuple<

    InNavPosLlh,InNavPvt
        >;

    using Frame = cc_ublox::frame::UbloxFrame<InMessage>;

    void processInputData();
    void sendPosPoll();
    void sendMessage(const OutMessage& msg);
    void configureUbxOutput();

    std::array<std::uint8_t, 512> m_inputBuf;
    std::vector<std::uint8_t> m_inData;
    Frame m_frame;

    CSerialPort m_sp;
};
