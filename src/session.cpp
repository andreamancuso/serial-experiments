#include "shared.h"
#include "session.h"

#include <iostream>
#include <cassert>

#include "cc_ublox/message/CfgPrtUart.h"
#include "cc_ublox/message/NavPvt.h"
#include "comms/units.h"
#include "comms/process.h"

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"

using namespace itas109;

Session::Session() : m_inputBuf() {};

Session::~Session() = default;

bool Session::start(const char* portName)
{
    m_sp.init(portName,
                BaudRate38400, // baudrate
                ParityNone,    // parity
                DataBits8,     // data bit
                StopOne,       // stop bit
                FlowNone,      // flow
                4096           // read buffer size
        );
    m_sp.setReadIntervalTimeout(0);               // read interval timeout 0ms
    // sp.setByteReadBufferFullNotify(4096 * 0.8); // buffer full notify

    m_sp.open();
    printf("Open %s %s\n", portName, m_sp.isOpen() ? "Success" : "Failed");
    printf("Code: %d, Message: %s\n", m_sp.getLastError(), m_sp.getLastErrorMsg());

    configureUbxOutput();

    // connect for read
    m_sp.connectReadEvent(this);
    return true;
}

void Session::stop() {
    m_sp.disconnectReadEvent();
    m_sp.close();
};


void Session::onReadEvent(const char *portName, unsigned int readBufferLen)
{
    if (readBufferLen > 0)
    {
        unsigned char *data = new unsigned char[readBufferLen];

        if (data)
        {
            // read
            int recLen = m_sp.readData(data, readBufferLen);

            if (recLen > 0)
            {
                // printf("%s\n", char2hexstr(data, recLen).c_str());
                // std::vector<std::uint8_t> inData;

                for (unsigned int i = 0; i < recLen; i++) {
                    // m_inData.push_back(data[i]);

                    if (data[i] == 0xb5) {
                        if (!m_inData.empty()) {
                            processAllWithDispatch(&m_inData[0], m_inData.size(), m_frame, *this);
                        }

                        m_inData.clear();

                        m_inData.push_back(data[i]);

                        printf("Here begins a new message\n");
                    } else {
                        m_inData.push_back(data[i]);
                    }
                }

                // if (!inData.empty()) {
                //     auto consumed = processAllWithDispatch(&inData[0], inData.size(), m_frame, *this);
                // }


            }

            delete[] data;
            data = NULL;
        }
    }
};

void Session::handle(InNavPvt& msg)
{
    std::cout << "POS: lat=" << comms::units::getDegrees<double>(msg.field_lat()) <<
        "; lon=" << comms::units::getDegrees<double>(msg.field_lon()) <<
        "; alt=" << comms::units::getMeters<double>(msg.field_height()) << std::endl;

    printf("InNavPvt\n");
}

void Session::handle(InNavPosLlh& msg)
{
    std::cout << "POS: lat=" << comms::units::getDegrees<double>(msg.field_lat()) <<
        "; lon=" << comms::units::getDegrees<double>(msg.field_lon()) <<
        "; alt=" << comms::units::getMeters<double>(msg.field_height()) << std::endl;

    printf("InNavPosLlh\n");
}

void Session::handle(InMessage& msg)
{
    printf("here 2\n");

    static_cast<void>(msg); // ignore
}

void Session::processInputData()
{
    if (!m_inData.empty()) {
        auto consumed = processAllWithDispatch(&m_inData[0], m_inData.size(), m_frame, *this);
        m_inData.erase(m_inData.begin(), m_inData.begin() + consumed);
    }    
}

void Session::sendPosPoll()
{
    using OutNavPosllhPoll = cc_ublox::message::NavPosllhPoll<OutMessage>;
    sendMessage(OutNavPosllhPoll());

    // m_pollTimer.expires_from_now(boost::posix_time::seconds(1));
    // m_pollTimer.async_wait(
    //     [this](const boost::system::error_code& ec) {
    //         if (ec == boost::asio::error::operation_aborted) {
    //             return;
    //         }
    //
    //         sendPosPoll();
    //     });
}

void Session::sendMessage(const OutMessage& msg)
{
    OutBuffer buf;
    buf.reserve(m_frame.length(msg)); // Reserve enough space
    auto iter = std::back_inserter(buf);
    auto es = m_frame.write(msg, iter, buf.max_size());
    if (es == comms::ErrorStatus::UpdateRequired) {
        auto* updateIter = &buf[0];
        es = m_frame.update(updateIter, buf.size());
    }
    static_cast<void>(es);
    assert(es == comms::ErrorStatus::Success); // do not expect any error

    // while (!buf.empty()) {
        // boost::system::error_code ec;
        // auto count = m_serial.write_some(boost::asio::buffer(buf), ec);

        auto bytesWritten = m_sp.writeData(buf.data(), buf.size());

        printf("wrote %d\n", bytesWritten);

        // if (ec) {
        //     std::cerr << "ERROR: write failed with message: " << ec.message() << std::endl;
        //     m_io.stop();
        //     return;
        // }

        // buf.erase(buf.begin(), buf.begin() + count);
    // }
}

void Session::configureUbxOutput()
{
    // B5 62 06 8A 09 00 01 01 00 00 2A 00 91 20 01 77 00

    unsigned char hex[17];
    hex[0] = 0xb5;
    hex[1] = 0x62;
    hex[2] = 0x06;
    hex[3] = 0x8a;
    hex[4] = 0x09;
    hex[5] = 0x00;
    hex[6] = 0x01;
    hex[7] = 0x01;
    hex[8] = 0x00;
    hex[9] = 0x00;
    hex[10] = 0x2a;
    hex[11] = 0x00;
    hex[12] = 0x91;
    hex[13] = 0x20;
    hex[14] = 0x01;
    hex[15] = 0x77;
    hex[16] = 0x00;

    auto bytesWritten = m_sp.writeData(hex, sizeof(hex));

    printf("wrote %d\n", bytesWritten);

    return;

    // using OutCfgValset = cc_ublox::message::CfgValset<OutMessage>;
    //
    // OutCfgValset msg;
    // auto& layers = msg.field_layers();
    // auto& cfgData = msg.field_cfgdata();
    //
    // cc_ublox::message::CfgValsetFields<>::Cfgdata a;
    //
    // cc_ublox::field::CfgValPairSimple b;
    //
    // b.field_key().setValue(0xa2);
    // b.field_val().setValue(1);
    //
    // cfgData.setValue(b);
    //
    // layers.setBitValue_ram(true);
    //
    // sendMessage(msg);
}
