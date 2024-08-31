#include <iostream>
#include <cassert>

#include "cc_ublox/message/CfgPrtUart.h"
#include "cc_ublox/message/NavPvt.h"
#include "comms/units.h"
#include "comms/process.h"

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
#include <nlohmann/json.hpp>

#include "shared.h"
#include "session.h"
#include "json_converter.h"

using namespace itas109;
using json = nlohmann::json;

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
                for (unsigned int i = 0; i < recLen; i++) {
                    if (data[i] == 0xb5) {
                        if (!m_inData.empty()) {
                            processAllWithDispatch(&m_inData[0], m_inData.size(), m_frame, *this);
                        }

                        m_inData.clear();

                        m_inData.push_back(data[i]);
                    } else {
                        m_inData.push_back(data[i]);
                    }
                }
            }

            delete[] data;
            data = NULL;
        }
    }
};

template <typename TMsg>
    void handle(TMsg& msg)
{
    printf("here 3\n");

    json json;
    comms::util::tupleForEach(msg.fields(), JsonConverter(json));
 }
