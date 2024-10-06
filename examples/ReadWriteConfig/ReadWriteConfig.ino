#include "../../src/BiConfig.hpp"

#include <SD.h>
#define SD_PIN 4
#define CONF_FILE "config.bcf"

struct SdDataStream : IDataStream 
{
    SdDataStream(File& file):
        m_file{file}
    {

    }
    inline void write(uint8_t val)
    {
        m_file.write(val);
    }
    inline void write(uint8_t* data, size_t len)
    {
        m_file.write(data, len);
    }
    inline uint8_t read()
    {
        return m_file.read();
    }
    inline void read(uint8_t* data, size_t len)
    {
        m_file.read(data, len);
    }
    inline bool available()
    {
        return m_file.available();
    }
private:
    File& m_file;
};

void setup()
{
#if SERIAL_DEBUG
    Serial.begin(9600);
    while(!Serial)
    {
        ;
    }
#endif
    pinMode(SD_PIN,OUTPUT);

    if(SD.begin(SD_PIN))
    {
        if(SD.exists(CONF_FILE))
        {
            File file = SD.open(CONF_FILE, FILE_READ);

            if(file)
            {
                SdDataStream stream{file};
                BiConfig::Ledger* ledger = BiConfig::read(&stream);

                if(ledger)
                {
                    uint8_t* data = ledger->get("test16");
                    if(data)
                    {
                        uint16_t val = *(uint16_t*)data;
                        Serial.print("test16:");
                        Serial.println(val, DEC);

                        delete[] data;
                    }
                    else
                    {
                        Serial.println("cannot found value for key test16");
                    }

                    data = ledger->get("subgroup/sub8");
                    if(data)
                    {
                        uint8_t val = *data;
                        Serial.print("sub8:");
                        Serial.println(val, DEC);

                        delete[] data;
                    }
                    else
                    {
                        Serial.println("cannot found value for key subgroup/sub8");
                    }

                    delete ledger;
                }
                else
                {
                    Serial.println("oop! read biconfig fail");
                }

                file.close();
            }
        }
        else
        {
            File file = SD.open(CONF_FILE, FILE_WRITE);
            if(file)
            {
                SdDataStream stream{file};
                Serial.println("begin test");
                BiConfig::Ledger* ledger = new BiConfig::Ledger();
                ledger->add("test8",(uint8_t)5);
                ledger->add("test16",(uint16_t)6);
                ledger->add("test32",(uint32_t)8);
                ledger->add("test64",(uint64_t)15);

                BiConfig::Ledger* ledger2 = new BiConfig::Ledger();
                ledger2->add("sub8",(uint8_t)5);
                ledger->addGroup("subgroup", ledger2);

                ledger->write(&stream);

                delete ledger;
                delete ledger2;

                Serial.println("end test");

                if(SD.exists("config.b"))
                {
                    Serial.println("correct");
                }
                else
                {
                    Serial.println("incorrect");
                }

                file.close();
            }
        }
    }
    else
    {
        Serial.println("Init SD fail");
    }
}

void loop()
{

}
