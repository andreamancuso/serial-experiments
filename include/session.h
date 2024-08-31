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

public:
    Session();
    ~Session() override;

    bool start(const char* portName);

    void stop();

    template <typename TMsg>
    void handle(TMsg& msg);

    void onReadEvent(const char *portName, unsigned int readBufferLen) override;

private:
    using Frame = cc_ublox::frame::UbloxFrame<InMessage>;

    std::array<std::uint8_t, 512> m_inputBuf;
    std::vector<std::uint8_t> m_inData;
    Frame m_frame;

    CSerialPort m_sp;
};
