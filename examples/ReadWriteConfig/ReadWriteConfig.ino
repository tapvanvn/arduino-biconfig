#include <BiConfig.hpp>


#define SD_PIN 4
#define CONF_FILE "config.bcf"

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
                BiConfig::Ledger* ledger = BiConfig::read(&file);

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
                Serial.println("begin test");
                BiConfig::Ledger* ledger = new BiConfig::Ledger();
                ledger->add("test8",(uint8_t)5);
                ledger->add("test16",(uint16_t)6);
                ledger->add("test32",(uint32_t)8);
                ledger->add("test64",(uint64_t)15);

                BiConfig::Ledger* ledger2 = new BiConfig::Ledger();
                ledger2->add("sub8",(uint8_t)5);
                lledger->addGroup("subgroup", ledger2);

                ledger->write(&file);

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
